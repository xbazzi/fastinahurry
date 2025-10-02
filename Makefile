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
	cmake --build $(BUILD_DIR)

# Build only the "options" target
options: configure ## Build options target
	cmake --build $(BUILD_DIR) --target options

# Configure step (only runs once unless CMakeLists.txt changes)
configure: ## Configure build directory
	cmake -S . -B $(BUILD_DIR)

# Clean build files
clean:
	rm -rf $(BUILD_DIR)
