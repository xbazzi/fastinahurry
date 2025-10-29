#include "Controller.hh"
#include "io/Config.hh"
#include "Algo.hh"
// #include <thread>

Controller::Controller(io::Config&& config) 
    noexcept(noexcept(std::make_unique<Algo>()))
 : p_algo{std::make_unique<Algo>(config)}
{ }

void Controller::start_server() {
    if (!p_algo->is_initialized()) p_algo->initialize();
    p_algo->work_market();
}
void Controller::start_client() {
    if (!p_algo->is_initialized()) p_algo->initialize();
    p_algo->work_client();
}
