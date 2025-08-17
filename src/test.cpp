#include <iostream>
#include <future>
#include <functional>
#include <thread>

int slow_add(int a, int b) {
    std::this_thread::sleep_for(std::chrono::seconds(3));
    return a + b;
}

int main() {
    // --- std::function ---
    std::function<int(int,int)> f = slow_add;
    std::cout << "Immediate result: " << f(2, 3) << "\n"; // blocks until done

    // --- std::packaged_task ---
    std::packaged_task<int(int,int)> task(slow_add);
    std::future<int> fut = task.get_future();
    std::thread t(std::move(task), 2, 3); // run in background

    std::cout << "Waiting for async result...\n";
    std::cout << "Async result: " << fut.get() << "\n"; // non-blocking wait possible
    std::cout << "Doing ma nails💅 ";
    t.join();
}
