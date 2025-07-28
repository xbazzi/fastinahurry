// C++ Includes
#include <cstdint>
#include <cassert>
#include <iostream>

// FastInAHurry Includes
#include <Controller.hh>
#include <Algo.hh>

int main(int argc, char* argv[]) {
    for (uint8_t i = 0; i < argc; i++) {
        std::printf("argument[%d]: %s\n", i, argv[i]);
    }
    assert(2 + 2 == 4);
    Controller ctlr(argc, argv);
    ctlr.start();

    return 0;
}