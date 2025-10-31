#include "Algo.hh"
#include "io/Config.hh"

class Controller {
public:
    Controller() = default;
    explicit Controller(io::Config&&) noexcept(noexcept(std::make_unique<Algo>()));
    Controller(const io::Config&);
    void start_server();
    void start_client();
private:
    std::unique_ptr<Algo> p_algo;
};