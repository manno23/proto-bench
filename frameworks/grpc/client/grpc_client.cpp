#include "grpc_client.h"
#include "benchmark_utils.h"
#include <grpcpp/grpcpp.h>

namespace benchmark {
namespace grpc_impl {

// Conversion functions - Request types
::benchmark::EchoRequest ToProto(const common::EchoRequest& req) {
  ::benchmark::EchoRequest proto;
  proto.set_message(req.message);
  proto.set_timestamp(req.timestamp);
  proto.set_sequence_number(req.sequence_number);
  return proto;
}

common::EchoRequest FromProto(const ::benchmark::EchoRequest& proto) {
  common::EchoRequest req;
  req.message = proto.message();
  req.timestamp = proto.timestamp();
  req.sequence_number = proto.sequence_number();
  return req;
}

// Conversion functions - Response types
common::EchoResponse FromProto(const ::benchmark::EchoResponse& proto) {
  common::EchoResponse resp;
  resp.message = proto.message();
  resp.client_timestamp = proto.client_timestamp();
  resp.server_timestamp = proto.server_timestamp();
  resp.sequence_number = proto.sequence_number();
  return resp;
}

::benchmark::EchoResponse ToProto(const common::EchoResponse& resp) {
  ::benchmark::EchoResponse proto;
  proto.set_message(resp.message);
  proto.set_client_timestamp(resp.client_timestamp);
  proto.set_server_timestamp(resp.server_timestamp);
  proto.set_sequence_number(resp.sequence_number);
  return proto;
}

// Conversion functions - Stream types
::benchmark::StreamRequest ToProto(const common::StreamRequest& req) {
  ::benchmark::StreamRequest proto;
  proto.set_chunk_size(req.chunk_size);
  proto.set_chunk_count(req.chunk_count);
  proto.set_delay_ms(req.delay_ms);
  return proto;
}

common::StreamRequest FromProto(const ::benchmark::StreamRequest& proto) {
  common::StreamRequest req;
  req.chunk_size = proto.chunk_size();
  req.chunk_count = proto.chunk_count();
  req.delay_ms = proto.delay_ms();
  return req;
}

common::DataChunk FromProto(const ::benchmark::DataChunk& proto) {
  common::DataChunk chunk;
  chunk.sequence_number = proto.sequence_number();
  chunk.data.assign(proto.data().begin(), proto.data().end());
  chunk.checksum = proto.checksum();
  chunk.timestamp = proto.timestamp();
  return chunk;
}

::benchmark::DataChunk ToProto(const common::DataChunk& chunk) {
  ::benchmark::DataChunk proto;
  proto.set_sequence_number(chunk.sequence_number);
  proto.set_data(chunk.data.data(), chunk.data.size());
  proto.set_checksum(chunk.checksum);
  proto.set_timestamp(chunk.timestamp);
  return proto;
}

common::UploadResponse FromProto(const ::benchmark::UploadResponse& proto) {
  common::UploadResponse resp;
  resp.total_bytes = proto.total_bytes();
  resp.chunk_count = proto.chunk_count();
  resp.duration_ns = proto.duration_ns();
  resp.checksum_valid = proto.checksum_valid();
  return resp;
}

::benchmark::UploadResponse ToProto(const common::UploadResponse& resp) {
  ::benchmark::UploadResponse proto;
  proto.set_total_bytes(resp.total_bytes);
  proto.set_chunk_count(resp.chunk_count);
  proto.set_duration_ns(resp.duration_ns);
  proto.set_checksum_valid(resp.checksum_valid);
  return proto;
}

// Conversion functions - Batch types
::benchmark::BatchRequest ToProto(const common::BatchRequest& req) {
  ::benchmark::BatchRequest proto;
  proto.set_fail_on_error(req.fail_on_error);
  for (const auto& item : req.items) {
    auto* proto_item = proto.add_items();
    proto_item->set_id(item.id);
    proto_item->set_operation(item.operation);
    proto_item->set_data(item.data.data(), item.data.size());
  }
  return proto;
}

common::BatchRequest FromProto(const ::benchmark::BatchRequest& proto) {
  common::BatchRequest req;
  req.fail_on_error = proto.fail_on_error();
  for (const auto& item : proto.items()) {
    common::BatchItem batch_item;
    batch_item.id = item.id();
    batch_item.operation = item.operation();
    batch_item.data.assign(item.data().begin(), item.data().end());
    req.items.push_back(batch_item);
  }
  return req;
}

common::BatchResponse FromProto(const ::benchmark::BatchResponse& proto) {
  common::BatchResponse resp;
  resp.total_processed = proto.total_processed();
  resp.total_failed = proto.total_failed();
  for (const auto& result : proto.results()) {
    common::BatchResult r;
    r.id = result.id();
    r.success = result.success();
    r.error_message = result.error_message();
    r.result_data.assign(result.result_data().begin(), result.result_data().end());
    resp.results.push_back(r);
  }
  return resp;
}

::benchmark::BatchResponse ToProto(const common::BatchResponse& resp) {
  ::benchmark::BatchResponse proto;
  proto.set_total_processed(resp.total_processed);
  proto.set_total_failed(resp.total_failed);
  for (const auto& result : resp.results) {
    auto* proto_result = proto.add_results();
    proto_result->set_id(result.id);
    proto_result->set_success(result.success);
    proto_result->set_error_message(result.error_message);
    proto_result->set_result_data(result.result_data.data(), result.result_data.size());
  }
  return proto;
}

common::ErrorCode FromGrpcStatus(grpc::StatusCode code) {
  switch (code) {
    case grpc::StatusCode::OK:
      return common::ErrorCode::OK;
    case grpc::StatusCode::INVALID_ARGUMENT:
      return common::ErrorCode::INVALID_ARGUMENT;
    case grpc::StatusCode::DEADLINE_EXCEEDED:
      return common::ErrorCode::DEADLINE_EXCEEDED;
    case grpc::StatusCode::NOT_FOUND:
      return common::ErrorCode::NOT_FOUND;
    case grpc::StatusCode::UNAVAILABLE:
      return common::ErrorCode::UNAVAILABLE;
    default:
      return common::ErrorCode::INTERNAL;
  }
}

// GrpcServiceAdapter implementation
GrpcServiceAdapter::GrpcServiceAdapter(
    std::unique_ptr<::benchmark::BenchmarkService::Stub> stub)
  : stub_(std::move(stub)) {}

common::Result<common::EchoResponse> GrpcServiceAdapter::Echo(
    const common::EchoRequest& request) {

  auto proto_req = ToProto(request);
  ::benchmark::EchoResponse proto_resp;

  grpc::ClientContext context;
  auto status = stub_->Echo(&context, proto_req, &proto_resp);

  if (status.ok()) {
    return common::Result<common::EchoResponse>(FromProto(proto_resp));
  } else {
    return common::Result<common::EchoResponse>(
        FromGrpcStatus(status.error_code()),
        status.error_message()
    );
  }
}

void GrpcServiceAdapter::EchoAsync(
    const common::EchoRequest& request,
    common::ResponseCallback<common::EchoResponse> callback) {
  auto result = Echo(request);
  callback(result);
}

void GrpcServiceAdapter::StreamData(
    const common::StreamRequest& request,
    common::StreamCallback<common::DataChunk> on_chunk,
    common::CompletionCallback on_complete) {

  auto proto_req = ToProto(request);

  grpc::ClientContext context;
  auto reader = stub_->StreamData(&context, proto_req);

  ::benchmark::DataChunk proto_chunk;
  while (reader->Read(&proto_chunk)) {
    on_chunk(FromProto(proto_chunk));
  }

  auto status = reader->Finish();
  on_complete(FromGrpcStatus(status.error_code()), status.error_message());
}

void GrpcServiceAdapter::UploadData(
    common::StreamCallback<common::DataChunk>& chunk_provider,
    common::ResponseCallback<common::UploadResponse> on_complete) {
  common::UploadResponse resp;
  on_complete(common::Result<common::UploadResponse>(resp));
}

void GrpcServiceAdapter::BidirectionalStream(
    common::StreamCallback<common::DataChunk>& chunk_provider,
    common::StreamCallback<common::DataChunk> on_chunk,
    common::CompletionCallback on_complete) {
  on_complete(common::ErrorCode::OK, "");
}

common::Result<common::BatchResponse> GrpcServiceAdapter::BatchProcess(
    const common::BatchRequest& request) {

  auto proto_req = ToProto(request);
  ::benchmark::BatchResponse proto_resp;

  grpc::ClientContext context;
  auto status = stub_->BatchProcess(&context, proto_req, &proto_resp);

  if (status.ok()) {
    return common::Result<common::BatchResponse>(FromProto(proto_resp));
  } else {
    return common::Result<common::BatchResponse>(
        FromGrpcStatus(status.error_code()),
        status.error_message()
    );
  }
}

void GrpcServiceAdapter::BatchProcessAsync(
    const common::BatchRequest& request,
    common::ResponseCallback<common::BatchResponse> callback) {
  auto result = BatchProcess(request);
  callback(result);
}

// GrpcBenchmarkClient implementation
GrpcBenchmarkClient::GrpcBenchmarkClient() = default;

common::IBenchmarkService* GrpcBenchmarkClient::GetService() {
  return service_.get();
}

bool GrpcBenchmarkClient::Connect(const std::string& address) {
  channel_ = grpc::CreateChannel(address, grpc::InsecureChannelCredentials());

  auto deadline = std::chrono::system_clock::now() + std::chrono::seconds(5);
  if (!channel_->WaitForConnected(deadline)) {
    return false;
  }

  auto stub = ::benchmark::BenchmarkService::NewStub(channel_);
  service_ = std::make_unique<GrpcServiceAdapter>(std::move(stub));

  return true;
}

void GrpcBenchmarkClient::Disconnect() {
  service_.reset();
  channel_.reset();
}

bool GrpcBenchmarkClient::IsConnected() const {
  if (!channel_) return false;
  auto state = channel_->GetState(false);
  return state == GRPC_CHANNEL_READY || state == GRPC_CHANNEL_IDLE;
}

} // namespace grpc_impl
} // namespace benchmark
