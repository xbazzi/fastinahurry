# FastInAHurry🏃‍♀️‍➡️
Financial market streaming and order execution simulator written in C++.

Low latency, data locality, and high performance are paramount.


<mark>Only tested on Linux</mark>, and it  works on any distro as long as you have `nix`. Currently, there is no Windows support. Or ever.

# Build n' Run
## Build
Install nix (and optionally direnv) and run the `flake.nix` to load all dependencies in your local environment:
```bash
nix develop
```

Then you can build the project
```bash
make all
```

## Run

First, run the server:
```bash
./build/bin/server &
```

Then run the client:
```bash
./build/bin/client 
```

If you have direnv, run `direnv allow` and enjoy having your packages loaded automatically; no need to run `nix develop` everytime you're in a new shell or load another `shell.nix`/`flake.nix` from another project.