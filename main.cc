// C++ Includes
#include <cstdint>
#include <cassert>
#include <iostream>

// FastInAHurry Includes
#include <Controller.hh>
#include <Algo.hh>
#include <io/Cassandra.hpp>
#include <io/Config.hh>



int main(int argc, char* argv[]) {

  for (uint8_t i = 0; i < argc; i++) {
      std::printf("argument[%d]: %s\n", i, argv[i]);
  }

  // Create Scylla db, tables
  io::init_db(argc, argv);

  // Load config
  auto path = std::filesystem::path(argv[2]);
  bool result = std::filesystem::exists(path);
  std::cout << "Exists: " << result << std::endl;
  io::Config config(path);
  std::cout << "hi" << std::endl;
  config.parse_config();
  std::cout << "hi" << std::endl;

  Controller ctlr(argc, argv);
  ctlr.start();

  return 0;
}