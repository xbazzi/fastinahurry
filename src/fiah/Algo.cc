// C++ Includes
#include <sstream>
#include <cstdint>
#include <cstddef>
#include <expected>
#include <stop_token>

// FastInAHurry Includes
#include "fiah/Algo.hh"
#include "fiah/io/JSONReader.hh"
#include "fiah/io/TcpClient.hh"
#include "fiah/structs/Structs.hh"
#include "fiah/utils/Timer.hpp"
#include "fiah/utils/Logger.hh"
#include "fiah/AlgoException.hh"
#include "fiah/io/Config.hh"

// Third Party Includes
#include <nlohmann/json.hpp>


namespace fiah {

Algo::Algo(io::Config&& config)
    : p_config{memory::make_unique<io::Config>(std::move(config))}
{}

Algo::~Algo()
{
    stop_client();
}

// ============================================================================
// INITIALIZATION
// ============================================================================

auto Algo::initialize_server()
    -> std::expected<void, AlgoError>
{
    utils::Timer timer{"Algo::initialize_server()"};

    if (is_server_initialized())
    {
        LOG_WARN("TCP server already initialized");
        return {};
    }

    // Invariant check: p_config must exist (programming error if null)
    if (!p_config) [[unlikely]]
    {
        throw AlgoException("FATAL: p_config is null - Algo object in invalid state",
                          AlgoError::INVALID_STATE);
    }

    std::string market_ip = p_config->get_market_ip();
    std::uint16_t market_port = p_config->get_market_port();

    p_tcp_server = memory::make_unique<io::TcpServer>(
        market_ip, market_port
    );

    auto result = p_tcp_server->start();
    if (!result.has_value()) 
    {
        LOG_ERROR(
            "Algo failed to start TCP server on " 
            , market_ip , ':' , std::to_string(market_port)
            , ", with error: "
        );
        return std::unexpected(AlgoError::INIT_SERVER_FAIL);

        // throw AlgoException(
        //     "Algo failed to start TCP server on " 
        //     + market_ip + ':' + std::to_string(market_port)
        //     + ", with error: ",
        //     result.error()
        // );
    }
    m_server_started.store(true, std::memory_order_release);
    return {};
}

auto Algo::initialize_client()
    -> std::expected<void, AlgoError>
{
    utils::Timer timer{"Algo::initialize_client()"};
    if (is_client_initialized())
    {
        LOG_WARN("TCP client already initialized");
        return {};
    }

    // Invariant check: p_config must exist (programming error if null)
    if (!p_config) [[unlikely]]
    {
        throw AlgoException("FATAL: p_config is null - Algo object in invalid state",
                          AlgoError::INVALID_STATE);
    }

    const std::string& market_ip = p_config->get_market_ip();

    std::uint16_t market_port = p_config->get_market_port();

    p_tcp_client = memory::make_unique<io::TcpClient>(
        market_ip, market_port
    );

    LOG_INFO(
        "Client initialized with market server ip: ",
        market_ip, ", and port: ", market_port
    );

    // Connect immediately if possible
    auto connect_result = p_tcp_client->connect_to_server();
    if(!connect_result.has_value())
    {
        LOG_ERROR(
            "Couldn't connect to server during initialization. ",
            "Maybe the server is not online yet."
        );
        // Clean up the partially initialized client to prevent segfault
        p_tcp_client.reset();
        m_client_started.store(false, std::memory_order_release);
        return std::unexpected(AlgoError::SERVER_NOT_ONLINE);
    }

    m_client_started.store(true, std::memory_order_release);
    LOG_INFO("Client initialized and connected to market.");
    return {};
}

auto Algo::reconnect_client()
    -> std::expected<void, AlgoError>
{
    LOG_INFO("Attempting to reconnect client...");

    // Invariant check
    if (!p_config) [[unlikely]]
    {
        throw AlgoException("FATAL: p_config is null during reconnect",
                          AlgoError::INVALID_STATE);
    }

    const std::string& market_ip = p_config->get_market_ip();
    std::uint16_t market_port = p_config->get_market_port();

    // Clean up old client
    p_tcp_client.reset();
    m_client_started.store(false, std::memory_order_release);

    // Create new client and attempt connection
    p_tcp_client = memory::make_unique<io::TcpClient>(market_ip, market_port);

    auto connect_result = p_tcp_client->connect_to_server();
    if (!connect_result.has_value())
    {
        LOG_WARN("Reconnection attempt failed - server not available");
        p_tcp_client.reset();
        return std::unexpected{AlgoError::SERVER_NOT_ONLINE};
    }

    m_client_started.store(true, std::memory_order_release);
    LOG_INFO("Client successfully reconnected to market.");
    return {};
}

auto Algo::work_client()
    -> std::expected<void, AlgoError>
{
    utils::Timer timer{"Algo::work_client()"};

    if (is_client_running())
    {
        LOG_WARN("Client already running.");
        return {};
    }
    if (!is_client_initialized())
    {
        auto result = initialize_client();
        if (!result.has_value())
        {
            LOG_ERROR("Failed to initialize client while starting.");
            return std::unexpected(result.error());
        }
    }

    m_client_running.store(true, std::memory_order_release);
    m_client_stopping.store(false, std::memory_order_release);
    m_client_stopped.store(false, std::memory_order_release);
    LOG_INFO("Starting multithreaded pipeline...");

    m_network_thread = std::jthread([this](std::stop_token){
        this->_network_loop();
    });
    LOG_INFO("Started network thread with id: ", m_network_thread.get_id());

    m_strategy_thread = std::jthread([this](std::stop_token){
        this->_strategy_loop();
    });
    LOG_INFO("Started strategy thread with id: ", m_strategy_thread.get_id());

    m_execution_thread = std::jthread([this](std::stop_token){
        this->_execution_loop();
    });
    LOG_INFO("Started execution thread with id: ", m_execution_thread.get_id());

    // Attempt to set affinity (prints warnings if it can't)
    _set_thread_affinity(m_network_thread.native_handle(), 0);
    _set_thread_affinity(m_strategy_thread.native_handle(), 1);
    _set_thread_affinity(m_execution_thread.native_handle(), 2);

    return {};
}

///
/// @brief THREAD 1: Network Loop - Receives market data 
///
void Algo::_network_loop() {
    using namespace std::chrono_literals;
    try
    {

        LOG_INFO("Network thread started (Core 0)");
        std::byte buffer[sizeof(MarketData)];

        while (m_client_running.load(std::memory_order_acquire))
        {
            utils::Timer timer("Algo::network_loop");
            // Critical: Validate p_tcp_client exists before dereferencing
            if (!p_tcp_client)
            {
                LOG_WARN("TCP client is null - attempting reconnection...");

                // Attempt to reconnect with exponential backoff
                int retry_count = 0;
                const int max_retries = 5;

                while (retry_count < max_retries && m_client_running.load(std::memory_order_acquire))
                {
                    auto reconnect_result = reconnect_client();
                    if (reconnect_result.has_value())
                    {
                        LOG_INFO("Reconnection successful!");
                        break;
                    }

                    retry_count++;
                    if (retry_count < max_retries)
                    {
                        auto backoff = std::chrono::milliseconds(100 * (1 << retry_count)); // 200ms, 400ms, 800ms, 1600ms, 3200ms
                        LOG_WARN("Reconnection attempt ", retry_count, " failed. Retrying in ", backoff.count(), "ms...");
                        std::this_thread::sleep_for(backoff);
                    }
                }

                if (!p_tcp_client)
                {
                    LOG_ERROR("Failed to reconnect after ", max_retries, " attempts. Thread exiting.");
                    m_client_running.store(false, std::memory_order_release);
                    return;
                }

                continue; // Retry recv with new connection
            }

            auto recv_result = p_tcp_client->recv(buffer, sizeof(buffer));
            if (!recv_result.has_value())
            {
                LOG_WARN("Failed to receive market data packet. Socket disconnected - will attempt reconnect.");
                // Don't exit - clean up and let the null check above handle reconnection
                p_tcp_client.reset();
                m_client_started.store(false, std::memory_order_release);
                continue;
            }

            if (recv_result.value() != sizeof(MarketData))
            {
                LOG_WARN("Received incomplete market data packet. Discarding...");
                continue;
            }

            MarketData md;
            std::memcpy(&md, buffer, sizeof(MarketData));
            m_ticks_received.fetch_add(1, std::memory_order_relaxed);

            LOG_DEBUG("Got md (raw): " 
                , "Symbol: ", md.symbol, ", "
                , "Seq: ", md.seq_num, ", "
                , "ASk: ", md.ask, ", "
                , "Bid: ", md.bid, ", "
                , "CountStamp: ", static_cast<uint64_t>(md.timestamp_ns), ", "
                , "ticksReceived: ", m_ticks_received.load(std::memory_order_acquire)
            );

            if (!m_market_data_queue.push(md))
            {
                // Queue full
                m_queue_full_count.fetch_add(1, std::memory_order_relaxed);

                // Try to push with timeout to avoid infinite spin if shutting down
                while (m_client_running.load(std::memory_order_acquire))
                {
                    if (m_market_data_queue.push(md))
                        break;
                    std::this_thread::yield();
                }
            }
        }
        LOG_INFO("Network thread exiting...");
    }
    catch (const std::exception& e)
    {
        LOG_ERROR("Network thread crashed with exception: ", e.what());
        m_client_running.store(false, std::memory_order_release);
    }
    catch (...)
    {
        LOG_ERROR("Network thread crashed with unknown exception");
        m_client_running.store(false, std::memory_order_release);
    }
}

void Algo::_strategy_loop()
{
    try
    {
        LOG_INFO("Strategy thread started (Core 1)");
        MarketData md;

        while (m_client_running.load(std::memory_order_acquire))
        {
            if (m_market_data_queue.pop(md))
            {
                Signal signal = _compute_signal(md);
                if (signal.type != Signal::Type::HOLD)
                {
                    if (!m_signal_queue.push(signal))
                    {
                        LOG_WARN("Signal queue full for ", signal.symbol);
                        // dropping signal for now
                    } else {
                        LOG_DEBUG("Pushed into signal queue. " 
                            , signal.price, ", "
                            , signal.quantity, ", "
                            , signal.symbol, ", "
                            , static_cast<uint64_t>(signal.type), ", "
                        );
                        m_signals_generated.fetch_add(1, std::memory_order_relaxed);
                    }
                }
            } else {
                // Queue empty
                /// @todo benchmark against busy-spinning
                std::this_thread::yield();
            }
        }
        LOG_INFO("Strategy thread exiting.");
    }
    catch (const std::exception& e)
    {
        LOG_ERROR("Strategy thread crashed with exception: ", e.what());
        m_client_running.store(false, std::memory_order_release);
    }
    catch (...)
    {
        LOG_ERROR("Strategy thread crashed with unknown exception");
        m_client_running.store(false, std::memory_order_release);
    }
}

void Algo::_execution_loop()
{
    try
    {
        LOG_INFO("Execution thread started (Core 2)");
        Signal signal;

        while (m_client_running.load(std::memory_order_acquire))
        {
            if (m_signal_queue.pop(signal))
            {
                Order order = _generate_order(signal);

                // Send order out
                LOG_INFO(
                    "Executing order: ",
                    order.symbol, " ",
                    (order.side == Order::Side::BUY ? "BUY" : "SELL"), " ",
                    order.quantity, " @ ", order.price
                );

                m_orders_sent.fetch_add(1, std::memory_order_relaxed);

                /// @todo send order to market through the gateway
                ///         tcp_client->send or something
            } else {
                std::this_thread::yield();
            }
        }
        LOG_INFO("Execution thread exiting.");
    }
    catch (const std::exception& e)
    {
        LOG_ERROR("Execution thread crashed with exception: ", e.what());
        m_client_running.store(false, std::memory_order_release);
    }
    catch (...)
    {
        LOG_ERROR("Execution thread crashed with unknown exception");
        m_client_running.store(false, std::memory_order_release);
    }
}

Algo::Signal Algo::_compute_signal(const MarketData &md)
{
    utils::Timer timer{"_compute_signal()"};
    Signal signal;
    std::memcpy(signal.symbol, md.symbol, sizeof(signal.symbol));
    signal.timestamp_ns = md.timestamp_ns;

    // Spread check
    double spread = md.ask - md.bid;
    double mid = (md.ask + md.bid) / 2.0;
    LOG_DEBUG("ask, bid:", md.ask, ", ", md.bid);

    if (spread < 0.05) // Tight spread
    {
        if (mid < 190.0)
        {
            signal.type = Signal::Type::BUY;
            signal.price = md.ask;
            signal.quantity = 100;
        } else if (mid > 190.0005) {
            signal.type = Signal::Type::SELL;
            signal.price = md.bid;
            signal.quantity = 100;
        } else {
            signal.type = Signal::Type::HOLD;
            signal.price = -1;
        }
    }

    LOG_DEBUG("Signal: " 
        , (signal.type == Signal::Type::BUY)
          ? "BUY " : "MAYBE-SELL "
        , signal.symbol, ", "
        , signal.timestamp_ns, ", "
        , signal.price, ", "
        , signal.quantity, ", "
        , mid, ", "
    );
    return signal;
}

Algo::Order Algo::_generate_order(const Algo::Signal &signal)
{
    static std::atomic<uint64_t> s_order_id_counter{1};
    Order order;
    std::memcpy(order.symbol, signal.symbol, sizeof(order.symbol));
    order.side = (signal.type == Signal::Type::BUY)
                 ? Order::Side::BUY
                 : Order::Side::SELL;
    order.price = signal.price;
    order.quantity = signal.quantity;
    order.order_id = s_order_id_counter.fetch_add(1, std::memory_order_relaxed);

    auto now = std::chrono::steady_clock::now();
    order.timestamp_ns = now.time_since_epoch().count();
    return order;
}

void Algo::_set_thread_affinity(std::thread::native_handle_type thread, int cpu_id)
{
#ifdef __linux__
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu_id, &cpuset);
    int result = pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
    if (result != 0)
        LOG_WARN(
            "Algo failed to set thread affinity to CPU ID "
            , cpu_id, " ", "for thread ", thread
        );
    else
        LOG_INFO("Thread id ", thread, " pinned to CPU ", cpu_id);
#else
    LOG_WARN("Get on Linux so I can give your threads some affinity...")
#endif
}


