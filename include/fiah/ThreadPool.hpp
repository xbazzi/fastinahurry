// C++ Includes
#include <vector>
#include <queue>
#include <thread>
#include <functional>
#include <condition_variable>
#include <mutex>
#include <future>

namespace fiah::utils {

class ThreadPool {
private:
    std::vector<std::thread> _workers;
    std::queue<std::function<void()>> _tasks;

    std::mutex _queue_mutex;
    std::condition_variable _cv;
    bool _stop{false};

public:
    explicit ThreadPool(std::size_t threads = std::thread::hardware_concurrency());
    ~ThreadPool();

    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args) 
        -> std::future<std::invoke_result_t<F, Args...>> {
        using return_type = std::invoke_result_t<F, Args...>;

        auto task_ptr = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

        std::future<return_type> result = task_ptr->get_future();

        {
            std::unique_lock lock(_queue_mutex);
            if (_stop)
                throw std::runtime_error("Tried to enqueue on stopped ThreadPool");

            _tasks.emplace([task_ptr]() { (*task_ptr)(); });
        }
        _cv.notify_one();
        return result;
    }
};
} // End namespace fiah::utils