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

          nativeBuildInputs = pyPkgs ++ (with pkgs; [
            gcc
            # clang-tools
            # clang
            # bear
            # llvm.lldb
            # llvm.clang
            # llvmPackages_18.libcxxStdenv

            # llvmPackages_18.libcxxClang
            # llvmPackages_18.compiler-rt

            # gtest
            # llvmPackages_latest.lldb
            # llvmPackages_latest.libllvm
            # llvmPackages_latest.libcxx
            # llvmPackages_latest.clang

            binutils
            cmake
            gdb 
            # pkg-config 
            boost 
            toml11 
            openssl
            valgrind
            nlohmann_json
            doxygen
            graphviz
            zsh
            grpc
            protobuf
          ]);

          buildInputs = with pkgs; [
            # pkgs.cassandra-cpp-driver
            gcc
            binutils
            cmake
            gdb 
            pkg-config 
            boost 
            toml11 
            openssl
            valgrind
            nlohmann_json
            doxygen
            graphviz
            zsh
            grpc
            protobuf
            # llvm.libcxx
          ];

          # CPATH = builtins.concatStringsSep ":" [
          #   (lib.makeSearchPathOutput "dev" "include" [ llvm.libcxx ])
          #   (lib.makeSearchPath "resource-root/include" [ llvm.clang ])
          # ];
        };
      }
    );
}
# {
#   description = "FastInAHurry";

#   inputs.nixpkgs.url = "github:nixos/nixpkgs/nixos-25.05";

#   outputs = { self, nixpkgs }:
#     let
#       system = "x86_64-linux";
#       pkgs = import nixpkgs {
#         inherit system;
#         config.allowUnfree = true;
#       };
#       pythonPackages = pkgs.python313Packages;
#       pyPkgs = with pythonPackages; [
#         pandas
#         matplotlib
#         numpy
#         plotly
#         seaborn
#       ];
#     in
#     {
#       devShells.${system}.default = 
#         pkgs.mkShell.override { stdenv = pkgs.clangStdenv; } {
#           buildInputs = pyPkgs ++ (with pkgs; [
#             clang
#             llvmPackages_18.lld
#             llvmPackages_18.libllvm
#             llvmPackages_18.libcxx
#             llvmPackages_18.libcxxStden v

#             llvmPackages_18.libcxxClang
#             llvmPackages_18.compiler-rt
#           ]);

#         shell = pkgs.zsh;
#         shellHook = ''
#           echo "Welcome to the FastInAHurry flake dev shell" 
#           export CC=clang
#           export CXX=clang++
#           # clang++ -E -x c++ - -v < /dev/null
#           # export CFLAGS="--stdlib=libc++"
#           # export CXXFLAGS="--stdlib=libc++"
#           # export LDFLAGS="-stdlib=libc++ -lc++abi"
#         '';
#       };
#     };
# }