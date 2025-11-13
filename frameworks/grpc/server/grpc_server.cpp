#include "grpc_server.h"
#include "grpc_client.h"
#include "reference_service.h"
#include <iostream>

namespace benchmark {
namespace grpc_impl {

grpc::StatusCode ToGrpcStatus(common::ErrorCode code) {
  switch (code) {
    case common::ErrorCode::OK:
      return grpc::StatusCode::OK;
    case common::ErrorCode::INVALID_ARGUMENT:
      return grpc::StatusCode::INVALID_ARGUMENT;
    case common::ErrorCode::DEADLINE_EXCEEDED:
      return grpc::StatusCode::DEADLINE_EXCEEDED;
    case common::ErrorCode::NOT_FOUND:
      return grpc::StatusCode::NOT_FOUND;
    case common::ErrorCode::UNAVAILABLE:
      return grpc::StatusCode::UNAVAILABLE;
    default:
      return grpc::StatusCode::INTERNAL;
  }
}

// BenchmarkServiceImpl implementation
BenchmarkServiceImpl::BenchmarkServiceImpl(
    std::shared_ptr<common::IBenchmarkService> service)
  : service_(service) {}

grpc::Status BenchmarkServiceImpl::Echo(
    grpc::ServerContext* context,
    const ::benchmark::EchoRequest* request,
    ::benchmark::EchoResponse* response) {

  auto common_req = FromProto(*request);
  auto result = service_->Echo(common_req);

  if (result.ok()) {
    *response = ToProto(result.value);
    return grpc::Status::OK;
  } else {
    return grpc::Status(
        ToGrpcStatus(result.error_code),
        result.error_message
    );
  }
}

grpc::Status BenchmarkServiceImpl::StreamData(
    grpc::ServerContext* context,
    const ::benchmark::StreamRequest* request,
    grpc::ServerWriter<::benchmark::DataChunk>* writer) {

  auto common_req = FromProto(*request);

  std::string error_msg;
  common::ErrorCode error_code = common::ErrorCode::OK;

  service_->StreamData(
      common_req,
      // on_chunk callback
      [writer](const common::DataChunk& chunk) {
        writer->Write(ToProto(chunk));
      },
      // on_complete callback
      [&error_code, &error_msg](common::ErrorCode code, const std::string& msg) {
        error_code = code;
        error_msg = msg;
      }
  );

  if (error_code == common::ErrorCode::OK) {
    return grpc::Status::OK;
  } else {
    return grpc::Status(ToGrpcStatus(error_code), error_msg);
  }
}

grpc::Status BenchmarkServiceImpl::UploadData(
    grpc::ServerContext* context,
    grpc::ServerReader<::benchmark::DataChunk>* reader,
    ::benchmark::UploadResponse* response) {

  // Simplified implementation
  common::UploadResponse resp;
  *response = ToProto(resp);
  return grpc::Status::OK;
}

grpc::Status BenchmarkServiceImpl::BidirectionalStream(
    grpc::ServerContext* context,
    grpc::ServerReaderWriter<::benchmark::DataChunk, ::benchmark::DataChunk>* stream) {

  // Simplified implementation
  return grpc::Status::OK;
}

grpc::Status BenchmarkServiceImpl::BatchProcess(
    grpc::ServerContext* context,
    const ::benchmark::BatchRequest* request,
    ::benchmark::BatchResponse* response) {

  auto common_req = FromProto(*request);
  auto result = service_->BatchProcess(common_req);

  if (result.ok()) {
    *response = ToProto(result.value);
    return grpc::Status::OK;
  } else {
    return grpc::Status(
        ToGrpcStatus(result.error_code),
        result.error_message
    );
  }
}

// GrpcBenchmarkServer implementation
GrpcBenchmarkServer::GrpcBenchmarkServer(
    std::shared_ptr<common::IBenchmarkService> service)
  : service_(service), running_(false) {}

bool GrpcBenchmarkServer::Start(const std::string& address) {
  grpc::ServerBuilder builder;
  builder.AddListeningPort(address, grpc::InsecureServerCredentials());

  service_impl_ = std::make_unique<BenchmarkServiceImpl>(service_);
  builder.RegisterService(service_impl_.get());

  server_ = builder.BuildAndStart();
  if (server_) {
    running_ = true;
    std::cout << "gRPC server listening on " << address << std::endl;
    return true;
  }

  return false;
}

void GrpcBenchmarkServer::Stop() {
  if (server_) {
    server_->Shutdown();
    running_ = false;
  }
}

bool GrpcBenchmarkServer::IsRunning() const {
  return running_;
}

void GrpcBenchmarkServer::Wait() {
  if (server_) {
    server_->Wait();
  }
}

// GrpcFrameworkFactory implementation
std::string GrpcFrameworkFactory::GetName() const {
  return "gRPC";
}

std::unique_ptr<common::IBenchmarkClient> GrpcFrameworkFactory::CreateClient() {
  return std::make_unique<GrpcBenchmarkClient>();
}

std::unique_ptr<common::IBenchmarkServer> GrpcFrameworkFactory::CreateServer(
    std::shared_ptr<common::IBenchmarkService> service) {
  return std::make_unique<GrpcBenchmarkServer>(service);
}

// Factory function
std::unique_ptr<common::IFrameworkFactory> CreateGrpcFactory() {
  return std::make_unique<GrpcFrameworkFactory>();
}

} // namespace grpc_impl
} // namespace benchmark
