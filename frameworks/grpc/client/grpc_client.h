#pragma once

#include "benchmark_service.h"
#include "benchmark_types.h"
#include "benchmark.grpc.pb.h"
#include <grpcpp/grpcpp.h>
#include <memory>

namespace benchmark {
namespace grpc_impl {

// Adapter that wraps gRPC stub and implements common service interface
class GrpcServiceAdapter : public common::IBenchmarkService {
public:
  explicit GrpcServiceAdapter(std::unique_ptr<::benchmark::BenchmarkService::Stub> stub);
  ~GrpcServiceAdapter() override = default;

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
  std::unique_ptr<::benchmark::BenchmarkService::Stub> stub_;
};

// gRPC client implementation
class GrpcBenchmarkClient : public common::IBenchmarkClient {
public:
  GrpcBenchmarkClient();
  ~GrpcBenchmarkClient() override = default;

  common::IBenchmarkService* GetService() override;
  bool Connect(const std::string& address) override;
  void Disconnect() override;
  bool IsConnected() const override;

private:
  std::shared_ptr<grpc::Channel> channel_;
  std::unique_ptr<GrpcServiceAdapter> service_;
};

// Conversion functions
::benchmark::EchoRequest ToProto(const common::EchoRequest& req);
common::EchoRequest FromProto(const ::benchmark::EchoRequest& proto);
common::EchoResponse FromProto(const ::benchmark::EchoResponse& proto);
::benchmark::EchoResponse ToProto(const common::EchoResponse& resp);
::benchmark::StreamRequest ToProto(const common::StreamRequest& req);
common::StreamRequest FromProto(const ::benchmark::StreamRequest& proto);
common::DataChunk FromProto(const ::benchmark::DataChunk& proto);
::benchmark::DataChunk ToProto(const common::DataChunk& chunk);
common::UploadResponse FromProto(const ::benchmark::UploadResponse& proto);
::benchmark::UploadResponse ToProto(const common::UploadResponse& resp);
::benchmark::BatchRequest ToProto(const common::BatchRequest& req);
common::BatchRequest FromProto(const ::benchmark::BatchRequest& proto);
common::BatchResponse FromProto(const ::benchmark::BatchResponse& proto);
::benchmark::BatchResponse ToProto(const common::BatchResponse& resp);
common::ErrorCode FromGrpcStatus(grpc::StatusCode code);

} // namespace grpc_impl
} // namespace benchmark
