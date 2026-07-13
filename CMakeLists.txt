cmake_minimum_required(VERSION 3.25)
project(FastInAHurry CXX)
enable_testing()
include(CTest)
include(GoogleTest)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
    message("*** Build type not set.  defaulting to Debug")
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(
        -g3
        -Og
        -std=c++23
        -fsanitize=address,undefined,leak
        -fno-omit-frame-pointer
        -Wall
        -Wconversion
        -Wpedantic
        -Werror
        -Wextra
        -Wno-interference-size
    )
    add_link_options(-Wno-interference-size -fsanitize=address,undefined,leak)
else()
    add_compile_options(
        -O3
        -std=c++23
        -Wall
        -Wconversion
        -Wpedantic
        -Werror
        -Wextra
        -march=native
        -Wall
    )
endif()

# Create header-only library interface
add_library(fiah INTERFACE)
target_include_directories(fiah INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(fiah INTERFACE cxx_std_23)

# Compiler flags and other goodies
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(GTest REQUIRED)
find_package(Threads REQUIRED)
include(FetchContent)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

# Put all executables in the same directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Convention for executables and libraries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

message("************************************************************")
message("**** Building in ${CMAKE_BUILD_TYPE} mode ")
message("************************************************************")

# ============================================================================
# Header Compilation Check
# ============================================================================
add_library(fiah_header_check OBJECT check/all_headers.cc)
target_include_directories(fiah_header_check PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(fiah_header_check PRIVATE
    Threads::Threads
)

# ============================================================================
# Testing Configuration
# ============================================================================

# Only build tests if this is the main project (not included via FetchContent)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS
      "${CMAKE_SOURCE_DIR}/tests/*_test.cc"
      "${CMAKE_SOURCE_DIR}/tests/test.cc"
  )

  add_executable(fiah_tests
      ${TEST_SOURCES}
  )

  set_target_properties(fiah_tests PROPERTIES
      CXX_STANDARD 23
      CXX_STANDARD_REQUIRED YES
  )

  target_include_directories(fiah_tests PRIVATE
      ${CMAKE_SOURCE_DIR}/include
      ${CMAKE_SOURCE_DIR}/tests        # For test utilities and headers
      ${CMAKE_CURRENT_BINARY_DIR}      # For generated files
  )

  target_link_libraries(fiah_tests PRIVATE
      GTest::gtest
      GTest::gtest_main
      Threads::Threads
  )

  target_precompile_headers(fiah_tests PRIVATE
      <gtest/gtest.h>
      <iostream>
      <algorithm>
      <ranges>
      <print>
  )

  # Register tests with CTest
  add_test(NAME fiah_tests COMMAND fiah_tests)
  
  # Optional: Use gtest_discover_tests for better test reporting
  gtest_discover_tests(fiah_tests)
endif()

# ============================================================================
# Benchmarking Configuration
# ============================================================================

# Only build benchmarks if this is the main project
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  find_package(benchmark REQUIRED)

  # Collect all benchmark source files
  file(GLOB_RECURSE BENCHMARK_SOURCES
      "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/*_bench.cc"
  )

if(BENCHMARK_SOURCES)
    add_executable(fiah_benchmarks
        ${BENCHMARK_SOURCES}
    )

    set_target_properties(fiah_benchmarks PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED YES
    )

    target_include_directories(fiah_benchmarks PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/benchmarks
        ${CMAKE_CURRENT_BINARY_DIR}
    )

    target_link_libraries(fiah_benchmarks PRIVATE
        benchmark::benchmark
        benchmark::benchmark_main
    )
endif()
endif()

# ============================================================================
# Examples Configuration
# ============================================================================

# Collect all example source files
file(GLOB_RECURSE EXAMPLE_SOURCES
    "${CMAKE_SOURCE_DIR}/examples/*_example.cc"
)

# Build each example as a separate executable
foreach(EXAMPLE_SOURCE ${EXAMPLE_SOURCES})
    get_filename_component(EXAMPLE_NAME ${EXAMPLE_SOURCE} NAME_WE)
    
    add_executable(${EXAMPLE_NAME}
        ${EXAMPLE_SOURCE}
    )

    set_target_properties(${EXAMPLE_NAME} PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED YES
    )

    target_include_directories(${EXAMPLE_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/include
    )

    target_link_libraries(${EXAMPLE_NAME} PRIVATE
        Threads::Threads
    )
endforeach()
