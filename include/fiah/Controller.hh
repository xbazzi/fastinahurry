#include "fiah/Algo.hh"
#include "fiah/io/Config.hh"
#include "fiah/utils/Logger.hh"

namespace fiah {

class Controller {
public:
    Controller() = default;
    explicit Controller(io::Config&&) noexcept(noexcept(std::make_unique<Algo>()));
    Controller(const io::Config&);
    void start_server();
    void start_client();
private:
    std::unique_ptr<Algo> p_algo;
    static inline utils::Logger<Controller>& m_logger{utils::Logger<Controller>::get_instance()};
};

} // End namespace fiah