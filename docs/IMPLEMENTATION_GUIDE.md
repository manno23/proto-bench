# Implementation Guide

This guide explains how to implement framework-specific adapters for proto-bench.

## Overview

Each framework requires three main components:

1. **Schema Definition** - IDL/schema files in the framework's format
2. **Client Implementation** - Adapter that implements `IBenchmarkClient`
3. **Server Implementation** - Adapter that implements `IBenchmarkServer`

## Step-by-Step Guide

### 1. Create Schema Definition

First, translate the common schema to your framework's IDL:

**Location**: `frameworks/<framework>/schema/`

**Common Schema**: See `common/schema_definition.md` for the reference

**Example for gRPC** (`frameworks/grpc/schema/benchmark.proto`):
```protobuf
syntax = "proto3";
package benchmark;

service BenchmarkService {
  rpc Echo(EchoRequest) returns (EchoResponse);
  // ... other methods
}

message EchoRequest {
  string message = 1;
  int64 timestamp = 2;
  uint32 sequence_number = 3;
}
```

### 2. Implement Type Conversions

Create conversion functions between framework types and common types:

```cpp
// Convert common type to framework type
::benchmark::EchoRequest ToProto(const common::EchoRequest& req) {
  ::benchmark::EchoRequest proto;
  proto.set_message(req.message);
  proto.set_timestamp(req.timestamp);
  proto.set_sequence_number(req.sequence_number);
  return proto;
}

// Convert framework type to common type
common::EchoResponse FromProto(const ::benchmark::EchoResponse& proto) {
  common::EchoResponse resp;
  resp.message = proto.message();
  resp.client_timestamp = proto.client_timestamp();
  resp.server_timestamp = proto.server_timestamp();
  resp.sequence_number = proto.sequence_number();
  return resp;
}
```

### 3. Implement Service Adapter

Create a class that wraps the framework's client stub and implements `IBenchmarkService`:

```cpp
class GrpcServiceAdapter : public common::IBenchmarkService {
public:
  explicit GrpcServiceAdapter(
      std::unique_ptr<::benchmark::BenchmarkService::Stub> stub)
    : stub_(std::move(stub)) {}

  common::Result<common::EchoResponse> Echo(
      const common::EchoRequest& request) override {

    // Convert to proto
    auto proto_req = ToProto(request);
    ::benchmark::EchoResponse proto_resp;

    // Make RPC call
    grpc::ClientContext context;
    auto status = stub_->Echo(&context, proto_req, &proto_resp);

    // Handle result
    if (status.ok()) {
      return common::Result<common::EchoResponse>(FromProto(proto_resp));
    } else {
      return common::Result<common::EchoResponse>(
          ConvertErrorCode(status.error_code()),
          status.error_message()
      );
    }
  }

private:
  std::unique_ptr<::benchmark::BenchmarkService::Stub> stub_;
};
```

### 4. Implement Client

Implement `IBenchmarkClient` to manage connection and service access:

```cpp
class GrpcBenchmarkClient : public common::IBenchmarkClient {
public:
  GrpcBenchmarkClient() = default;

  bool Connect(const std::string& address) override {
    channel_ = grpc::CreateChannel(
        address,
        grpc::InsecureChannelCredentials()
    );

    auto stub = ::benchmark::BenchmarkService::NewStub(channel_);
    service_ = std::make_unique<GrpcServiceAdapter>(std::move(stub));

    return channel_->GetState(true) == GRPC_CHANNEL_READY;
  }

  void Disconnect() override {
    service_.reset();
    channel_.reset();
  }

  bool IsConnected() const override {
    return channel_ &&
           channel_->GetState(false) == GRPC_CHANNEL_READY;
  }

  common::IBenchmarkService* GetService() override {
    return service_.get();
  }

private:
  std::shared_ptr<grpc::Channel> channel_;
  std::unique_ptr<GrpcServiceAdapter> service_;
};
```

### 5. Implement Server

Implement `IBenchmarkServer` to run the service:

```cpp
class BenchmarkServiceImpl final
    : public ::benchmark::BenchmarkService::Service {
public:
  explicit BenchmarkServiceImpl(
      std::shared_ptr<common::IBenchmarkService> service)
    : service_(service) {}

  grpc::Status Echo(
      grpc::ServerContext* context,
      const ::benchmark::EchoRequest* request,
      ::benchmark::EchoResponse* response) override {

    // Convert to common type
    auto common_req = FromProto(*request);

    // Call service implementation
    auto result = service_->Echo(common_req);

    // Convert result
    if (result.ok()) {
      *response = ToProto(result.value);
      return grpc::Status::OK;
    } else {
      return grpc::Status(
          ConvertErrorCode(result.error_code),
          result.error_message
      );
    }
  }

private:
  std::shared_ptr<common::IBenchmarkService> service_;
};

class GrpcBenchmarkServer : public common::IBenchmarkServer {
public:
  explicit GrpcBenchmarkServer(
      std::shared_ptr<common::IBenchmarkService> service)
    : service_(service) {}

  bool Start(const std::string& address) override {
    grpc::ServerBuilder builder;
    builder.AddListeningPort(address, grpc::InsecureServerCredentials());

    service_impl_ = std::make_unique<BenchmarkServiceImpl>(service_);
    builder.RegisterService(service_impl_.get());

    server_ = builder.BuildAndStart();
    return server_ != nullptr;
  }

  void Stop() override {
    if (server_) {
      server_->Shutdown();
    }
  }

  bool IsRunning() const override {
    return server_ != nullptr;
  }

  void Wait() override {
    if (server_) {
      server_->Wait();
    }
  }

private:
  std::shared_ptr<common::IBenchmarkService> service_;
  std::unique_ptr<BenchmarkServiceImpl> service_impl_;
  std::unique_ptr<grpc::Server> server_;
};
```

