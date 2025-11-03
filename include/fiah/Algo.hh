#pragma once

// C++ Includes
#include <memory>
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
#include "fiah/market/MarketData.hh"
#include "fiah/structs/Structs.hh"

namespace fiah {


using Signal = fiah::structs::Signal;
using Order = fiah::structs::Order;

class Algo 
{
private:

    std::unique_ptr<io::Config> p_config;
    static inline utils::Logger<Algo>& m_logger{utils::Logger<Algo>::get_instance()};

    std::atomic<bool> m_server_started{false};
    std::atomic<bool> m_client_started{false};
    std::atomic<bool> m_client_stopping{false};
    std::atomic<bool> m_client_stopped{false};
    std::atomic<bool> m_client_running{false};

    structs::SPSCQueue<MarketData, 4096UL> m_market_data_queue;
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
    std::unique_ptr<io::TcpServer>  p_tcp_server;

    /// @brief TCP client handle
    std::unique_ptr<io::TcpClient>  p_tcp_client;

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

    // Thread affinity helper
    void _set_thread_affinity(std::thread::native_handle_type thread, int cpu_id);

    std::mutex                      _orders_mutex;
    std::mutex                      _send_mutex;
    std::mutex                      _futures_mutex;
    std::jthread                    _network_thread;
    std::vector<std::jthread>       _worker_threads;
    std::vector<std::future<bool>>  _futures;
public:
    explicit Algo(io::Config&&);
    ~Algo();

    std::expected<void, AlgoError> initialize_client();
    std::expected<void, AlgoError> initialize_server();
    bool is_server_initialized() const noexcept;
    bool is_client_initialized() const noexcept;
    bool is_client_stopped() const noexcept;
    void work_server();
    void work_client();
    void start_client();
    void stop_client();
    void print_client_stats() const;
    void process_market_data();
    // grpc::Status send(trading::Order&);
};
} // End namespace fiah