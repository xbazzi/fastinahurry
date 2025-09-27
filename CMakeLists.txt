cmake_minimum_required(VERSION 3.25)
project(grpc_trading CXX)

include(cmake/common.cmake)

find_package(gRPC REQUIRED)
find_package(Protobuf REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(CASS REQUIRED IMPORTED_TARGET cassandra)

include_directories(${Protobuf_INCLUDE_DIRS})

# Proto file
get_filename_component(trading_proto "proto/trading.proto" ABSOLUTE)
get_filename_component(trading_proto_path "${trading_proto}" PATH)

# Generate sources
set(trading_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/trading.pb.cc")
set(trading_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/trading.pb.h")
set(trading_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/trading.grpc.pb.cc")
set(trading_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/trading.grpc.pb.h")

add_custom_command(
      OUTPUT "${trading_proto_srcs}" "${trading_proto_hdrs}" "${trading_grpc_srcs}" "${trading_grpc_hdrs}"
      COMMAND ${_PROTOBUF_PROTOC}
      ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${trading_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${trading_proto}"
      DEPENDS "${trading_proto}"
)

include_directories("${CMAKE_SOURCE_DIR}/include")

# Glob all source files in src/
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.cc")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(client ${SRC_FILES} main.cc)
set_target_properties(client PROPERTIES
  CXX_STANDARD 23
  CXX_STANDARD_REQUIRED YES
)

add_executable(server ${SRC_FILES} server_main.cc)
set_target_properties(server PROPERTIES
  CXX_STANDARD 23
  CXX_STANDARD_REQUIRED YES
)

# add_executable(fastinahurry ${SRC_FILES} main.cc)
# add_executable(fastinahurry ${SRC_FILES} main.cc)

target_link_libraries(client PRIVATE
  trading_grpc_proto
  nlohmann_json::nlohmann_json
  PkgConfig::CASS
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF}
)

target_include_directories(client PRIVATE 
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_CURRENT_BINARY_DIR} # For generated *.pb.h
)

target_link_libraries(server PRIVATE
  trading_grpc_proto
  nlohmann_json::nlohmann_json
  # cassandra-cpp-driver::cassandra-cpp-driver
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF}
)

target_include_directories(server PRIVATE 
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_CURRENT_BINARY_DIR} # For generated *.pb.h
)

target_compile_features(server PRIVATE cxx_std_23)

add_library(trading_grpc_proto
  ${trading_grpc_srcs}
  ${trading_grpc_hdrs}
  ${trading_proto_srcs}
  ${trading_proto_hdrs})

target_compile_features(client PRIVATE cxx_std_23)
add_compile_options(-O1 -g -fno-omit-frame-pointer -fsanitize=address,undefined,leak)
add_link_options(-fsanitize=address,undefined,leak)
