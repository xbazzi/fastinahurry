// C++ Includes
#include <sstream>

// FastInAHurry Includes
#include <Algo.hh>
#include <readers/JSONReader.hh>

// Third Party Includes
#include <nlohmann/json.hpp>

Algo::Algo() {
    std::cout << "Algo created" << std::endl;
}

void Algo::initialize() {
    std::string server_addr = "localhost:50051";
    auto channel = grpc::CreateChannel(server_addr, grpc::InsecureChannelCredentials());
    _pub = std::make_shared<Publisher>(channel);

    _initialized = true;
}

void Algo::stop() {
    // _running = false;

    _reader_thread.request_stop();
    for (auto& t : _worker_threads) {
        t.request_stop();
    }

    for (auto& future : _futures) {
        bool ok = future.get();
        if (!ok) {
            std::cerr << "[Warning] Some orders failed to send" << std::endl;
        }
    }
}

void Algo::generate_orders() {
    _orders = readers::json::read_orders_from_json("orders.json");
}

bool Algo::initialized() {
    return _initialized;
}

void Algo::process() {
    // Thread to read from file source and enqueue
    _reader_thread = std::jthread([this](std::stop_token stoken) {
        while (!stoken.stop_requested()) {
            auto orders = readers::json::read_orders_from_json("orders.json");
            while(!orders.empty()) {
                _order_queue.push(orders.front());
                orders.pop();
            }
        }
    });

    std::size_t num_workers = 4;
    for (std::size_t i = 0; i < num_workers; ++i) {
        _worker_threads.emplace_back([this] (std::stop_token stoken) {
            while(!stoken.stop_requested()) {
                trading::Order order = _order_queue.wait_and_pop();

                auto future = _thread_pool.enqueue([this, order]() -> bool {
                    grpc::Status status;
                    {
                        std::lock_guard lock(_send_mutex);
                        status = _pub->send_order(order);
                    }

                    if (!status.ok()) {
                        std::cerr << "[Error] Failed to send order: "
                                  << order.symbol() << "\n";
                        return false;
                    }

                    std::cout << "[Success] Sent order for "
                              << order.symbol() << "\n";
                    return true;
                });

                if (future.wait_for(std::chrono::milliseconds(10)) == std::future_status::ready) {
                    bool ok = future.get();
                    if (!ok) {
                        std::cerr << "[Inline] Order failed immediately for " << order.symbol() << '\n';
                    }
                } else {
                    _futures.push_back(std::move(future));
                }
            }
        });
    }

    // while(_running) {
        // this->generate_orders();
        // while (!_orders.empty()) {
        //     auto order = _orders.front();
        //     _orders.pop();
        //     send(order);
        // }
        // sleep(1);
    // }
}

void Algo::send(trading::Order& order) {
    _pub->send_order(order);
}