void Algo::stop_client()
{
    if (is_client_stopped())
    {
        LOG_WARN("Client already stopped.");
        return;
    }

    LOG_INFO("Stopping client...");
    m_client_stopping.store(true, std::memory_order_release);
    m_client_running.store(false, std::memory_order_release);

    // Request cooperative cancellation
    if (m_network_thread.joinable())
        m_network_thread.request_stop();
    if (m_strategy_thread.joinable())
        m_strategy_thread.request_stop();
    if (m_execution_thread.joinable())
        m_execution_thread.request_stop();

    m_client_stopped.store(true, std::memory_order_release);

    print_client_stats();
    LOG_INFO("Client stopped.");
    // jthread dtor handles joining
}

void Algo::print_client_stats() const
{
    LOG_DEBUG(
        "\n=== Algo Statistics ==="
        , "\n\tTicks received: ",    m_ticks_received.load(std::memory_order_relaxed)
        , "\n\tSignals generated: ", m_signals_generated.load(std::memory_order_relaxed)
        , "\n\tOrders sent: ",       m_orders_sent.load(std::memory_order_relaxed)
        , "\n\tQueue full events: ", m_queue_full_count.load(std::memory_order_relaxed)
    // Queue status
        , "\n\tMarket data queue size: ", m_market_data_queue.size()
        , "\n\tSignal queue size: ",      m_signal_queue.size()
        , "\n\tOrder queue size: ",       m_order_queue.size()
    );
}


