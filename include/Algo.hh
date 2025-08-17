#pragma once

// C++ Includes
#include <memory>
#include <queue>
#include <atomic>
#include <thread>
#include <future>


// FastInAHurry includes
#include "Publisher.hh"
#include "utils/ThreadSafeQueue.hh"
#include "utils/ThreadPool.hh"


class Algo {
public:
    Algo();

    void initialize();
    void generate_orders();
    void process();
    void send(trading::Order&);
    bool initialized();
    void stop();
private:
    std::queue<trading::Order> _orders;
    std::atomic<bool> _initialized{false};
    std::atomic<bool> _stopping{false};
    std::atomic<bool> _running{false};
    std::shared_ptr<Publisher>  _pub;

    utils::ThreadSafeQueue<trading::Order> _order_queue;
    utils::ThreadPool                      _thread_pool;

    std::mutex                      _send_mutex;
    std::mutex                      _futures_mutex;
    std::jthread                    _reader_thread;
    std::vector<std::jthread>       _worker_threads;
    std::vector<std::future<bool>>  _futures;
};