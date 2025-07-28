#include "Controller.hh"

Controller::Controller(int argc, char* argv[]) 
 : _algo(new Algo()) {
    std::cout << "argc: " << argc << std::endl;
    for (uint16_t i = 1; i < argc; i++) {
        std::cout << argv[i] << " ";
    } std::cout << std::endl;
}

void Controller::start() {
    if (!_algo->initialized()) _algo->initialize();
    _algo->process();
}