/// @brief Receives MarketData structs from market
// void Algo::work_client()
// {
//     utils::Timer timer{"Algo::work_client()"};
//     LOG_INFO("Client working");
//     LOG_DEBUG("");
//     if (!is_client_initialized()) [[unlikely]]
//     {
//         utils::Timer timer{"client not init"};
//         auto result = initialize_client();
//         if (!result.has_value())
//         {
//             LOG_ERROR("Algo failed to initialize client.");
//             // throw AlgoException("Client initialization failed.", result.error());
//         }
//     }
//     LOG_DEBUG("client is initialized");

//     std::byte buf[1024];
//     auto connect_result = p_tcp_client->connect_to_server();
//     if (!connect_result.has_value())
//     {
//         LOG_ERROR("Could not connect to server.");
//     } 
//     std::uint64_t rec_bytes = p_tcp_client->recv(buf, sizeof(buf)).value();
//     MarketData* market_data = reinterpret_cast<MarketData*>(buf);

//     LOG_INFO(
//         "Read ",
//         rec_bytes,
//         " bytes: \n",
//         "Symbol: ", market_data->symbol,
//         "\n\tBid: ", market_data->ask,
//         "\n\tAsk: ", market_data->bid,
//         "\ntimestamp: ", market_data->timestamp_ns
//     );
// }

