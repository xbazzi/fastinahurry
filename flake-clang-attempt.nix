{
  description = "Minimal Hello World in C (Clang-based dev shell)";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  };

  outputs = { self, nixpkgs }:
    let
      system = "x86_64-linux";
      # Import nixpkgs with clang as the default stdenv
      pkgs = import nixpkgs {
        inherit system;
        overlays = [
          (final: prev: {
            stdenv = prev.llvmPackages_latest.stdenv;
          })
        ];
      };
    in {
      devShells.${system}.default = pkgs.mkShell {
        buildInputs = with pkgs; [
          clang-tools
          cmake
          python3
          ninja
          llvmPackages_latest.llvm
          llvmPackages_latest.clangWithLibcAndBasicRtAndLibcxx
          binutils
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
        ];

        shellHook = ''
          export CC=clang
          export CXX=clang++
          export CMAKE_BUILD_TYPE=Release
          export PATH=$PATH:${pkgs.llvmPackages_latest.llvm}/bin
          echo "🔧 Entered Clang-based dev shell"
        '';
      };
    };
}


# {
#   description = "LLVM build environment with GCC compatibility flags";

#   inputs = {
#     nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
#   };

#   outputs = { self, nixpkgs }:
#     let
#       system = "x86_64-linux";
#       pkgs = import nixpkgs { inherit system; };
#       gccForLibs = pkgs.stdenv.cc.cc;
#     in {
#       packages.${system}.default = pkgs.stdenv.mkDerivation {
#         pname = "llvm-env";
#         version = "1.0";

#         buildInputs = with pkgs; [
#           bashInteractive
#           python3
#           ninja
#           cmake
#           llvmPackages_latest.llvm
#           llvmPackages_latest.clangWithLibcAndBasicRtAndLibcxx
#           # gcc
#           binutils
#           gdb 
#           pkg-config 
#           boost 
#           toml11 
#           openssl
#           valgrind
#           nlohmann_json
#           doxygen
#           graphviz
#           zsh
#           grpc
#           protobuf
#         ];

#         # where to find libgcc
#         NIX_LDFLAGS = "-L${gccForLibs}/lib/gcc/${pkgs.targetPlatform.config}/${gccForLibs.version}";

#         # teach clang about C startup file locations
#         CFLAGS = "-B${gccForLibs}/lib/gcc/${pkgs.targetPlatform.config}/${gccForLibs.version} -B${pkgs.stdenv.cc.libc}/lib";

#         cmakeFlags = [
#           "-DGCC_INSTALL_PREFIX=${pkgs.gcc}"
#           "-DC_INCLUDE_DIRS=${pkgs.stdenv.cc.libc.dev}/include"
#           "-DCMAKE_C_COMPILER=clang" 
#           "-DCMAKE_CXX_COMPILER=clang++"
#           "-GNinja"
#           "-DCMAKE_BUILD_TYPE=Release"
#           "-DCMAKE_INSTALL_PREFIX=../inst"
#           "-DLLVM_INSTALL_TOOLCHAIN_ONLY=ON"
#           "-DLLVM_ENABLE_PROJECTS=clang"
#           "-DLLVM_ENABLE_RUNTIMES=libcxx;libcxxabi"
#           "-DLLVM_TARGETS_TO_BUILD=host"
#         ];
#       };

#       devShells.${system}.default = pkgs.mkShell {
#         name = "llvm-devshell";
#         buildInputs = with pkgs; [
#           bashInteractive
#           python3
#           ninja
#           cmake
#           llvmPackages_latest.llvm
#           llvmPackages_latest.clangWithLibcAndBasicRtAndLibcxx
#           # gcc
#           binutils
#           gdb 
#           pkg-config 
#           boost 
#           toml11 
#           openssl
#           valgrind
#           nlohmann_json
#           doxygen
#           graphviz
#           zsh
#           grpc
#           protobuf
#         ];
#       };
#     };
# }
