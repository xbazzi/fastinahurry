// C++ Includes
#include <cstdint>
#include <cassert>
#include <iostream>
#include <cxxabi.h>
#include <filesystem>

// FastInAHurry Includes
#include "Controller.hh"
#include "io/Config.hh"

// void RunServer(std::string);
void print_help()
{
  std::ostringstream ss;
  ss << "Usage: "                     << '\n'
     << "\tclient <config_file_path>" << '\n'
     << "\tEx: client config.toml"    << '\n';
  std::cout << ss.str() << std::endl;
}

int main(int argc, char* argv[]) 
{
  #ifdef _LIBCPP_VERSION
    std::cout << "Using libc++ " << _LIBCPP_VERSION << '\n';
  #elif defined(__GLIBCXX__)
    std::cout << "Using libstdc++ " << __GLIBCXX__ << '\n';
  #else
    std::cout << "Unknown standard library\n";
  #endif

  if (argc < 2)
  {
    print_help();
    return 1;
  }
  // Load config
  const auto& path = std::filesystem::path(argv[1]);
  if (!std::filesystem::exists(path))
  {
    std::cout << "Path doesn't exist: " << path << '\n';
    return 1;
  }

  io::Config config{path};
  if (!config.parse_config())
  {
    std::cout << "Unable to parse config" << '\n';
    return 1;
  }

  Controller ctlr(std::move(config));
  ctlr.start_server();

  return 0;
}