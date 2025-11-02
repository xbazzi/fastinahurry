#pragma once

// C++ Includes
#include <memory>
#include <atomic>
#include <thread>
#include <future>

// FastInAHurry includes
// #include "Publisher.hh"
// #include "structs/ThreadSafeQueue.hpp"
#include "ThreadPool.hpp"
#include "io/TcpServer.hh"
#include "io/TcpClient.hh"
#include "io/Config.hh"
#include "AlgoException.hh"


class Algo {
private:
    // std::queue<trading::Order> _orders;
    std::unique_ptr<io::Config> p_config;

    std::atomic<bool> m_initialized{false};
    std::atomic<bool> m_server_started{false};
    std::atomic<bool> m_client_started{false};
    std::atomic<bool> m_stopping{false};
    std::atomic<bool> m_running{false};

    // utils::ThreadSafeQueue<trading::Order> _order_queue;
    // utils::ThreadPool                      _thread_pool;
    std::mutex                      _orders_mutex;
    std::mutex                      _send_mutex;
    std::mutex                      _futures_mutex;
    std::jthread                    _reader_thread;
    std::jthread                    _network_thread;
    std::vector<std::jthread>       _worker_threads;
    std::vector<std::future<bool>>  _futures;
    std::unique_ptr<io::TcpServer>  p_tcp_server;
    std::unique_ptr<io::TcpClient>  p_tcp_client;
public:
    explicit Algo(const io::Config&) noexcept;
    void initialize();
    void generate_orders();
    void process();
    void work_server();
    bool start_work();
    bool start_work1();
    bool start_work2();
    bool start_work3();
    bool start_work4();
    bool start_work5();
    bool start_work6();
    void work_client();
    void start_background_processing();
    void start_market_data_streaming();
    // grpc::Status send(trading::Order&);
    __attribute__ ((__always_inline__))
    inline bool is_initialized() {
        return m_initialized.load(std::memory_order_acquire);
    }
    void stop();
    bool is_running();
};