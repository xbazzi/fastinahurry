#include "Algo.hh"

class Controller {
public:
    explicit Controller(int, char*[]);
    void start();
private:
    std::unique_ptr<Algo> _algo;
    uint32_t _count;
};