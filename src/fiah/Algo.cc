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

using namespace std::chrono_literals;

namespace fiah {

Algo::Algo(io::Config&& config)
    : p_config{memory::make_unique<io::Config>(std::move(config))}
{}

Algo::~Algo()
{}

// ============================================================================
// INITIALIZATION
// ============================================================================
auto Algo::initialize_server()
    -> std::expected<void, AlgoError>
{
    utils::Timer timer{"Algo::initialize_server()"};

    std::string market_ip = p_config->get_market_ip();
    std::uint16_t market_port = p_config->get_market_port();

    p_tcp_server = memory::make_unique<io::TcpServer>(
        market_ip, market_port
    );

    auto result = p_tcp_server->start();
    if (!result.has_value()) 
    {
        throw AlgoException(
            "Algo failed to start TCP server on " 
            + market_ip + ':' + std::to_string(market_port)
            + ", with error: ",
            result.error()
        );
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

    // const std::string& market_ip = p_config->get_market_ip();
    std::string market_ip = p_config->get_market_ip();

    std::uint16_t market_port = p_config->get_market_port();

    p_tcp_client = memory::make_unique<io::TcpClient>(
        std::move(market_ip), market_port
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
        return std::unexpected(AlgoError::SERVER_NOT_ONLINE);
    }

    m_client_started.store(true, std::memory_order_release);
    LOG_INFO("Client initialized and connected to market.");
    return {};
}

void Algo::start_client()
{
    utils::Timer timer{"Algo::start_client()"};

    if (is_client_running())
    {
        LOG_WARN("Client already running.");
        return;
    }
    if (!is_client_initialized())
    {
        auto result = initialize_client();
        if (!result.has_value())
        {
            LOG_ERROR("Failed to initialize client while starting.");
            throw AlgoException("Failed to initialize client while starting: "
                , result.error()
            );
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
}

///
/// @brief THREAD 1: Network Loop - Receives market data 
///
void Algo::_network_loop()
{
    utils::Timer timer("Algo::network_loop");

    LOG_INFO("Network thread started (Core 0)");
    std::byte buffer[sizeof(MarketData)];

    while (m_client_running.load(std::memory_order_acquire))
    {
        auto recv_result = p_tcp_client->recv(buffer, sizeof(buffer));
        if (!recv_result.has_value())
        {
            LOG_WARN("Did not receive a market data packet. Trying again...");
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

void Algo::_strategy_loop()
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
                    LOG_WARN("Signal queue full for ", signal.symbol);
                // dropping signal for now
                else
                    m_signals_generated.fetch_add(1, std::memory_order_relaxed);
            }
        } else {
            // Queue empty
            /// @todo benchmark against busy-spinning
            std::this_thread::yield();
        }
    }
    LOG_INFO("Strategy thread exiting.");
}

void Algo::_execution_loop()
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

Algo::Signal Algo::_compute_signal(const MarketData &md)
{
    Signal signal;
    std::memcpy(signal.symbol, md.symbol, sizeof(signal.symbol));
    signal.timestamp_ns = md.timestamp_ns;

    // Spread check
    double spread = md.ask - md.bid;
    double mid = (md.ask + md.bid) / 2.0;

    if (spread < 0.05) // Tight spread
    {
        if (mid < 190.0)
        {
            signal.type = Signal::Type::BUY;
            signal.price = md.ask;
            signal.quantity = 100;
        } else if (mid > 195.0) {
            signal.type = Signal::Type::SELL;
            signal.price = md.bid;
            signal.quantity = 100;
        } else {
            signal.type = Signal::Type::HOLD;
        }
    }
    return signal;
}

Algo::Order Algo::_generate_order(const Algo::Signal &signal)
{
    static std::atomic<uint64_t> order_id_counter{1};
    Order order;
    std::memcpy(order.symbol, signal.symbol, sizeof(order.symbol));
    order.side = (signal.type == Signal::Type::BUY)
                 ? Order::Side::BUY
                 : Order::Side::SELL;
    order.price = signal.price;
    order.quantity = signal.quantity;
    order.order_id = order_id_counter.fetch_add(1, std::memory_order_relaxed);

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
        LOG_WARN("Algo failed to set thread affinity to CPU ID ", cpu_id);
    else
        LOG_INFO("Thread pinned to CPU ", cpu_id);
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
    m_client_running.store(false, std::memory_order_release);
    m_client_stopping.store(true, std::memory_order_release);

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
    LOG_INFO("=== Algo Statistics ===");
    LOG_INFO("Ticks received: ",    m_ticks_received.load(std::memory_order_relaxed));
    LOG_INFO("Signals generated: ", m_signals_generated.load(std::memory_order_relaxed));
    LOG_INFO("Orders sent: ",       m_orders_sent.load(std::memory_order_relaxed));
    LOG_INFO("Queue full events: ", m_queue_full_count.load(std::memory_order_relaxed));
    
    // Queue status
    LOG_INFO("Market data queue size: ", m_market_data_queue.size());
    LOG_INFO("Signal queue size: ",      m_signal_queue.size());
    LOG_INFO("Order queue size: ",       m_order_queue.size());
}


/// @brief Receives MarketData structs from market
void Algo::work_client()
{
    utils::Timer timer{"Algo::work_client()"};
    LOG_INFO("Client working");
    LOG_DEBUG("");
    if (!is_client_initialized()) [[unlikely]]
    {
        utils::Timer timer{"client not init"};
        auto result = initialize_client();
        if (!result.has_value())
        {
            LOG_ERROR("Algo failed to initialize client.");
            throw AlgoException("Client initialization failed.", result.error());
        }
    }
    LOG_DEBUG("client is initialized");

    std::byte buf[1024];
    auto connect_result = p_tcp_client->connect_to_server();
    if (!connect_result.has_value())
    {
        LOG_ERROR("Could not connect to server.");
    } 
    std::uint64_t rec_bytes = p_tcp_client->recv(buf, sizeof(buf)).value();
    MarketData* market_data = reinterpret_cast<MarketData*>(buf);

    LOG_INFO(
        "Read ",
        rec_bytes,
        " bytes: \n",
        "Symbol: ", market_data->symbol,
        "\n\tBid: ", market_data->ask,
        "\n\tAsk: ", market_data->bid,
        "\ntimestamp: ", market_data->timestamp_ns
    );
}

void Algo::work_server()
{
    utils::Timer timer{"Algo::work_server()"};
    if (!is_server_initialized()) [[unlikely]]
    {
        auto result = initialize_server();
        if (!result.has_value())
        {
            LOG_ERROR("Algo failed to initialize server.");
            throw AlgoException("Server pooped: ", result.error());
        }
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
            LOG_ERROR("Could not accept client with fd: ", client_socket.value());
            // continue;
        }
        LOG_INFO("Got client with fd: ", client_socket.value());
        for (;;)
        {
            if (client_socket.value() < 1) break;
            MarketData tick{
                i, "AAPL", 
                190.1 + 0.01*i, 
                190.2 + 0.01*i, 
                ++i
            };
            p_tcp_server->send(client_socket.value(), std::addressof(tick), sizeof(tick));
            std::this_thread::sleep_for(800ms);
        }
    } while (true);
}
} // End namespace fiah