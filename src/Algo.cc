// C++ Includes
#include <sstream>
#include <cstdint>
#include <chrono>

// FastInAHurry Includes
#include "Algo.hh"
#include "io/JSONReader.hh"
#include "MarketData.hh"
#include "io/TcpClient.hh"

// Third Party Includes
#include <nlohmann/json.hpp>
// #include <cassandra.h>

using namespace std::chrono_literals;

Algo::Algo(const io::Config& config) noexcept
    : p_config{std::make_unique<io::Config>(config)}
{ }

void Algo::initialize() 
{
    if (_initialized.load(std::memory_order_acquire))
        return;
    p_tcp_server = std::make_unique<io::TcpServer>(
        p_config->get_market_ip(), p_config->get_market_port()
    );
    p_tcp_server->start();
    _initialized.store(true, std::memory_order_release);
}

void Algo::work_client()
{
    if (!_initialized.load(std::memory_order_acquire))
        initialize();
    io::TcpClient client{};
    if (!client.connect_to_server().value())
        std::cout << "Could not connect to server." << '\n';

}
void Algo::work_server()
{
    if (!_initialized.load(std::memory_order_acquire))
        initialize();

    for (size_t i{1}; i <= 100; ++i)
    {
        SocketRAII client = p_tcp_server->accept_client();
        if (!client.valid()) continue;

        for (size_t seq{1}; seq <= 5; ++seq)
        {
            MarketData tick{
                seq, "AAPL", 
                190.1 + 0.01*seq, 
                190.2 + 0.01*seq, 
                seq
            };
            p_tcp_server->send(client, std::addressof(tick), sizeof(tick));
            std::this_thread::sleep_for(500ms);
        }
    }
}

bool Algo::is_running() {
    return _running.load(std::memory_order_relaxed);
}

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