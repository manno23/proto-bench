# Getting Started with proto-bench

## Overview

proto-bench is a benchmarking suite for comparing RPC framework performance. It provides a common interface and test scenarios for evaluating:

- **Throughput**: Requests per second and data transfer rates
- **Latency**: Response times (p50, p95, p99)
- **Reliability**: Error rates and connection stability
- **Implementation Complexity**: Lines of code and verbosity

## Supported Frameworks

- **gRPC** - Google's high-performance RPC framework
- **Cap'n Proto** - Fast data interchange with capability-based security
- **tRPC-cpp** - Tencent's high-performance RPC framework
- **oRPC** - (Under investigation) Object capability security focused

## Project Structure

```
proto-bench/
├── common/                  # Framework-agnostic API and utilities
│   ├── include/            # Common headers
│   │   ├── benchmark_types.h       # Common message types
│   │   ├── benchmark_service.h     # Service interfaces
│   │   └── benchmark_utils.h       # Utility functions
│   └── src/                # Common implementations
│
├── frameworks/             # Framework-specific implementations
│   ├── grpc/              # gRPC implementation
│   │   ├── schema/        # Protocol buffer definitions
│   │   ├── client/        # Client implementation
│   │   └── server/        # Server implementation
│   ├── capnproto/         # Cap'n Proto implementation
│   ├── trpc-cpp/          # tRPC-cpp implementation
│   └── orpc/              # oRPC implementation
│
├── benchmarks/            # Benchmark scenarios
│   ├── scenarios/         # Test scenarios (echo, throughput, reliability)
│   └── benchmark_runner.cpp  # Main benchmark executable
│
├── tests/                 # Unit and integration tests
│   ├── unit/             # Unit tests per framework
│   └── integration/      # Cross-framework tests
│
└── docs/                  # Documentation
```

## Prerequisites

### Build Tools
- CMake 3.14 or higher
- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- Git

### Framework Dependencies

#### gRPC
```bash
# Ubuntu/Debian
sudo apt-get install -y libgrpc++-dev libprotobuf-dev protobuf-compiler-grpc

# macOS
brew install grpc protobuf

# Or build from source:
git clone --recurse-submodules -b v1.58.0 https://github.com/grpc/grpc
cd grpc
mkdir -p cmake/build
cd cmake/build
cmake ../..
make -j$(nproc)
sudo make install
```

#### Cap'n Proto
```bash
# Ubuntu/Debian
sudo apt-get install -y capnproto libcapnp-dev

# macOS
brew install capnp

# Or build from source:
curl -O https://capnproto.org/capnproto-c++-0.10.4.tar.gz
tar zxf capnproto-c++-0.10.4.tar.gz
cd capnproto-c++-0.10.4
./configure
make -j$(nproc) check
sudo make install
```

#### tRPC-cpp
```bash
# Build from source:
git clone https://github.com/trpc-group/trpc-cpp.git
cd trpc-cpp
mkdir build && cd build
cmake ..
make -j$(nproc)
sudo make install
```

## Building proto-bench

### Quick Start

```bash
# Clone the repository
git clone <repository-url>
cd proto-bench

# Create build directory
mkdir build
cd build

# Configure (auto-detects available frameworks)
cmake ..

# Build
make -j$(nproc)

# Run tests
ctest
```

### Build Options

Control which frameworks to build:

```bash
cmake .. \
  -DBUILD_GRPC=ON \
  -DBUILD_CAPNPROTO=ON \
  -DBUILD_TRPC=ON \
  -DBUILD_ORPC=OFF \
  -DBUILD_BENCHMARKS=ON \
  -DBUILD_TESTS=ON
```

CMake will automatically detect which frameworks are available and skip those that aren't installed.

## Running Benchmarks

### Basic Usage

```bash
# From the build directory
./bin/benchmark_runner --framework grpc --scenario echo --duration 10
```

### Command Line Options

- `--framework <name>` - Framework to test (grpc|capnproto|trpc|all)
- `--scenario <name>` - Scenario to run (echo|throughput|reliability|all)
- `--duration <seconds>` - Test duration in seconds (default: 10)
- `--message-size <bytes>` - Message payload size (default: 1024)
- `--address <addr>` - Server address (default: localhost:50051)
- `--output <file>` - Save JSON results to file
- `--verbose` - Enable verbose output

### Example Benchmark Runs

```bash
# Test all frameworks with echo scenario
./bin/benchmark_runner --framework all --scenario echo --duration 30

# Test gRPC throughput with large messages
./bin/benchmark_runner \
  --framework grpc \
  --scenario throughput \
  --message-size 65536 \
  --duration 60

# Full benchmark suite with output to JSON
./bin/benchmark_runner \
  --framework all \
  --scenario all \
  --duration 120 \
  --output results.json
```

## Benchmark Scenarios

### Echo Benchmark
Tests basic request/response latency with various message sizes. Measures:
- Round-trip latency (min, max, mean, p50, p95, p99)
- Requests per second
- Basic throughput

### Throughput Benchmark
Tests streaming performance:
- Server streaming (download throughput)
- Client streaming (upload throughput)
- Bidirectional streaming

### Reliability Benchmark
Tests error handling and stability:
- Connection stability over time
- Batch processing with failures
- Timeout handling
- Error recovery

## Next Steps

1. **Implement Framework Adapters**: The framework-specific client/server implementations need to be completed. See stubs in `frameworks/*/client/` and `frameworks/*/server/`.

2. **Add Service Implementation**: Create a reference service implementation that responds to benchmark requests.

3. **Extend Scenarios**: Add more benchmark scenarios based on your specific needs.

4. **Add Tests**: Implement unit tests for each framework adapter.

## Contributing

When adding a new framework:

1. Create schema definition in `frameworks/<framework>/schema/`
2. Implement `IBenchmarkClient` in `frameworks/<framework>/client/`
3. Implement `IBenchmarkServer` in `frameworks/<framework>/server/`
4. Add CMake configuration in `frameworks/<framework>/CMakeLists.txt`
5. Register factory in `benchmarks/benchmark_runner.cpp`

See existing framework implementations for reference.

## Troubleshooting

### Framework Not Detected

If CMake doesn't detect an installed framework:

```bash
# Check CMake can find the package
cmake .. -DCMAKE_PREFIX_PATH=/path/to/framework/install

# Or set PKG_CONFIG_PATH
export PKG_CONFIG_PATH=/path/to/framework/lib/pkgconfig:$PKG_CONFIG_PATH
cmake ..
```

### Build Errors

- Ensure all dependencies are installed
- Check C++ compiler version (needs C++17 support)
- Try cleaning the build directory: `rm -rf build && mkdir build`

### Runtime Errors

- Verify server address is reachable
- Check firewall settings
- Ensure no port conflicts (default: 50051)

## Additional Resources

- [gRPC Documentation](https://grpc.io/docs/)
- [Cap'n Proto Documentation](https://capnproto.org/)
- [tRPC-cpp Documentation](https://github.com/trpc-group/trpc-cpp)
