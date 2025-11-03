// C++ Includes
#include <sstream>
#include <cstdint>
#include <cstddef>
#include <expected>

// FastInAHurry Includes
#include "fiah/Algo.hh"
#include "fiah/io/JSONReader.hh"
#include "fiah/market/MarketData.hh"
#include "fiah/io/TcpClient.hh"
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
// THREAD 1: Network Loop - Receives market data 
// ============================================================================
void Algo::_network_loop()
{
    LOG_INFO("Network thread started (Core 0)");

}

void Algo::_strategy_loop()
{
    LOG_INFO("Strategy thread started (Core 1)");
    MarketData md;

    while (m_client_running.load(std::memory_order_relaxed))
    {
        return;
    }
}

void Algo::_execution_loop()
{
}

Algo::Signal Algo::_compute_signal(const MarketData &md)
{
    return Signal{};
}

Algo::Order Algo::_generate_order(const Algo::Signal &signal)
{
    return Order{};
}

void Algo::_set_thread_affinity(std::thread::native_handle_type thread, int cpu_id)
{
#ifdef __linux__
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu_id, &cpuset);
    int result = pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
    if (result != 0)
        LOG_WARN("Failed to set thread affinity to CPU ID ", cpu_id);
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
        LOG_DEBUG("Client already stopped.");
        return;
    }

    LOG_INFO("Stopping client...");
    m_client_running.store(false, std::memory_order_release);
    m_client_stopping.store(true, std::memory_order_release);
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
            "Failed to start TCP server on " 
            + market_ip + ':' + std::to_string(market_port)
            + ", with error: ",
            result.error()
        );
    }
    m_server_started.store(true, std::memory_order_release);
    return {};
}

bool Algo::is_server_initialized() const noexcept
{
    return m_server_started;
}

bool Algo::is_client_initialized() const noexcept
{
    return m_client_started;
}

bool Algo::is_client_stopped() const noexcept
{
    return m_client_stopped;
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

void Algo::work_client()
{
    utils::Timer timer{"Algo::work_client()"};
    LOG_INFO("Client working");
    if (!is_client_initialized()) [[unlikely]]
    {
        auto result = initialize_client();
        if (!result.has_value())
        {
            LOG_ERROR("Failed to initialize client.");
            throw AlgoException("Client initialization failed.", result.error());
        }
    }

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
        "\n\tAsk: ", market_data->bid
    );
    // m_market_data_queue;
}

void Algo::process_market_data()
{


}

void Algo::work_server()
{
    utils::Timer timer{"Algo::work_server()"};
    if (!is_server_initialized()) [[unlikely]]
    {
        auto result = initialize_server();
        if (!result.has_value())
        {
            LOG_ERROR("Failed to initialize client.");
            throw AlgoException("Server pooped: ", result.error());
        }
    }

    LOG_INFO(
        "Server listening on port ", 
        p_config->get_market_port()
    );
    LOG_INFO("Accepting client connections...");
    for (std::uint16_t i{1}; i <= 1; ++i)
    {
        auto client_socket = p_tcp_server->accept_client();
        if (!client_socket.has_value())
        {
            LOG_ERROR("Could not accept client with fd: ", client_socket.value());
            continue;
        }
        LOG_INFO("Got client with fd: ", client_socket.value());

        MarketData tick{
            i, "AAPL", 
            190.1 + 0.01*i, 
            190.2 + 0.01*i, 
            i
        };
        p_tcp_server->send(client_socket.value(), std::addressof(tick), sizeof(tick));
        std::this_thread::sleep_for(800ms);
    }
}
} // End namespace fiah

/// @brief Old implementation of thread workers
// bool Algo::is_running() 
// {
//     return m_running.load(std::memory_order_relaxed);
// }

// void Algo::stop() {
//     _stopping.store(true, std::memory_order_release);
//     _running.store(false, std::memory_order_release);

//     _reader_thread.request_stop();
//     for (auto& t : _worker_threads) {
//         t.request_stop();
//     }

