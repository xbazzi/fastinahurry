#include <chrono>
#include <iostream>
#include <string_view>

namespace utils {

class Timer
{
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> m_start_timepoint;
    std::string_view m_scope_name;
public:
Timer(const std::string_view scope_name) 
    : m_scope_name{scope_name}
{ 
    start(); 
}

void start()
{
    m_start_timepoint = std::chrono::high_resolution_clock::now();
}

void stop()
{
    auto end_timepoint = std::chrono::high_resolution_clock::now();
    auto start = std::chrono::time_point_cast<std::chrono::microseconds>(m_start_timepoint).time_since_epoch().count();
    auto end = std::chrono::time_point_cast<std::chrono::microseconds>(end_timepoint).time_since_epoch().count();
    std::cout << "[" << m_scope_name << "] Took " << 1e-3 * (end - start) << " ms" << std::endl;
}

~Timer()
{
    stop();
}
};
} // End namespace utils 