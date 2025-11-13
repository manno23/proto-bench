#pragma once

#include "benchmark_types.h"
#include <functional>
#include <memory>

namespace benchmark {
namespace common {

// Callback types for async operations and streaming
template<typename T>
using ResponseCallback = std::function<void(const Result<T>&)>;

template<typename T>
using StreamCallback = std::function<void(const T&)>;

using CompletionCallback = std::function<void(ErrorCode, const std::string&)>;

// Abstract interface for the benchmark service
// All framework implementations must provide this interface
class IBenchmarkService {
public:
  virtual ~IBenchmarkService() = default;

  // Simple request/response for latency testing
  virtual Result<EchoResponse> Echo(const EchoRequest& request) = 0;

  // Async version
  virtual void EchoAsync(
      const EchoRequest& request,
      ResponseCallback<EchoResponse> callback) = 0;

  // Server streaming for throughput testing
  virtual void StreamData(
      const StreamRequest& request,
      StreamCallback<DataChunk> on_chunk,
      CompletionCallback on_complete) = 0;

  // Client streaming for upload throughput testing
  virtual void UploadData(
      StreamCallback<DataChunk>& chunk_provider,
      ResponseCallback<UploadResponse> on_complete) = 0;

  // Bidirectional streaming
  virtual void BidirectionalStream(
      StreamCallback<DataChunk>& chunk_provider,
      StreamCallback<DataChunk> on_chunk,
      CompletionCallback on_complete) = 0;

  // Batch processing for reliability testing
  virtual Result<BatchResponse> BatchProcess(const BatchRequest& request) = 0;

  // Async version
  virtual void BatchProcessAsync(
      const BatchRequest& request,
      ResponseCallback<BatchResponse> callback) = 0;
};

// Abstract client interface
class IBenchmarkClient {
public:
  virtual ~IBenchmarkClient() = default;

  // Get the service stub for making RPC calls
  virtual IBenchmarkService* GetService() = 0;

  // Connect to server
  virtual bool Connect(const std::string& address) = 0;

  // Disconnect from server
  virtual void Disconnect() = 0;

  // Check if connected
  virtual bool IsConnected() const = 0;
};

// Abstract server interface
class IBenchmarkServer {
public:
  virtual ~IBenchmarkServer() = default;

  // Start the server on the specified address
  virtual bool Start(const std::string& address) = 0;

  // Stop the server
  virtual void Stop() = 0;

  // Check if server is running
  virtual bool IsRunning() const = 0;

  // Wait for server to shut down
  virtual void Wait() = 0;
};

// Factory interface for creating clients and servers
class IFrameworkFactory {
public:
  virtual ~IFrameworkFactory() = default;

  // Get framework name
  virtual std::string GetName() const = 0;

  // Create a client instance
  virtual std::unique_ptr<IBenchmarkClient> CreateClient() = 0;

  // Create a server instance with the provided service implementation
  virtual std::unique_ptr<IBenchmarkServer> CreateServer(
      std::shared_ptr<IBenchmarkService> service) = 0;
};

} // namespace common
} // namespace benchmark
