#include "Controller.hh"
// #include <thread>

Controller::Controller(int argc, char* argv[]) 
 : _algo(new Algo()), _count(0) {
}

void Controller::start() {
    if (!_algo->initialized()) _algo->initialize();
    
    _algo->start_market_data_streaming();
    
    while(_algo->is_running()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        _count++;
        if (_count >= 10) _algo->stop();
    }
}
