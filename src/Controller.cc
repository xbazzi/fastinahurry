#include "Controller.hh"
// #include <thread>

Controller::Controller(int argc, char* argv[]) 
 : _algo(new Algo()), _count(0) {
    std::cout << "argc: " << argc << std::endl;
    for (uint16_t i = 1; i < argc; i++) {
        std::cout << argv[i] << " ";
    } std::cout << std::endl;
}

void Controller::start() {
    if (!_algo->initialized()) _algo->initialize();
    while(_algo->is_running()) {
        _algo->process();
        std::this_thread::sleep_for(std::chrono::seconds(3));
        _count++;
        if (_count > 5) _algo->stop();
    }
}
