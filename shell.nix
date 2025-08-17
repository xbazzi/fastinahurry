{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  name = "fastinahurry-dev";
  buildInputs = with pkgs; [
    openssl
    gcc
    cmake
    gdb
    pkg-config
    boost
    valgrind
    zsh
    grpc
    protobuf
    nlohmann_json
    doxygen
    graphviz
    claude-code
  ];
}
