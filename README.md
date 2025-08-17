# FastInAHurry
Financial market client/server orders simulator written in C++.

Low latency, data locality, and high performance are paramount.

# Build n' Run
## Build
Install nix (and optionally direnv) and run the `shell.nix` to load all dependencies in your local environment:

```bash
nix-shell
```

Then you can build the project
```bash
make
```

## Run

First, run the server:

```bash
# Suppress output
./build/server > /dev/null 2>&1 &

#OR

# Get hit with stdout
./build/server &
```

Then run the client:
```bash
./build/fastinahurry 
```

If you have direnv, run `direnv allow` and enjoy having your packages loaded automatically; no need to run `nix-shell` everytime you're in a new shell or load another `shell.nix` from another project.