### 6. Implement Factory

Create a factory for creating clients and servers:

```cpp
class GrpcFrameworkFactory : public common::IFrameworkFactory {
public:
  std::string GetName() const override {
    return "gRPC";
  }

  std::unique_ptr<common::IBenchmarkClient> CreateClient() override {
    return std::make_unique<GrpcBenchmarkClient>();
  }

  std::unique_ptr<common::IBenchmarkServer> CreateServer(
      std::shared_ptr<common::IBenchmarkService> service) override {
    return std::make_unique<GrpcBenchmarkServer>(service);
  }
};

// Factory function for registration
std::unique_ptr<common::IFrameworkFactory> CreateGrpcFactory() {
  return std::make_unique<GrpcFrameworkFactory>();
}
```

### 7. Update CMakeLists.txt

Ensure your framework's CMakeLists.txt:
- Finds the framework package
- Generates code from schema
- Builds client/server libraries
- Links dependencies

```cmake
find_package(gRPC CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)

# Generate code
add_custom_command(...)

# Build libraries
add_library(benchmark_grpc_proto ${PROTO_SRCS} ${GRPC_SRCS})
add_library(benchmark_grpc_client client/grpc_client.cpp)
add_library(benchmark_grpc_server server/grpc_server.cpp)

# Link dependencies
target_link_libraries(benchmark_grpc_client
  PUBLIC
    benchmark_common
    benchmark_grpc_proto
)
```

### 8. Register in Benchmark Runner

Add factory registration in `benchmarks/benchmark_runner.cpp`:

```cpp
#ifdef HAS_GRPC
extern std::unique_ptr<common::IFrameworkFactory> CreateGrpcFactory();
#endif

// In main():
#ifdef HAS_GRPC
  if (framework == "grpc" || framework == "all") {
    factories.push_back(CreateGrpcFactory());
  }
#endif
```

## Framework-Specific Considerations

### gRPC
- Uses synchronous and async APIs
- Channel management for connection pooling
- Deadline/timeout handling
- Error status codes map well to common ErrorCode

### Cap'n Proto
- Uses promise-based async API (KJ library)
- Capability-based security model
- Need to adapt promise chains to callbacks
- Very efficient binary protocol

### tRPC-cpp
- Similar to gRPC but with Tencent-specific features
- Plugin-based architecture
- Custom service proxy system
- May have different threading model

### oRPC
- Object capability security (unforgeable references)
- May require different connection model
- Investigate schema definition format
- Potentially TypeScript/JavaScript focus

## Testing Your Implementation

### Unit Tests

Create unit tests in `tests/unit/<framework>/`:

```cpp
TEST(GrpcClientTest, ConnectAndDisconnect) {
  auto client = CreateGrpcFactory()->CreateClient();
  EXPECT_TRUE(client->Connect("localhost:50051"));
  EXPECT_TRUE(client->IsConnected());
  client->Disconnect();
  EXPECT_FALSE(client->IsConnected());
}

TEST(GrpcServiceTest, Echo) {
  auto client = CreateGrpcFactory()->CreateClient();
  client->Connect("localhost:50051");

  common::EchoRequest req;
  req.message = "test";
  req.timestamp = GetTimestampNanos();
  req.sequence_number = 1;

  auto result = client->GetService()->Echo(req);
  EXPECT_TRUE(result.ok());
  EXPECT_EQ(result.value.message, "test");
}
```

### Integration Tests

Test cross-framework compatibility in `tests/integration/`:

```cpp
TEST(IntegrationTest, GrpcClientToGrpcServer) {
  // Start server
  auto service = std::make_shared<ReferenceServiceImpl>();
  auto server = CreateGrpcFactory()->CreateServer(service);
  server->Start("localhost:50051");

  // Create client
  auto client = CreateGrpcFactory()->CreateClient();
  client->Connect("localhost:50051");

  // Test RPC
  common::EchoRequest req;
  req.message = "integration test";
  auto result = client->GetService()->Echo(req);

  EXPECT_TRUE(result.ok());
  EXPECT_EQ(result.value.message, req.message);

  // Cleanup
  client->Disconnect();
  server->Stop();
}
```

## Common Pitfalls

### 1. Error Code Mapping
Ensure error codes are properly converted between framework and common types.

### 2. Memory Management
Be careful with ownership - use smart pointers consistently.

### 3. Threading
Framework threading models may differ - ensure thread safety.

### 4. Streaming APIs
Streaming requires careful handling of callbacks and lifecycle.

### 5. Resource Cleanup
Ensure proper cleanup in destructors and Disconnect/Stop methods.

## Reference Implementation

See `frameworks/grpc/` for a reference implementation (when complete).

## Getting Help

- Check existing framework implementations
- Review common interface documentation
- See architecture documentation for design patterns
- Open an issue if you encounter problems

## Checklist

Before submitting your implementation:

- [ ] Schema definition matches common schema
- [ ] Type conversions implemented and tested
- [ ] IBenchmarkClient implemented
- [ ] IBenchmarkServer implemented
- [ ] IFrameworkFactory implemented
- [ ] CMake build configuration added
- [ ] Factory registered in benchmark runner
- [ ] Unit tests added
- [ ] Integration tests added
- [ ] Documentation updated
