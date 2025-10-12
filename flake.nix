{
  description = "FastInAHurry";

  inputs.nixpkgs.url = "github:nixos/nixpkgs/nixos-25.05";

  outputs = { self, nixpkgs }:
    let
      pkgs = import nixpkgs {
        system = "x86_64-linux";
      };
      pythonPackages = pkgs.python313Packages;
      pyPkgs = with pythonPackages; [
        pandas
        matplotlib
        numpy
        plotly
        seaborn
      ];
    in
    {
      devShells.x86_64-linux = {
        default = pkgs.mkShell {
          buildInputs = [ 
            # pyPkgs
            pkgs.gcc
            pkgs.binutils
            pkgs.cmake
            pkgs.gdb
            pkgs.pkg-config
            pkgs.boost
            pkgs.openssl
            pkgs.valgrind
            pkgs.nlohmann_json
            pkgs.doxygen
            pkgs.graphviz
            pkgs.cassandra-cpp-driver
            pkgs.zsh
            pkgs.grpc
            pkgs.protobuf

            # pkgs.streamlit 
            # pkgs.cowsay 
          ];
        };
      };
    };
}