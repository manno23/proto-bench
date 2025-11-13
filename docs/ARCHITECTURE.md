# Architecture Overview

## Design Philosophy

proto-bench is designed with a **framework-agnostic core** that provides:

1. **Common Interface**: All frameworks implement the same C++ interface
2. **Portable Benchmarks**: Test scenarios work with any framework
3. **Fair Comparison**: Same tests, same measurements across all frameworks
4. **Extensibility**: Easy to add new frameworks or scenarios

## Component Architecture

```
┌─────────────────────────────────────────────────────────┐
│                  Benchmark Runner                        │
│            (benchmarks/benchmark_runner.cpp)             │
└───────────────┬─────────────────────────────────────────┘
                │
                ├──► Benchmark Scenarios
                │    ├─ Echo (Latency)
                │    ├─ Throughput (Streaming)
                │    └─ Reliability (Error Handling)
                │
                ├──► Common Interface Layer
                │    ├─ IBenchmarkService
                │    ├─ IBenchmarkClient
                │    ├─ IBenchmarkServer
                │    └─ IFrameworkFactory
                │
                └──► Framework Implementations
                     ├─ gRPC
                     ├─ Cap'n Proto
                     ├─ tRPC-cpp
                     └─ oRPC
```

## Core Abstractions

### 1. Common Types (benchmark_types.h)

Framework-agnostic message types that mirror the schema definition:

```cpp
struct EchoRequest {
  std::string message;
  int64_t timestamp;
  uint32_t sequence_number;
};

struct EchoResponse {
  std::string message;
  int64_t client_timestamp;
  int64_t server_timestamp;
  uint32_t sequence_number;
};
```

All frameworks must convert between their native types and these common types.

### 2. Service Interface (benchmark_service.h)

Abstract interface that all frameworks implement:

```cpp
class IBenchmarkService {
public:
  virtual Result<EchoResponse> Echo(const EchoRequest& request) = 0;
  virtual void StreamData(...) = 0;
  virtual void UploadData(...) = 0;
  // ... other methods
};
```

### 3. Client/Server Interfaces

```cpp
class IBenchmarkClient {
public:
  virtual IBenchmarkService* GetService() = 0;
  virtual bool Connect(const std::string& address) = 0;
  virtual void Disconnect() = 0;
  virtual bool IsConnected() const = 0;
};

class IBenchmarkServer {
public:
  virtual bool Start(const std::string& address) = 0;
  virtual void Stop() = 0;
  virtual bool IsRunning() const = 0;
  virtual void Wait() = 0;
};
```

### 4. Factory Pattern

Each framework provides a factory for creating clients and servers:

```cpp
class IFrameworkFactory {
public:
  virtual std::string GetName() const = 0;
  virtual std::unique_ptr<IBenchmarkClient> CreateClient() = 0;
  virtual std::unique_ptr<IBenchmarkServer> CreateServer(
      std::shared_ptr<IBenchmarkService> service) = 0;
};
```

## Framework Adapter Pattern

Each framework implementation follows this pattern:

```
Schema Definition → Code Generation → Adapter Layer → Common Interface
```

### Example: gRPC Adapter

```
benchmark.proto
    ↓ (protoc + grpc plugin)
benchmark.pb.h + benchmark.grpc.pb.h
    ↓ (adapter implementation)
GrpcBenchmarkClient : IBenchmarkClient
GrpcBenchmarkServer : IBenchmarkServer
```

The adapter layer:
1. Converts between protobuf messages and common types
2. Wraps gRPC-specific error handling
3. Adapts streaming APIs to common callback interface
4. Manages gRPC channels and servers

## Data Flow

### Request/Response (Echo)

```
Benchmark Scenario
    ↓ EchoRequest (common type)
IBenchmarkClient::GetService()->Echo(request)
    ↓ Convert to framework type
gRPC/CapnProto/tRPC client stub
    ↓ Network (protocol-specific)
gRPC/CapnProto/tRPC server
    ↓ Convert to common type
IBenchmarkService::Echo(request)
    ↓ Business logic
Return EchoResponse (common type)
    ↓ Convert to framework type
Network response
    ↓ Convert to common type
Result<EchoResponse> returned to scenario
```

### Streaming

Streaming uses callbacks to avoid framework-specific async models:

```cpp
// Server streaming
void StreamData(
    const StreamRequest& request,
    StreamCallback<DataChunk> on_chunk,      // Called per chunk
    CompletionCallback on_complete)           // Called when done

// Client streaming
void UploadData(
    StreamCallback<DataChunk>& chunk_provider, // Iterator for chunks
    ResponseCallback<UploadResponse> on_complete)
```

