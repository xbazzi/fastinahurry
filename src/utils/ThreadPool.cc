// C++ Includes
#include <vector>
#include <queue>
#include <thread>
#include <functional>
#include <condition_variable>
#include <mutex>
#include <future>
#include "utils/ThreadPool.hh"

namespace utils {

ThreadPool::ThreadPool(std::size_t threads) {
    for (std::size_t i = 0; i < threads; ++i) {
        _workers.emplace_back([this] {
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock lock(_queue_mutex);
                    _cv.wait(lock, [this] { 
                        return _stop || !_tasks.empty();
                    });
                    if (_stop && _tasks.empty()) return;
                    task = std::move(_tasks.front());
                    _tasks.pop();
                } 
                task();
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock lock(_queue_mutex);
        _stop = true;
    }
    _cv.notify_all();

    for (auto& worker : _workers) {
        if (worker.joinable()) worker.join();
    }

}

} // End utils namespace