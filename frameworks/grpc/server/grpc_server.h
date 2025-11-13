#pragma once

#include "benchmark_service.h"
#include "benchmark_types.h"
#include "benchmark.grpc.pb.h"
#include <grpcpp/grpcpp.h>
#include <memory>

namespace benchmark {
namespace grpc_impl {

// Forward declaration
grpc::StatusCode ToGrpcStatus(common::ErrorCode code);

// gRPC service implementation that bridges to common service
class BenchmarkServiceImpl final : public ::benchmark::BenchmarkService::Service {
public:
  explicit BenchmarkServiceImpl(std::shared_ptr<common::IBenchmarkService> service);

  grpc::Status Echo(
      grpc::ServerContext* context,
      const ::benchmark::EchoRequest* request,
      ::benchmark::EchoResponse* response) override;

  grpc::Status StreamData(
      grpc::ServerContext* context,
      const ::benchmark::StreamRequest* request,
      grpc::ServerWriter<::benchmark::DataChunk>* writer) override;

  grpc::Status UploadData(
      grpc::ServerContext* context,
      grpc::ServerReader<::benchmark::DataChunk>* reader,
      ::benchmark::UploadResponse* response) override;

  grpc::Status BidirectionalStream(
      grpc::ServerContext* context,
      grpc::ServerReaderWriter<::benchmark::DataChunk, ::benchmark::DataChunk>* stream) override;

  grpc::Status BatchProcess(
      grpc::ServerContext* context,
      const ::benchmark::BatchRequest* request,
      ::benchmark::BatchResponse* response) override;

private:
  std::shared_ptr<common::IBenchmarkService> service_;
};

// gRPC server wrapper
class GrpcBenchmarkServer : public common::IBenchmarkServer {
public:
  explicit GrpcBenchmarkServer(std::shared_ptr<common::IBenchmarkService> service);
  ~GrpcBenchmarkServer() override = default;

  bool Start(const std::string& address) override;
  void Stop() override;
  bool IsRunning() const override;
  void Wait() override;

private:
  std::shared_ptr<common::IBenchmarkService> service_;
  std::unique_ptr<BenchmarkServiceImpl> service_impl_;
  std::unique_ptr<grpc::Server> server_;
  bool running_;
};

// Factory for creating gRPC clients and servers
class GrpcFrameworkFactory : public common::IFrameworkFactory {
public:
  std::string GetName() const override;
  std::unique_ptr<common::IBenchmarkClient> CreateClient() override;
  std::unique_ptr<common::IBenchmarkServer> CreateServer(
      std::shared_ptr<common::IBenchmarkService> service) override;
};

// Factory function
std::unique_ptr<common::IFrameworkFactory> CreateGrpcFactory();

} // namespace grpc_impl
} // namespace benchmark
