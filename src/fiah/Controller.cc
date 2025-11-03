// FastInAHurry Includes
#include "fiah/utils/Timer.hpp"
#include "fiah/utils/Logger.hh"
#include "fiah/Controller.hh"
#include "fiah/io/Config.hh"
#include "fiah/Algo.hh"
#include "fiah/memory/unique_ptr.hh"

namespace fiah
{

    Controller::Controller(io::Config &&config) 
      noexcept(noexcept(AlgoUniquePtr()))
        : p_algo{memory::make_unique<Algo>(std::move(config))}
    {}

    bool Controller::start_server() 
      noexcept
    {
        try
        {
            if (!p_algo->is_server_initialized())
                p_algo->initialize_server();
            p_algo->work_server();
            return 0;
        }
        catch (AlgoException &e)
        {
            LOG_ERROR("Controller failed to start server (AlgoException): ", e.what());
            return 1;
        }
        catch (std::exception &e)
        {
            LOG_ERROR("Controller failed to start server (std::exception): ", e.what());
            return 1;
        }
    }

    bool Controller::start_client()
      noexcept
    {
        try
        {
            if (!p_algo->is_client_initialized())
                p_algo->initialize_client();
            p_algo->start_client();
            p_algo->stop_client();
            return 0;
        }
        catch (AlgoException &e)
        {
            LOG_ERROR("Controller failed to start client (AlgoException): ", e.what());
            return 1;
        }
        catch (std::exception &e)
        {
            LOG_ERROR("Controller failed to start client (std::exception): ", e.what());
            return 1;
        }
    }

} // End namespace fiah