//     if (_reader_thread.joinable()) {
//         _reader_thread.join();
//     }
//     for (auto& t : _worker_threads) {
//         if (t.joinable()) {
//             t.join();
//         }
//     }

//     {
//         std::lock_guard<std::mutex> lk(_futures_mutex);
//         for (auto& future : _futures) {
//             bool ok = future.get();
//             if (!ok) {
//                 std::cerr << "[Warning] Some orders failed to send" << std::endl;
//             }
//         }
//         _futures.clear();
//     }
// }

// void Algo::generate_orders() {
//     std::lock_guard<std::mutex> lock(_orders_mutex);
//     // _orders = io::read_orders_from_json("orders.json");
// }


// void Algo::process() {
//     this->generate_orders();
//     std::lock_guard<std::mutex> lock(_orders_mutex);
//     while (!_orders.empty()) {
//         auto order = _orders.front();
//         _orders.pop();
//         auto status = send(order);
//         if (status.ok()) {
//             std::ostringstream ss;
//             ss << "[Success] Sent order to buy " << order.quantity()
//                       << " shares of " << order.symbol() << " at $" 
//                       << order.price() << std::endl;
//             std::cout << ss.str();
//         } else {
//             std::cout << "[Error] Failed to send order for " << order.symbol() 
//                       << " - " << status.error_message() << std::endl;
//         }
//     }
// }

// void Algo::start_market_data_streaming() {
//     if (!_pub) {
//         std::cerr << "[Error] Publisher not initialized. Call initialize() first." << std::endl;
//         return;
//     }
    
//     _pub->start_market_data_stream("AAPL");
// }

// void Algo::start_background_processing() {
//     // Thread to read from file source and enqueue
//     _reader_thread = std::jthread([this](std::stop_token stoken) {
//         while (!stoken.stop_requested()) {
//             auto orders = io::read_orders_from_json("orders.json");
//             while(!orders.empty()) {
//                 _order_queue.push(orders.front());
//                 orders.pop();
//             }
//         }
//     });

//     std::size_t num_workers = 1;
//     for (std::size_t i = 0; i < num_workers; ++i) {
//         _worker_threads.emplace_back([this] (std::stop_token stoken) {
//             while(!stoken.stop_requested()) {
//                 trading::Order order = _order_queue.wait_and_pop();

//                 auto future = _thread_pool.enqueue([this, order]() -> bool {
//                     grpc::Status status;
//                     {
//                         std::lock_guard lock(_send_mutex);
//                         status = _pub->send_order(order);
//                         std:: cout << "WOAH!" << std::endl;
//                     }

//                     if (!status.ok()) {
//                         std::cerr << "[Error] Failed to send order: "
//                                   << order.symbol() << "\n";
//                         return false;
//                     }

//                     std::cout << "[Success] Sent order for "
//                               << order.symbol() << "\n";
//                     return true;
//                 });

//                 if (future.wait_for(std::chrono::milliseconds(10)) == std::future_status::ready) {
//                     bool ok = future.get();
//                     if (!ok) {
//                         std::cerr << "[Inline] Order failed immediately for " << order.symbol() << '\n';
//                     }
//                 } else {
//                     if (_stopping.load(std::memory_order_relaxed)) {
//                         bool ok = future.get();
//                         if (!ok) std::cerr << "[Late] Order failed for " << order.symbol() << std::endl;
//                     } else {
//                         std::cout << "future got pushed brah" << std::endl;
//                         {
//                             std::lock_guard<std::mutex> lk(_futures_mutex);
//                             _futures.emplace_back(std::move(future));
//                         }
//                     }
//                 }
//             }
//         });
//     }
// }

// void Algo::cleanup_completed_futures() {
//     std::lock_guard<std::mutex> lk(_futures_mutex);
//     auto it = _futures.begin();
//     while (it != _futures.end()) {
//         if (it->wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
//             bool ok = it->get();
//             if (!ok) {
//                 std::cerr << "[Background] Future failed" << std::endl;
//             }
//             it = _futures.erase(it);
//         } else {
//             ++it;
//         }
//     }
// }

// Order& Algo::receive() {
//     // return _sub->receive_order(order);

// }