auto Algo::work_server()
    -> std::expected<void, AlgoError>
{
    utils::Timer timer{"Algo::work_server()"};
    using namespace std::chrono_literals;

    if (!is_server_initialized()) [[unlikely]]
    {
        auto result = initialize_server();
        if (!result.has_value())
        {
            LOG_ERROR("Algo failed to initialize server.");
            return std::unexpected(result.error());
        }
    }

    // Invariant check: p_tcp_server must exist after initialization
    if (!p_tcp_server) [[unlikely]]
    {
        throw AlgoException("FATAL: p_tcp_server is null after initialization",
                          AlgoError::INVALID_STATE);
    }

    LOG_INFO(
        "Server listening on port ",
        p_config->get_market_port()
    );
    LOG_INFO("Accepting client connections...");

    // for (uint64_t i{0}; i <= 1; ++i)
    std::uint64_t i{0};
    do
    {
        /// Blocks while waiting for clients!
        auto client_socket = p_tcp_server->accept_client();
        if (!client_socket.has_value())
        {
            LOG_ERROR("Could not accept client - accept_client() failed");
            continue;
        }
        LOG_INFO("Got client with fd: ", client_socket.value());

        // Send market data continuously to this client until they disconnect
        for (;;)
        {
            utils::Timer timer{"Algo::work_server::LOOP"};
            if (client_socket.value() < 1)
            {
                LOG_WARN("Invalid client socket. Breaking send loop.");
                break;
            }

            MarketData tick{
                i,
                "AAPL",
                190.0 + i * 0.001,
                190.0 + i * 0.001 + 0.01,
                ++i
            };

            auto send_result = p_tcp_server->send(client_socket.value(), std::addressof(tick), sizeof(tick));
            if (!send_result.has_value())
            {
                LOG_INFO("Client fd: ", client_socket.value(), " disconnected. Waiting for next client...");
                break; // Client disconnected, go back to accept loop
            }

            LOG_DEBUG("Sent tick ", i, " to client fd: ", client_socket.value());
            std::this_thread::sleep_for(1s);
        }
    } while (true);

    // Unreachable in normal operation, but needed for signature
    return {};
}
} // End namespace fiah