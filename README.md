# proto-bench

A comprehensive benchmarking suite for comparing RPC framework performance in terms of throughput, latency, reliability, and implementation verbosity.

## Overview

proto-bench provides a unified testing framework with a common schema and consistent benchmarking methodology for evaluating different RPC frameworks. All frameworks are tested against the same scenarios using identical workloads to ensure fair comparisons.

### Frameworks Under Test

**Active Benchmarking:**
- [gRPC](https://github.com/grpc/grpc) - Google's high-performance RPC framework
- [Cap'n Proto](https://github.com/capnproto/capnproto) - Fast data interchange with capability-based security
- [tRPC-cpp](https://github.com/trpc-group/trpc-cpp) - Tencent's high-performance RPC framework

**Under Investigation:**
- [oRPC](https://github.com/unnoq/orpc) - Object capability security focused RPC
- Other agent-to-agent interfaces with object capability security properties

## Key Features

- **Framework-Agnostic Interface**: Common C++ API that all frameworks implement
- **Consistent Testing**: Same benchmarks run against all frameworks
- **Comprehensive Metrics**: Throughput, latency (p50/p95/p99), reliability, resource usage
- **Flexible Architecture**: Easy to add new frameworks or test scenarios
- **Conditional Building**: Only builds frameworks that are installed

## Benchmark Scenarios

1. **Echo Benchmark** - Request/response latency testing
   - Measures round-trip time at various message sizes
   - Captures latency percentiles (p50, p95, p99)

2. **Throughput Benchmark** - Streaming performance
   - Server streaming (download)
   - Client streaming (upload)
   - Bidirectional streaming

3. **Reliability Benchmark** - Error handling and stability
   - Connection stability over time
   - Batch processing with failures
   - Timeout and error recovery

## Quick Start

```bash
# Clone and build
git clone <repository-url>
cd proto-bench
mkdir build && cd build
cmake ..
make -j$(nproc)

# Run benchmarks
./bin/benchmark_runner --framework all --scenario echo --duration 10
```

See [docs/GETTING_STARTED.md](docs/GETTING_STARTED.md) for detailed setup instructions.

## Project Structure

```
proto-bench/
├── common/                 # Framework-agnostic API and utilities
├── frameworks/             # Framework-specific implementations
│   ├── grpc/              # gRPC adapter
│   ├── capnproto/         # Cap'n Proto adapter
│   ├── trpc-cpp/          # tRPC-cpp adapter
│   └── orpc/              # oRPC adapter (under investigation)
├── benchmarks/            # Test scenarios and runner
├── tests/                 # Unit and integration tests
└── docs/                  # Documentation
```

## Evaluation Criteria

### 1. Throughput
- Requests per second
- Data transfer rate (MB/s)
- Scalability under load

### 2. Latency
- Mean, min, max response times
- Percentile distribution (p50, p95, p99)
- Tail latency characteristics

### 3. Reliability
- Connection stability
- Error rate under normal conditions
- Error handling and recovery
- Timeout behavior

### 4. Implementation Verbosity
- Lines of code required
- Schema definition complexity
- Client/server implementation effort
- Boilerplate overhead

### 5. Special Features
- Streaming capabilities
- Object capability security (Cap'n Proto, oRPC)
- Protocol efficiency
- Ecosystem and tooling

## Current Status

✅ Project structure and build system
✅ Common interface and schema definitions
✅ Benchmark framework and scenarios
⏳ Framework-specific implementations (in progress)
⏳ Complete test coverage
⏳ oRPC investigation and integration

## Documentation

- [Getting Started Guide](docs/GETTING_STARTED.md) - Setup and usage instructions
- [Architecture Overview](docs/ARCHITECTURE.md) - Design and implementation details
- [Common Schema Definition](common/schema_definition.md) - RPC interface specification

## Building and Testing

### Prerequisites

- CMake 3.14+
- C++17 compatible compiler
- At least one RPC framework installed (gRPC, Cap'n Proto, or tRPC-cpp)

### Build Options

```bash
cmake .. \
  -DBUILD_GRPC=ON \          # Enable gRPC (if available)
  -DBUILD_CAPNPROTO=ON \     # Enable Cap'n Proto (if available)
  -DBUILD_TRPC=ON \          # Enable tRPC-cpp (if available)
  -DBUILD_BENCHMARKS=ON \    # Build benchmark executables
  -DBUILD_TESTS=ON           # Build tests
```

CMake will automatically detect which frameworks are available on your system.

### Running Benchmarks

```bash
# Test specific framework
./bin/benchmark_runner --framework grpc --scenario echo --duration 30

# Test all frameworks
./bin/benchmark_runner --framework all --scenario all --duration 60

# Save results to JSON
./bin/benchmark_runner --output results.json
```

## Contributing

Contributions are welcome! Areas of interest:

1. **Framework Implementations**: Complete the adapter implementations in `frameworks/*/`
2. **New Scenarios**: Add additional benchmark scenarios
3. **oRPC Integration**: Help investigate and integrate oRPC
4. **Documentation**: Improve guides and add examples
5. **Testing**: Add unit and integration tests

## License

[Add your license here]

## References

- gRPC: https://grpc.io/
- Cap'n Proto: https://capnproto.org/
- tRPC-cpp: https://github.com/trpc-group/trpc-cpp
- oRPC: https://github.com/unnoq/orpc
