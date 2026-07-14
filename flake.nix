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
              ninja
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
              zsh
              include-what-you-use

            ]);

          buildInputs = with pkgs; [
            boost
            toml11
            openssl
            gtest
            gbenchmark
          ];
          shell = pkgs.zsh;
          shellHook = ''
            echo "Welcome to the FastInAHurry flake dev shell"
            export CC=''${CC:-gcc}
            export CXX=''${CXX:-g++}
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
