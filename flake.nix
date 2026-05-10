{
  description = "C and C++";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs =
    {
      self,
      nixpkgs,
      flake-utils,
      ...
    }:
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = import nixpkgs { inherit system; };
        llvm = pkgs.llvmPackages_latest;
        lib = nixpkgs.lib;
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
        # devShell = pkgs.mkShell.override { stdenv = pkgs.clangStdenv; } rec {
        devShell = pkgs.mkShell {
          nativeBuildInputs =
            pyPkgs
            ++ (with pkgs; [
              # Compilers and build tools
              clang
              gcc
              llvm.lldb
              llvm.clang
              llvm.clang-tools
              bear
              binutils
              cmake
              pkg-config

              # Development and debugging tools
              gdb
              valgrind
              doxygen
              graphviz
              nodejs_22
              zsh

              # Testing frameworks
              gtest
              gbenchmark
            ]);

          buildInputs = with pkgs; [
            # Libraries your code links against
            boost
            toml11
            openssl
            nlohmann_json
            grpc
            protobuf
          ];
          shell = pkgs.zsh;
          shellHook = ''
            echo "Welcome to the FastInAHurry flake dev shell" 
            export CC=gcc
            export CXX=g++
          '';

          # LLVM stuff
          # CPATH = builtins.concatStringsSep ":" [
          #   (lib.makeSearchPathOutput "dev" "include" [ llvm.libcxx ])
          #   (lib.makeSearchPath "resource-root/include" [ llvm.clang ])
          # ];
        };
      }
    );
}
