// C++ Includes
#include <queue>
#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>

namespace utils {

template<typename T>
class ThreadSafeQueue {
private:
    std::queue<T> _queue;
    std::mutex _mutex;
    std::condition_variable _cv;

public:
    void push(T value) {

        {
            std::lock_guard<std::mutex> lock(_mutex);
            _queue.push(std::move(value));
        }
        _cv.notify_one();
    }

    T wait_and_pop() {
        std::unique_lock<std::mutex> lock(_mutex);
        _cv.wait(lock, [&] { return !_queue.empty(); });
        T val = std::move(_queue.front());
        _queue.pop();
        return val;
    }
};
} // End utils namespace