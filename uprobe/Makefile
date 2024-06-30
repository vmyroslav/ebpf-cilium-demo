BIN_DIR := ./../../bin
APP_DIR := server
TRACER_DIR := tracer
APP_BINARY := $(BIN_DIR)/uprobe_app
TRACER_BINARY := $(BIN_DIR)/uprobe_trace

# Default target to build everything
all: clean download generate build-app build-tracer

# Build the application binary
build-app:
	cd $(APP_DIR) && go build -o $(APP_BINARY) ./server.go

# Generate BPF additional files required by the tracer
generate:
	cd $(TRACER_DIR) && go generate

# Download dependencies for the tracer
download:
	cd $(TRACER_DIR) && go mod download

# Build the tracer binary
build-tracer:
	cd $(TRACER_DIR) && go build -o $(TRACER_BINARY) main.go bpf_bpfel.go

# Clean up generated binaries
clean:
	rm -f $(APP_BINARY)
	rm -f $(TRACER_BINARY)

# Run the application
run-app:
	$(APP_BINARY)

# Run the tracer with sudo privileges
run-trace:
	sudo $(TRACER_BINARY)

# Read the debug tracing information
debug:
	sudo cat /sys/kernel/debug/tracing/trace_pipe

# Display help information
help:
	@echo "Makefile commands:"
	@echo "  all            - Clean, download dependencies, generate files, build app and tracer"
	@echo "  build-app      - Build the Go application binary"
	@echo "  generate       - Generate any additional files required by the tracer"
	@echo "  download       - Download dependencies for the tracer"
	@echo "  build-tracer   - Build the tracer binary"
	@echo "  clean          - Remove generated binaries"
	@echo "  run            - Run the tracer with sudo privileges"
	@echo "  debug          - Read and output the debug tracing information"
	@echo "  help           - Display this help information"