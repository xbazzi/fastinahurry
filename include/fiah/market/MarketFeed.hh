#pragma once

// C++ Includes
#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <expected>
#include <thread>

// FastInAHurry includes
#include "fiah/error/Error.hh"
#include "fiah/structs/Structs.hh"
#include "quick/handle/UniquePtr.hh"
#include "quick/io/Config.hh"
#include "quick/io/TcpClient.hh"
#include "quick/structs/SPSCQueue.hh"
#include "quick/utils/Logger.hh"
#include "quick/utils/Timer.hh"

namespace fiah::io
{

/// @brief Handles incoming market data from the network
///
/// This class encapsulates all logic related to receiving market data:
/// - TCP connection lifecycle (connect, disconnect, reconnect)
/// - Receiving raw bytes and parsing into MarketData structs
/// - Pushing data into a lock-free queue for downstream processing
/// - Connection recovery with exponential backoff
/// - Performance metrics tracking
///
/// @section thread_safety Thread Safety
/// - receive_loop() is designed to be called from a dedicated thread
/// - All public methods are thread-safe through atomic operations
/// - The queue reference must remain valid for the lifetime of this object
///
/// @section error_handling Error Handling
/// - initialize() returns std::expected for recoverable connection errors
/// - receive_loop() catches all exceptions and logs them (never throws)
/// - Connection failures trigger automatic reconnection logic
class MarketFeed
{
  private:
    using MarketData = structs::MarketData;
    using TcpClientUniquePtr = quick::handle::UniquePtr<quick::io::TcpClient>;

    /// @brief Configuration reference
    const quick::io::Config &m_config;

    /// @brief TCP client for receiving market data
    TcpClientUniquePtr p_tcp_client;

    /// @brief Reference to market data queue (owned by caller)
    quick::structs::SPSCQueue<MarketData, 4096UL> &m_market_data_queue;

    /// @brief Initialization state
    std::atomic<bool> m_initialized{false};

    /// @brief Performance counter: total ticks received
    alignas(64) std::atomic<uint64_t> m_ticks_received{0};

    /// @brief Performance counter: queue full events
    alignas(64) std::atomic<uint64_t> m_queue_full_count{0};

    /// @brief Logger instance
    static inline quick::utils::Logger<MarketFeed> &m_logger{
        quick::utils::Logger<MarketFeed>::get_instance("MarketFeed")};

    /// @brief Internal reconnection logic
    /// @return Success or CoreError::SERVER_NOT_ONLINE
    std::expected<void, CoreError> _reconnect();

  public:
    /// @brief Construct a MarketFeed
    /// @param config Configuration containing market server details
    /// @param queue Reference to the market data queue to push into
    explicit MarketFeed(const quick::io::Config &config, quick::structs::SPSCQueue<MarketData, 4096UL> &queue);

    /// @brief Destructor
    ~MarketFeed();

    // Delete copy/move to avoid queue reference issues
    MarketFeed(const MarketFeed &) = delete;
    MarketFeed &operator=(const MarketFeed &) = delete;
    MarketFeed(MarketFeed &&) = delete;
    MarketFeed &operator=(MarketFeed &&) = delete;

    /// @brief Initialize and connect to the market server
    /// @return Success or CoreError::SERVER_NOT_ONLINE
    std::expected<void, CoreError> initialize();

    /// @brief Main receive loop - call from a dedicated thread
    /// @param running_flag Atomic flag to control loop lifetime
    ///
    /// This method:
    /// - Receives raw bytes from TCP socket
    /// - Parses into MarketData structs
    /// - Pushes to the queue
    /// - Handles reconnection on connection loss
    /// - Never throws (catches all exceptions)
    void receive_loop(std::atomic<bool> &running_flag);

    /// @brief Stop the feed and clean up resources
    void stop();

    /// @brief Get total ticks received
    __always_inline std::uint64_t ticks_received() const noexcept
    {
        return m_ticks_received.load(std::memory_order_relaxed);
    }

    /// @brief Get total queue full events
    __always_inline uint64_t queue_full_count() const noexcept
    {
        return m_queue_full_count.load(std::memory_order_relaxed);
    }

