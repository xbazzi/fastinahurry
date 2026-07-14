// clang-format off
// C++ Includes
#include <source_location>
#include <thread>
#include <type_traits>
#include <utility>
#include <cstdio>
#include <mutex>
#include <cstddef>
#include <algorithm>
#include <cinttypes>
#include <ranges>
#include <cstring>
#include <chrono>
#include <time.h>

// FastInAHurry Includes
#include "fiah/utils/Types.hh"
#include "fiah/structs/SPSCQueue.hh"
#include "fiah/structs/Vector.hh"
#include "fiah/utils/TimeStamp.hh"

namespace fiah
{

/// @todo use fmt::
class SPSCLogger
{
public:

    static constexpr sz_t QUEUE_SIZE{1 << 10};
    using Location = std::source_location;
#if defined(_cpp_lib_hardware_interference_size)
    using CacheLine = std::integral_constant<sz_t, std::hardware_destructive_interference_size>;
#else
    using CacheLine = std::integral_constant<sz_t, 64ULL>;
#endif

    enum class Level : u8_t
    {
        INFO,
        DEBUG,
        ERROR,
        WARN
    };

    struct Record
    {
        static constexpr sz_t BUFF_SIZE{1 << 9};

        alignas(CacheLine::value) char buff[BUFF_SIZE]; // 8 cache lines
        u64_t ts_ns;
        Location loc; 
        u16_t buff_size;
        Level level; 
        std::byte _reserved[6]{};

        static_assert(sizeof(_reserved) == 6);
    }; // 9 cache lines
    static_assert(sizeof(Record) == 9ULL * CacheLine::value);
    static_assert(alignof(Record) == CacheLine::value);

    using QueueT = fiah::SPSCQueue<Record, QUEUE_SIZE>;

public:
    ~SPSCLogger() noexcept;

    static SPSCLogger& get_instance() noexcept;

    template <class... Args>
    __always_inline
    void info(Location loc, const char* fmt, Args&& ...args) noexcept;
    
    template <class... Args>
    __always_inline
    void warn(Location loc, const char* fmt, Args&& ...args) noexcept;

    template <class... Args>
    __always_inline
    void error(Location loc, const char* fmt, Args&& ...args) noexcept;

    sz_t get_num_threads() const noexcept;


private:
    std::mutex m_queues_mutex;
    std::jthread m_consumer_thread;
    Vector<QueueT*> m_queues;

private:
    SPSCLogger() noexcept;
    SPSCLogger(const SPSCLogger&) = delete;
    SPSCLogger(SPSCLogger&& ) = delete;
    SPSCLogger &operator=(SPSCLogger&&) = delete;
    SPSCLogger &operator=(const SPSCLogger&) = delete;

    static constexpr char COLOR_TERMINATOR[]{"\033[0m"};
    static constexpr u8_t EXPECTED_NUM_PRODUCERS{4};

    void _run(std::stop_token st) noexcept;

    void _flush(Record* recs, u32_t n) noexcept;

    i32_t _serialize(char* out, const Record& rec, u32_t n) const noexcept;

    QueueT* _get_queue_ptr() noexcept;

    QueueT* _register_thread() noexcept;

    template <class... Args> 
    __always_inline
    void _log(SPSCLogger::Level level, Location loc, const char* fmt, Args &&...args) noexcept;

    static const char* _level_to_string(Level level) noexcept;

