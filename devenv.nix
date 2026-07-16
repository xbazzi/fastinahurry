{ pkgs, ... }:
let
  llvm = pkgs.llvmPackages_latest;
  pythonPackages = pkgs.python313Packages;
in
{
  packages =
    (with pythonPackages; [
      pandas
      matplotlib
      numpy
      plotly
      seaborn
    ])
    ++ [ llvm.lldb llvm.clang llvm.clang-tools ]
    ++ (with pkgs; [
      ninja
      clang
      gcc
      bear
      binutils
      cmake
      pkg-config
      gdb
      valgrind
      doxygen
      graphviz
      zsh
      include-what-you-use
      boost
      toml11
      openssl
      gtest
      gbenchmark
    ]);

  enterShell = ''
    echo "Welcome to the FastInAHurry dev shell"
    export CC=''${CC:-gcc}
    export CXX=''${CXX:-g++}
  '';
}