    /// @brief Check if feed is initialized and connected
    __always_inline bool is_initialized() const noexcept
    {
        return m_initialized.load(std::memory_order_relaxed);
    }
};

// ============================================================================
// Inline Implementation
// ============================================================================

inline MarketFeed::MarketFeed(const quick::io::Config &config, quick::structs::SPSCQueue<MarketData, 4096UL> &queue)
    : m_config(config), m_market_data_queue(queue)
{
    LOG_DEBUG("MarketFeed constructed");
}

inline MarketFeed::~MarketFeed()
{
    stop();
    LOG_DEBUG("MarketFeed destroyed");
}

inline auto MarketFeed::initialize() -> std::expected<void, CoreError>
{
    quick::utils::Timer timer{"MarketFeed::initialize()"};

    if (is_initialized())
    {
        LOG_WARN("MarketFeed already initialized");
        return {};
    }

    const std::string &market_ip = m_config.get_market_ip();
    std::uint16_t market_port = m_config.get_market_port();

    p_tcp_client = quick::handle::make_unique<quick::io::TcpClient>(market_ip, market_port);

    LOG_INFO("MarketFeed initialized with market server ip: ", market_ip, ", and port: ", market_port);

    auto connect_result = p_tcp_client->connect_to_server();
    if (!connect_result.has_value())
    {
        LOG_ERROR("Couldn't connect to market server during initialization. ", "Maybe the server is not online yet.");
        p_tcp_client.reset();
        m_initialized.store(false, std::memory_order_release);
        return std::unexpected(CoreError::SERVER_NOT_ONLINE);
    }

    m_initialized.store(true, std::memory_order_release);
    LOG_INFO("MarketFeed initialized and connected to market.");
    return {};
}

inline auto MarketFeed::_reconnect() -> std::expected<void, CoreError>
{
    LOG_INFO("Attempting to reconnect MarketFeed...");

    const std::string &market_ip = m_config.get_market_ip();
    std::uint16_t market_port = m_config.get_market_port();

    p_tcp_client.reset();
    m_initialized.store(false, std::memory_order_release);

    p_tcp_client = quick::handle::make_unique<quick::io::TcpClient>(market_ip, market_port);

    auto connect_result = p_tcp_client->connect_to_server();
    if (!connect_result.has_value())
    {
        LOG_WARN("Reconnection attempt failed - server not available");
        p_tcp_client.reset();
        return std::unexpected{CoreError::SERVER_NOT_ONLINE};
    }

    m_initialized.store(true, std::memory_order_release);
    LOG_INFO("MarketFeed successfully reconnected to market.");
    return {};
}

inline void MarketFeed::receive_loop(std::atomic<bool> &running_flag)
{
    using namespace std::chrono_literals;

    try
    {
        LOG_INFO("MarketFeed receive loop started");
        std::byte buffer[sizeof(MarketData)];

        while (running_flag.load(std::memory_order_acquire))
        {
            quick::utils::Timer timer("MarketFeed::receive_loop");

            if (!p_tcp_client)
            {
                LOG_WARN("TCP client is null - attempting reconnection...");

                int retry_count = 0;
                const int max_retries = 5;

                while (retry_count < max_retries && running_flag.load(std::memory_order_acquire))
                {
                    auto reconnect_result = _reconnect();
                    if (reconnect_result.has_value())
                    {
                        LOG_INFO("Reconnection successful!");
                        break;
                    }

                    retry_count++;
                    if (retry_count < max_retries)
                    {
                        auto backoff = std::chrono::milliseconds(100 * (1 << retry_count));
                        LOG_WARN("Reconnection attempt ", retry_count, " failed. Retrying in ", backoff.count(),
                                 "ms...");
                        std::this_thread::sleep_for(backoff);
                    }
                }

                if (!p_tcp_client)
                {
                    LOG_ERROR("Failed to reconnect after ", max_retries, " attempts. Thread exiting.");
                    running_flag.store(false, std::memory_order_release);
                    return;
                }

                continue;
            }

            auto recv_result = p_tcp_client->recv(buffer, sizeof(buffer));
            if (!recv_result.has_value())
            {
                LOG_WARN("Failed to receive market data packet. "
                         "Socket disconnected - will attempt reconnect.");
                p_tcp_client.reset();
                m_initialized.store(false, std::memory_order_release);
                continue;
            }

            if (recv_result.value() != sizeof(MarketData))
            {
                LOG_WARN("Received incomplete market data packet. Discarding...");
                continue;
            }

            MarketData md;
            std::memcpy(&md, buffer, sizeof(MarketData));
            m_ticks_received.fetch_add(1, std::memory_order_relaxed);

            LOG_DEBUG("Got md (raw): ", "Symbol: ", md.symbol, ", ", "Seq: ", md.seq_num, ", ", "Ask: ", md.ask, ", ",
                      "Bid: ", md.bid, ", ", "Timestamp: ", static_cast<uint64_t>(md.timestamp_ns), ", ",
                      "ticksReceived: ", m_ticks_received.load(std::memory_order_acquire));

            if (!m_market_data_queue.push(md))
            {
                m_queue_full_count.fetch_add(1, std::memory_order_relaxed);

                while (running_flag.load(std::memory_order_acquire))
                {
                    if (m_market_data_queue.push(md))
                        break;
                    std::this_thread::yield();
                }
            }
        }
        LOG_INFO("MarketFeed receive loop exiting...");
    }
    catch (const std::exception &e)
    {
        LOG_ERROR("MarketFeed receive loop crashed with exception: ", e.what());
        running_flag.store(false, std::memory_order_release);
    }
    catch (...)
    {
        LOG_ERROR("MarketFeed receive loop crashed with unknown exception");
        running_flag.store(false, std::memory_order_release);
    }
}

inline void MarketFeed::stop()
{
    LOG_INFO("Stopping MarketFeed...");

    if (p_tcp_client)
    {
        p_tcp_client.reset();
    }

    m_initialized.store(false, std::memory_order_release);

    LOG_INFO("MarketFeed stopped. Stats: ", "Ticks received: ", m_ticks_received.load(std::memory_order_relaxed),
             ", Queue full events: ", m_queue_full_count.load(std::memory_order_relaxed));
}

} // namespace fiah::io