## Benchmark Execution Flow

```
1. Parse command line arguments
   ↓
2. Create benchmark scenarios (echo, throughput, reliability)
   ↓
3. Register framework factories
   ↓
4. For each framework:
      For each scenario:
         Create client
         ↓
         Connect to server
         ↓
         Run warm-up phase
         ↓
         Execute benchmark
         ↓
         Collect metrics
         ↓
         Disconnect
   ↓
5. Aggregate and display results
   ↓
6. Export to JSON (optional)
```

## Metrics Collection

### Latency Tracking

```cpp
auto start = GetTimestampNanos();
auto result = service->Echo(request);
auto end = GetTimestampNanos();
latency_stats.AddSample(end - start);
```

Calculated metrics:
- Min, Max, Mean
- Percentiles (P50, P95, P99)
- Standard deviation

### Throughput Tracking

```cpp
total_bytes += request.size() + response.size();
duration_ns = end_time - start_time;
throughput_mbps = CalculateThroughputMBps(total_bytes, duration_ns);
```

### Reliability Tracking

```cpp
if (result.ok()) {
  successful_requests++;
} else {
  failed_requests++;
  // Log error details
}
success_rate = successful_requests / total_requests;
```

## Build System Architecture

### CMake Configuration Hierarchy

```
CMakeLists.txt (root)
    ├─ Find framework packages (gRPC, CapnProto, tRPC)
    ├─ Set HAS_<FRAMEWORK> variables
    │
    ├─ common/CMakeLists.txt
    │   └─ Build benchmark_common library
    │
    ├─ frameworks/grpc/CMakeLists.txt (if HAS_GRPC)
    │   ├─ Generate protobuf/gRPC code
    │   └─ Build benchmark_grpc_{client,server}
    │
    ├─ frameworks/capnproto/CMakeLists.txt (if HAS_CAPNPROTO)
    │   ├─ Generate Cap'n Proto code
    │   └─ Build benchmark_capnproto_{client,server}
    │
    ├─ frameworks/trpc-cpp/CMakeLists.txt (if HAS_TRPC)
    │   ├─ Generate tRPC code
    │   └─ Build benchmark_trpc_{client,server}
    │
    └─ benchmarks/CMakeLists.txt
        ├─ Build benchmark_scenarios library
        └─ Build benchmark_runner executable
            └─ Link available frameworks
```

### Conditional Compilation

The benchmark runner uses preprocessor definitions:

```cpp
#ifdef HAS_GRPC
  factories.push_back(CreateGrpcFactory());
#endif

#ifdef HAS_CAPNPROTO
  factories.push_back(CreateCapnProtoFactory());
#endif

#ifdef HAS_TRPC
  factories.push_back(CreateTrpcFactory());
#endif
```

This allows building with any subset of frameworks.

## Extension Points

### Adding a New Framework

1. **Schema**: Create schema definition in `frameworks/<name>/schema/`
2. **Adapter**: Implement client/server adapters
3. **Factory**: Implement `IFrameworkFactory`
4. **CMake**: Add detection and build logic
5. **Register**: Add to benchmark runner

### Adding a New Scenario

1. **Inherit**: Extend `BenchmarkScenario`
2. **Implement**: Override `Run()` method
3. **Register**: Add factory function
4. **Link**: Add to CMakeLists.txt

### Adding Custom Metrics

1. **Extend** `BenchmarkResults` struct
2. **Collect** in scenario `Run()` method
3. **Display** in `Print()` and `ToJSON()` methods

## Performance Considerations

### Minimal Overhead

- Common types use standard C++ containers
- Virtual function overhead is minimal (one vtable lookup per RPC)
- Type conversion happens at framework boundary only
- No reflection or serialization in common layer

### Memory Management

- Smart pointers for ownership (unique_ptr, shared_ptr)
- Move semantics for large data transfers
- Preallocated buffers where possible

### Threading

- Benchmark scenarios are single-threaded by default
- Multi-threaded scenarios can be added
- Framework-specific threading handled internally

## Security Considerations

### Object Capability Security

Cap'n Proto and oRPC support capability-based security:
- Each RPC is an unforgeable capability
- No ambient authority
- Fine-grained access control

The common interface supports this model through:
- Factory creates clients with specific capabilities
- No global service registry
- Explicit connection management

## Future Enhancements

1. **Async API**: Add fully async interface option
2. **Streaming Improvements**: Better bidirectional streaming support
3. **Resource Monitoring**: CPU/memory tracking during benchmarks
4. **Distributed Testing**: Multi-machine benchmark coordination
5. **Protocol Analysis**: Deep packet inspection and analysis
6. **Custom Transports**: Support for different transport layers
