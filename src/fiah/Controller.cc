#include "fiah/Controller.hh"
#include "fiah/io/Config.hh"
#include "fiah/Algo.hh"
#include "fiah/Algo.hh"

namespace fiah
{
Controller::Controller(io::Config&& config) 
  noexcept(noexcept(std::make_unique<Algo>()))
    : p_algo{std::make_unique<Algo>(std::move(config))}
{}

void Controller::start_server() 
{
    try 
    {
        if (!p_algo->is_server_initialized()) p_algo->initialize_server();
        p_algo->work_server();
    } 
    catch(AlgoException& e) 
    {
        LOG_ERROR("Controller failed to start server in Algo: ", e.what()) ;
    }
    catch(std::exception& e) 
    {
        LOG_ERROR("Controller failed to start client for some reason: ", e.what());
    }
}
void Controller::start_client() 
{
    try 
    {
        if (!p_algo->is_client_initialized()) p_algo->initialize_client();
        p_algo->work_client();
        p_algo->stop_client();

    } catch(AlgoException& e) {
        LOG_ERROR("Controller failed to start client");
    }
}

} // End namespace fiah