let 
  config = {
    allowUnfree = true;
    # allowUnfreePredicate = pkg: builtins.elem (builtins.parseDrvName pkg.name).name [
    #   "claude-code"
    # ];
  };
  pkgs = import <nixpkgs> { inherit config; };
in
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
    cassandra-cpp-driver
  ];

  # nixpkgs.config.allowUnfreePredicate = pkg: builtins.elem (lib.getName pkg) [
  #   "claude-code"
  # ];
}
