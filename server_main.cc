#include <iostream>
#include <sstream>

void RunServer(std::string);
void print_help() {
    std::ostringstream ss;
    ss << "Usage: ./build/server <port>" << std::endl;
    std::cout << ss.str();
}

int main(int argc, char** argv) {
    std::cout << "Starting gRPC market data server..." << std::endl;
    if (argc != 2) {
        print_help();
        return 1;
    };
    RunServer(std::string(argv[1]));

    return 0;
}