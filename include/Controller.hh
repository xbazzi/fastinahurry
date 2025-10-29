#include "Algo.hh"
#include "io/Config.hh"

class Controller {
public:
    explicit Controller(io::Config&&) noexcept(noexcept(std::make_unique<Algo>()));
    void start_server();
    void start_client();
private:
    std::unique_ptr<Algo> p_algo;
};