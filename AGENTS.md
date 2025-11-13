# Agent Guidelines for proto-bench

## Build Commands
- **CMake**: `mkdir build && cd build && cmake .. && make`
- **Single test**: `cd build && ctest -R <test_name>` or `./<test_binary>`
- **Clean**: `cd build && make clean` or `rm -rf build/`

## Code Style
- **Language**: C++17 or newer
- **Formatting**: Use `clang-format` with Google or LLVM style
- **Naming**: `snake_case` for files/functions/variables, `PascalCase` for types
- **Includes**: Group by standard library, third-party, then local headers
- **Error handling**: Use exceptions or status codes (consistent per framework)
- **Comments**: Document RPC interface definitions and benchmark metrics

## Project Structure
- Each RPC framework gets its own directory (e.g., `grpc/`, `capnproto/`)
- Shared benchmark scenarios in `benchmarks/`
- Common utilities in `common/`
- Build scripts at root level

## Testing
- Unit tests for each framework implementation
- Integration tests for cross-framework compatibility
- Performance benchmarks measure: throughput, latency, reliability

## Dependencies
- trpc-cpp, capnproto, grpc, orpc
- CMake 3.14+
- C++ compiler with C++17 support
- Benchmarking tools (Google Benchmark recommended)
