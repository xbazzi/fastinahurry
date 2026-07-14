BUILD_DIR := build
help: ## Show this help message
	@echo "Available build targets"
	@echo ""
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "  \033[36m%-15s\033[0m %s\n", $$1, $$2}'
	@echo ""
	@echo "For example, to build just the options executable:"
	@echo "      > make options"              

# Default target: build everything
all: configure ## Build all targets
	CLICOLOR_FORCE=1 cmake --build $(BUILD_DIR)

# Build only the "options" target
options: configure ## Build options target
	CLICOLOR_FORCE=1 cmake --build $(BUILD_DIR) --target options

check: configure ## Compile all library headers (no tests/examples/benchmarks)
	CLICOLOR_FORCE=1 cmake --build $(BUILD_DIR) --target fiah_header_check

build-test: configure ## Build test target
	CLICOLOR_FORCE=1 cmake --build $(BUILD_DIR) --target fiah_tests

tests: build-test ## Build and run tests (optional: FILTER=TestSuite.TestName)
	./$(BUILD_DIR)/bin/fiah_tests $(if $(FILTER),--gtest_filter=$(FILTER),)

build-bench: configure ## Build benchmark target
	CLICOLOR_FORCE=1 cmake --build $(BUILD_DIR) --target fiah_benchmarks

bench: build-bench ## Build and run benchmarks (optional: FILTER=BenchmarkName)
	./$(BUILD_DIR)/bin/fiah_benchmarks $(if $(FILTER),--benchmark_filter=$(FILTER),)

# Configure step (only runs once unless CMakeLists.txt changes)
$(BUILD_DIR)/CMakeCache.txt: CMakeLists.txt ## Configure build directory
	cmake -S . -B $(BUILD_DIR) -G Ninja -DCMAKE_COLOR_DIAGNOSTICS=ON
	@if [ -f CMakeUserPresets.json ]; then \
		cmake --preset local; \
	else \
		cmake -S . -B $(BUILD_DIR) -G Ninja -DCMAKE_COLOR_DIAGNOSTICS=ON \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=ON; \
	fi

configure: $(BUILD_DIR)/CMakeCache.txt 

# IWYU_BUILD_DIR := $(BUILD_DIR)/iwyu

# # GCC stdlib headers (libstdc++, glibc) — exclude lib/gcc/ to avoid GCC-only intrinsics
# # that conflict with clang's own builtins.
# _IWYU_INCS := $(shell g++ -v -x c++ /dev/null -fsyntax-only 2>&1 | \
#   sed -n '/include <\.\.\.> search/,/End of search list/{//d;s/^ //;p}' | grep -v '/lib/gcc/')
# _IWYU_CXX_FLAGS := $(foreach d,$(_IWYU_INCS),-isystem$(d))

# $(IWYU_BUILD_DIR)/CMakeCache.txt: CMakeLists.txt
# 	cmake -S . -B $(IWYU_BUILD_DIR) -G Ninja \
# 	  -DCMAKE_C_COMPILER=clang \
# 	  -DCMAKE_CXX_COMPILER=clang++ \
# 	  -DCMAKE_BUILD_TYPE=Debug \
# 	  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
# 	  -DCMAKE_COLOR_DIAGNOSTICS=ON \
# 	  -DCMAKE_DISABLE_PRECOMPILE_HEADERS=ON \
# 	  "-DCMAKE_CXX_FLAGS=$(_IWYU_CXX_FLAGS)"

# iwyu: $(IWYU_BUILD_DIR)/CMakeCache.txt ## Run include-what-you-use on all translation units
# 	iwyu_tool.py -p $(IWYU_BUILD_DIR) -- \
# 	  -Wno-error=unknown-warning-option \
# 	  -Xiwyu --no_fwd_decls 2>&1 | tee iwyu.log || true
# 	@echo "Results saved to iwyu.log"

# Clean build files
clean:
	rm -rf $(BUILD_DIR)