    static const char* _level_to_color(SPSCLogger::Level level) noexcept;

};

SPSCLogger::SPSCLogger() noexcept
    : m_consumer_thread{[this](std::stop_token st) noexcept { _run(st); }}
{
    m_queues.reserve(EXPECTED_NUM_PRODUCERS);
}

SPSCLogger::~SPSCLogger() noexcept
{
    m_consumer_thread.request_stop();
    m_consumer_thread.join();
    std::ranges::for_each(m_queues, [](auto* q_ptr) { delete q_ptr; } );
}

void SPSCLogger::_run(std::stop_token st) noexcept
{
    constexpr u32_t OUT_LIST_SIZE{10};
    Record out_list[OUT_LIST_SIZE]; // buffering records before flushing
    u32_t list_tracker{};

    Vector<QueueT*> q_snapshot;
    q_snapshot.reserve(EXPECTED_NUM_PRODUCERS);
    auto drain = [&]() {
        for (QueueT* q : q_snapshot)
        {
            Record rec{};
            while(q->pop(rec))
            {
                out_list[list_tracker++] = rec;
                if (list_tracker >= OUT_LIST_SIZE)
                    _flush(out_list, list_tracker), list_tracker = 0;
            }
            if (list_tracker > 0U)
                _flush(out_list, list_tracker), list_tracker = 0;
        }
    };

    while(!st.stop_requested())
    {
        {
            q_snapshot.clear();
            std::lock_guard lock{m_queues_mutex};
            q_snapshot = m_queues;
        }
        drain();
        std::this_thread::yield();
    }
    drain();
}

i32_t SPSCLogger::_serialize(char* out, const Record& rec, u32_t len) const noexcept
{
    // file name
    const char* full_file_path = rec.loc.file_name();
    const char* slash = std::strrchr(full_file_path, '/');
    const char* filename = slash? slash + 1: full_file_path;
    
    // function name
    char func_buf[128];
    snprintf(func_buf, sizeof(func_buf), "%s", rec.loc.function_name());

    if(char* paren = std::strrchr(func_buf, '('))
        *paren = '\0';

    const char* func_name = func_buf;
    if(const char* colon = std::strrchr(func_buf, ':'))
        func_name = colon + 1;

    // time
    std::time_t s_since_epoch = rec.ts_ns / 1'000'000'000ULL;
    u64_t ns = static_cast<u64_t>(rec.ts_ns % 1'000'000'000ULL);
    std::tm tm_buf{};
    localtime_r(&s_since_epoch, &tm_buf);


    char time[24];
    std::snprintf(time, sizeof(time), "%02d:%02d:%02d %09lu %s",
        tm_buf.tm_hour, tm_buf.tm_min, tm_buf.tm_sec, ns, tm_buf.tm_zone);

    return std::snprintf(out, len, "%s[%s]\t[%s][%s %s:%u]\t%s %s",
        _level_to_color(rec.level),
        _level_to_string(rec.level),
        time,
        filename, func_name, rec.loc.line(),
        COLOR_TERMINATOR,
        rec.buff
    );
}

void SPSCLogger::_flush(Record* recs, u32_t n) noexcept
{
    char line[Record::BUFF_SIZE + 512];
    for (u32_t i{}; i < n; ++i)
    {
        Record& rec = recs[i];
        int written_bytes = _serialize(line, rec, sizeof(line));
        std::fwrite(line, 1, written_bytes < 0 ? 0uz : static_cast<std::size_t>(written_bytes), stdout);
    }
    std::fflush(stdout);

}

auto SPSCLogger::_register_thread() noexcept -> QueueT* 
{
    QueueT* q = new QueueT{};
    std::lock_guard lock{m_queues_mutex};
    m_queues.push_back(q);
    return q;
}

auto SPSCLogger::_get_queue_ptr() noexcept -> QueueT*
{

    thread_local auto* q = SPSCLogger::get_instance()._register_thread();
    return q;
}

__always_inline
/*static*/ SPSCLogger& SPSCLogger::get_instance() noexcept
{
    static SPSCLogger instance{};
    return instance;
}

template <class... Args>
__always_inline
void SPSCLogger::info(Location loc, const char* fmt, Args&&... args) noexcept
{
    _log(Level::INFO, loc, fmt, std::forward<Args>(args)...);
}

template <class... Args>
__always_inline
void SPSCLogger::warn(Location loc, const char* fmt, Args&&... args) noexcept
{
    _log(Level::WARN, loc, fmt, std::forward<Args>(args)...);
}

template <class... Args>
__always_inline
void SPSCLogger::error(Location loc, const char* fmt, Args&&... args) noexcept
{
    _log(Level::ERROR, loc, fmt, std::forward<Args>(args)...);
}

sz_t SPSCLogger::get_num_threads() const noexcept
{
    return m_queues.size();
}


template <class... Args> 
__always_inline
void SPSCLogger::_log(SPSCLogger::Level level, Location loc, const char* fmt, Args&&... args) noexcept
{
    Record rec;
    rec.ts_ns = static_cast<u64_t>(TimeStamp<Resolution::Nano, std::chrono::system_clock>{}.get_ticks());
    rec.loc   = loc;
    rec.buff_size = 0;
    rec.level = level;
    int n;
    if constexpr(sizeof...(args) == 0)
        n = std::snprintf(rec.buff, Record::BUFF_SIZE, "%s", fmt);
    else
        n = std::snprintf(rec.buff, Record::BUFF_SIZE, fmt, args...);

    rec.buff_size = std::min(static_cast<u16_t>(n), static_cast<u16_t>(Record::BUFF_SIZE - 1)); // snprintf writes a null terminator

    _get_queue_ptr()->push(rec);
}

/*static*/ const char* SPSCLogger::_level_to_string(SPSCLogger::Level level) noexcept
{
    switch (level)
    {
    case SPSCLogger::Level::INFO:
        return "INFO";
    case SPSCLogger::Level::DEBUG:
        return "DEBUG";
    case SPSCLogger::Level::ERROR:
        return "ERROR";
    case SPSCLogger::Level::WARN:
        return "WARN";
    default:
        std::unreachable();
    }
}

/*static*/ const char* SPSCLogger::_level_to_color(SPSCLogger::Level level) noexcept
{
    switch (level)
    {
    case SPSCLogger::Level::INFO:
        return "\033[32m"; // Green
    case SPSCLogger::Level::DEBUG:
        return "\033[36m"; // Cyan
    case SPSCLogger::Level::ERROR:
        return "\033[31m"; // Red
    case SPSCLogger::Level::WARN:
        return "\033[33m"; // Red
    default:
        return "\033[0m";
    }
}

} // End namespace fiah