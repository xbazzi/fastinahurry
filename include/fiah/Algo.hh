#pragma once

// C++ Includes
// #include <memory>
#include <atomic>
#include <thread>
#include <future>
#include <cstdint>

// FastInAHurry includes
#include "fiah/ThreadPool.hpp"
#include "fiah/io/TcpServer.hh"
#include "fiah/io/TcpClient.hh"
#include "fiah/io/Config.hh"
#include "fiah/AlgoException.hh"
#include "fiah/Error.hh"
#include "fiah/utils/Logger.hh"
#include "fiah/structs/SPSCQueue.hh"
#include "fiah/structs/Structs.hh"
#include "fiah/memory/unique_ptr.hh"

namespace fiah {

/// @brief Handles orchestration of the entire system
///
/// @section error_handling Error Handling Strategy
///
/// This class uses a hybrid error handling approach:
///
/// @subsection std_expected std::expected<void, AlgoError>
/// Used for **recoverable errors** and **expected failure modes**:
/// - Network unavailable (server not online, connection timeout)
/// - Initialization failures (port in use, permission denied)
/// - Resource temporarily unavailable
///
/// Functions returning std::expected:
/// - initialize_client() - Connection may fail if server offline
/// - initialize_server() - Bind may fail if port in use
/// - work_client() - Thread startup or initialization may fail
/// - work_server() - Server operations may fail gracefully
///
/// @subsection exceptions Exceptions (AlgoException, std::exception)
/// Used for **unrecoverable errors** and **programming errors**:
/// - Invariant violations (null pointers after successful initialization)
/// - Invalid state transitions (using uninitialized objects)
/// - System resource exhaustion (thread creation failure)
/// - Fatal errors that must propagate to Controller
///
/// Functions that may throw:
/// - All public functions may throw AlgoException for fatal errors
/// - Controller catches all exceptions as the termination point
///
/// @subsection thread_safety Thread Safety
/// - Thread loop functions (_network_loop, _strategy_loop, _execution_loop)
///   NEVER throw - they catch all exceptions internally and log them
/// - Atomic flags (m_client_running, etc.) provide thread-safe state
///
/// @attention Always catch exceptions when calling Algo methods from Controller!
class Algo
{

private:

    /// @brief We're gonna need these
    using Signal = structs::Signal;
    using Order = structs::Order;
    using MarketData = structs::MarketData;

    /// @brief Drop-in replacement for std::unique_ptr<> and std::make_unique<>
    using ConfigUniquePtr = memory::unique_ptr<io::Config>;
    using TcpServerUniquePtr = memory::unique_ptr<io::TcpServer>;
    using TcpClientUniquePtr = memory::unique_ptr<io::TcpClient>;

    ConfigUniquePtr p_config;
    static inline utils::Logger<Algo>& 
        m_logger{utils::Logger<Algo>::get_instance("Algo")};

    std::atomic<bool> m_server_started{false};
    std::atomic<bool> m_client_started{false};
    std::atomic<bool> m_client_stopping{false};
    std::atomic<bool> m_client_stopped{false};
    std::atomic<bool> m_client_running{false};

    // Lock-free single-producer, single-consumer queues 
    structs::SPSCQueue<structs::MarketData, 4096UL> m_market_data_queue;
    structs::SPSCQueue<Signal,     2048UL> m_signal_queue;
    structs::SPSCQueue<Order,      2048UL> m_order_queue;

    /// @brief Receives market data
    std::jthread m_network_thread;      

    /// @brief Processes signals with a strategy
    std::jthread m_strategy_thread;    
    
    /// @brief Sends orders
    std::jthread m_execution_thread;

    /// @brief Reads market data from JSON
    std::jthread                    m_reader_thread;

    /// @brief TCP server handle
    TcpServerUniquePtr p_tcp_server;

    /// @brief TCP client handle
    TcpClientUniquePtr p_tcp_client;

    /// @brief Performance counters (lock-free)
    alignas(64) std::atomic<uint64_t> m_ticks_received{0};
    alignas(64) std::atomic<uint64_t> m_signals_generated{0};
    alignas(64) std::atomic<uint64_t> m_orders_sent{0};
    alignas(64) std::atomic<uint64_t> m_queue_full_count{0};

    /// @brief Thread functions
    void _network_loop();
    void _strategy_loop();
    void _execution_loop();

    // Strategy logic
    Signal _compute_signal(const MarketData &md);
    Order  _generate_order(const Signal &signal);

    /// @brief Attempt to set thread affinity. Prints logs if it was unsuccessful.
    /// @param thread 
    /// @param cpu_id 
    void _set_thread_affinity(std::thread::native_handle_type thread, int cpu_id);
public:

    explicit Algo(io::Config&&);
    ~Algo();

    std::expected<void, AlgoError> initialize_client();
    std::expected<void, AlgoError> initialize_server();
    std::expected<void, AlgoError> reconnect_client();

    __always_inline
    bool is_server_initialized() const noexcept
    {
        return m_server_started.load(std::memory_order_relaxed);
    }

    __always_inline
    bool is_client_initialized() const noexcept
    {
        return m_client_started.load(std::memory_order_relaxed);
    }

    __always_inline
    bool is_client_running() const noexcept
    {
        return m_client_running.load(std::memory_order_relaxed);
    }

    // Same as `__always_inline` GCC helper macro
    inline __attribute__ ((__always_inline__)) 
    bool is_client_stopped() const noexcept
    {
        return m_client_stopped.load(std::memory_order_relaxed);
    }

    std::expected<void, AlgoError> work_server();
    std::expected<void, AlgoError> work_client();
    void stop_client();
    void print_client_stats() const;
};
} // End namespace fiah