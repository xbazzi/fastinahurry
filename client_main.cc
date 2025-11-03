// C++ Includes
#include <cstdint>
#include <cassert>
#include <csignal>
#include <iostream>
#include <cxxabi.h>
#include <filesystem>
#include <sstream>

// FastInAHurry Includes
#include "fiah/Controller.hh"
#include "fiah/io/Config.hh"
#include "fiah/utils/Timer.hpp"
#include "fiah/utils/Logger.hh"


void print_help()
{
  std::ostringstream ss;
  ss << "Usage: "                     << '\n'
     << "\tclient <config_file_path>" << '\n'
     << "\tEx: client config.toml"    << '\n';
  std::cout << ss.str() << std::endl;
}

std::atomic<bool> g_shutdown{false};

void signal_handler(int signal)
{
  if (signal == SIGINT || signal == SIGTERM)
  {
    std::cout << "\nShutdown signal received..." << std::endl;
    g_shutdown.store(true, std::memory_order_seq_cst);
  }
}

/// @brief Look at this dummy
struct DummyStructForMainLoggerTag
{ };

int main(int argc, char* argv[]) 
{
  #ifdef _LIBCPP_VERSION
    std::cout << "Using libc++ " << _LIBCPP_VERSION << '\n';
  #elif defined(__GLIBCXX__)
    std::cout << "Using libstdc++ " << __GLIBCXX__ << '\n';
  #else
    std::cout << "Unknown standard library\n";
  #endif
  static auto& m_logger{
    fiah::utils::Logger<DummyStructForMainLoggerTag>::get_instance("main") };
  LOG_INFO("Client entrypoint (client_main.cc) started.");

  // FastInAHurry Includes
  #include "fiah/structs/Structs.hh"
  fiah::structs::Signal signal;
  fiah::structs::MarketData md;
  
  if (argc < 2)
  {
    print_help();
    return 1;
  }

  const auto& path = std::filesystem::path(argv[1]);
  if (!std::filesystem::exists(path))
  {
    std::cout << "Path doesn't exist: " << path << '\n';
    return 1;
  }

  fiah::io::Config config{path};
  if (!config.parse_config())
  {
    std::cout << "Unable to parse config" << '\n';
    return 1;
  }

  /// Create this mf on the stack keep a stack
  fiah::Controller ctlr(std::move(config));

  /// Start some work around here
  if(auto rc = ctlr.start_client(); rc == 1)
  {
    LOG_ERROR("Client entrypoint exited with status ", rc);
    return rc;
  }
  LOG_INFO("Client entrypoint exited with status ", 0);
  return 0;
}