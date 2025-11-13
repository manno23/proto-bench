#pragma once

#include "benchmark_service.h"
#include "benchmark_types.h"
#include "benchmark.capnp.h"
#include <capnp/ez-rpc.h>
#include <kj/async.h>
#include <memory>

namespace benchmark {
namespace capnp_impl {

// Adapter that wraps Cap'n Proto capability and implements common service interface
class CapnProtoServiceAdapter : public common::IBenchmarkService {
public:
  explicit CapnProtoServiceAdapter(BenchmarkService::Client client);
  ~CapnProtoServiceAdapter() override = default;

  // Synchronous Echo
  common::Result<common::EchoResponse> Echo(
      const common::EchoRequest& request) override;

  // Async Echo
  void EchoAsync(
      const common::EchoRequest& request,
      common::ResponseCallback<common::EchoResponse> callback) override;

  // Server streaming
  void StreamData(
      const common::StreamRequest& request,
      common::StreamCallback<common::DataChunk> on_chunk,
      common::CompletionCallback on_complete) override;

  // Client streaming
  void UploadData(
      common::StreamCallback<common::DataChunk>& chunk_provider,
      common::ResponseCallback<common::UploadResponse> on_complete) override;

  // Bidirectional streaming
  void BidirectionalStream(
      common::StreamCallback<common::DataChunk>& chunk_provider,
      common::StreamCallback<common::DataChunk> on_chunk,
      common::CompletionCallback on_complete) override;

  // Batch processing
  common::Result<common::BatchResponse> BatchProcess(
      const common::BatchRequest& request) override;

  // Async batch processing
  void BatchProcessAsync(
      const common::BatchRequest& request,
      common::ResponseCallback<common::BatchResponse> callback) override;

private:
  BenchmarkService::Client client_;
  kj::AsyncIoContext* io_context_;
};

// Cap'n Proto client implementation
class CapnProtoBenchmarkClient : public common::IBenchmarkClient {
public:
  CapnProtoBenchmarkClient();
  ~CapnProtoBenchmarkClient() override;

  common::IBenchmarkService* GetService() override;
  bool Connect(const std::string& address) override;
  void Disconnect() override;
  bool IsConnected() const override;

private:
  std::unique_ptr<capnp::EzRpcClient> rpc_client_;
  std::unique_ptr<CapnProtoServiceAdapter> service_;
  bool connected_;
};

// Factory function
std::unique_ptr<common::IFrameworkFactory> CreateCapnProtoFactory();

// Conversion functions
void ToCapnp(::EchoRequest::Builder builder, const common::EchoRequest& req);
common::EchoRequest FromCapnp(::EchoRequest::Reader reader);
common::EchoResponse FromCapnp(::EchoResponse::Reader reader);
void ToCapnp(::EchoResponse::Builder builder, const common::EchoResponse& resp);
void ToCapnp(::StreamRequest::Builder builder, const common::StreamRequest& req);
common::StreamRequest FromCapnp(::StreamRequest::Reader reader);
common::DataChunk FromCapnp(::DataChunk::Reader reader);
void ToCapnp(::DataChunk::Builder builder, const common::DataChunk& chunk);
common::UploadResponse FromCapnp(::UploadResponse::Reader reader);
void ToCapnp(::UploadResponse::Builder builder, const common::UploadResponse& resp);
void ToCapnp(::BatchRequest::Builder builder, const common::BatchRequest& req);
common::BatchRequest FromCapnp(::BatchRequest::Reader reader);
common::BatchResponse FromCapnp(::BatchResponse::Reader reader);
void ToCapnp(::BatchResponse::Builder builder, const common::BatchResponse& resp);

} // namespace capnp_impl
} // namespace benchmark
