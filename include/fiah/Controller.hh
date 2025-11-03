#include "fiah/io/Config.hh"
#include "fiah/utils/Logger.hh"
#include "fiah/Algo.hh"
#include "fiah/memory/unique_ptr.hh"

namespace fiah {

/// @brief Initializes Algo and orchestrates high-level work, such as 
///        starting the server and client comms and strategies.
/// @attention Controller is a termination point, and as such it catches 
///            all exceptions without throwing back to the calling method (usually main).
class Controller {
public:
    Controller() noexcept = default;
    explicit Controller(io::Config&&) noexcept(noexcept(AlgoUniquePtr()));
    explicit Controller(const io::Config&) = delete;
    bool start_server() noexcept;
    bool start_client() noexcept;
private:
    using AlgoUniquePtr = memory::unique_ptr<Algo>;
    AlgoUniquePtr p_algo;
    static inline utils::Logger<Controller>& 
        m_logger{utils::Logger<Controller>::get_instance("Controller")};
};

} // End namespace fiah