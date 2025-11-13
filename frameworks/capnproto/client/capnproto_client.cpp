// Cap'n Proto client implementation
#include "capnproto_client.h"
#include "capnproto_server.h"
#include "reference_service.h"
#include <cstring>

namespace benchmark {
namespace capnp_impl {

// Conversion functions implementation
void ToCapnp(::EchoRequest::Builder builder, const common::EchoRequest& req) {
  builder.setMessage(req.message);
  builder.setTimestamp(req.timestamp);
  builder.setSequenceNumber(req.sequence_number);
}

common::EchoRequest FromCapnp(::EchoRequest::Reader reader) {
  common::EchoRequest req;
  req.message = reader.getMessage();
  req.timestamp = reader.getTimestamp();
  req.sequence_number = reader.getSequenceNumber();
  return req;
}

common::EchoResponse FromCapnp(::EchoResponse::Reader reader) {
  common::EchoResponse resp;
  resp.message = reader.getMessage();
  resp.client_timestamp = reader.getClientTimestamp();
  resp.server_timestamp = reader.getServerTimestamp();
  resp.sequence_number = reader.getSequenceNumber();
  return resp;
}

void ToCapnp(::EchoResponse::Builder builder, const common::EchoResponse& resp) {
  builder.setMessage(resp.message);
  builder.setClientTimestamp(resp.client_timestamp);
  builder.setServerTimestamp(resp.server_timestamp);
  builder.setSequenceNumber(resp.sequence_number);
}

void ToCapnp(::StreamRequest::Builder builder, const common::StreamRequest& req) {
  builder.setChunkSize(req.chunk_size);
  builder.setChunkCount(req.chunk_count);
  builder.setDelayMs(req.delay_ms);
}

common::StreamRequest FromCapnp(::StreamRequest::Reader reader) {
  common::StreamRequest req;
  req.chunk_size = reader.getChunkSize();
  req.chunk_count = reader.getChunkCount();
  req.delay_ms = reader.getDelayMs();
  return req;
}

common::DataChunk FromCapnp(::DataChunk::Reader reader) {
  common::DataChunk chunk;
  chunk.sequence_number = reader.getSequenceNumber();
  auto data = reader.getData();
  chunk.data.assign(data.begin(), data.end());
  chunk.checksum = reader.getChecksum();
  chunk.timestamp = reader.getTimestamp();
  return chunk;
}

void ToCapnp(::DataChunk::Builder builder, const common::DataChunk& chunk) {
  builder.setSequenceNumber(chunk.sequence_number);
  builder.setData(kj::ArrayPtr<const capnp::byte>(
      reinterpret_cast<const capnp::byte*>(chunk.data.data()),
      chunk.data.size()));
  builder.setChecksum(chunk.checksum);
  builder.setTimestamp(chunk.timestamp);
}

common::UploadResponse FromCapnp(::UploadResponse::Reader reader) {
  common::UploadResponse resp;
  resp.total_bytes = reader.getTotalBytes();
  resp.chunk_count = reader.getChunkCount();
  resp.duration_ns = reader.getDurationNs();
  resp.checksum_valid = reader.getChecksumValid();
  return resp;
}

void ToCapnp(::UploadResponse::Builder builder, const common::UploadResponse& resp) {
  builder.setTotalBytes(resp.total_bytes);
  builder.setChunkCount(resp.chunk_count);
  builder.setDurationNs(resp.duration_ns);
  builder.setChecksumValid(resp.checksum_valid);
}

void ToCapnp(::BatchRequest::Builder builder, const common::BatchRequest& req) {
  builder.setFailOnError(req.fail_on_error);
  auto items = builder.initItems(req.items.size());
  for (size_t i = 0; i < req.items.size(); ++i) {
    auto item = items[i];
    item.setId(req.items[i].id);
    item.setOperation(req.items[i].operation);
    item.setData(kj::ArrayPtr<const capnp::byte>(
        reinterpret_cast<const capnp::byte*>(req.items[i].data.data()),
        req.items[i].data.size()));
  }
}

common::BatchRequest FromCapnp(::BatchRequest::Reader reader) {
  common::BatchRequest req;
  req.fail_on_error = reader.getFailOnError();
  auto items = reader.getItems();
  for (auto item : items) {
    common::BatchItem batch_item;
    batch_item.id = item.getId();
    batch_item.operation = item.getOperation();
    auto data = item.getData();
    batch_item.data.assign(data.begin(), data.end());
    req.items.push_back(batch_item);
  }
  return req;
}

common::BatchResponse FromCapnp(::BatchResponse::Reader reader) {
  common::BatchResponse resp;
  resp.total_processed = reader.getTotalProcessed();
  resp.total_failed = reader.getTotalFailed();
  auto results = reader.getResults();
  for (auto result : results) {
    common::BatchResult batch_result;
    batch_result.id = result.getId();
    batch_result.success = result.getSuccess();
    batch_result.error_message = result.getErrorMessage();
    auto data = result.getResultData();
    batch_result.result_data.assign(data.begin(), data.end());
    resp.results.push_back(batch_result);
  }
  return resp;
}

void ToCapnp(::BatchResponse::Builder builder, const common::BatchResponse& resp) {
  builder.setTotalProcessed(resp.total_processed);
  builder.setTotalFailed(resp.total_failed);
  auto results = builder.initResults(resp.results.size());
  for (size_t i = 0; i < resp.results.size(); ++i) {
    auto result = results[i];
    result.setId(resp.results[i].id);
    result.setSuccess(resp.results[i].success);
    result.setErrorMessage(resp.results[i].error_message);
    result.setResultData(kj::ArrayPtr<const capnp::byte>(
        reinterpret_cast<const capnp::byte*>(resp.results[i].result_data.data()),
        resp.results[i].result_data.size()));
  }
}

// CapnProtoServiceAdapter implementation
CapnProtoServiceAdapter::CapnProtoServiceAdapter(BenchmarkService::Client client)
    : client_(kj::mv(client)), io_context_(nullptr) {
}

common::Result<common::EchoResponse> CapnProtoServiceAdapter::Echo(
    const common::EchoRequest& request) {
  // Cap'n Proto RPC is inherently async, so we need to wait for the promise
  auto req_builder = client_.echoRequest();
  ToCapnp(req_builder.initRequest(), request);

  try {
    auto promise = req_builder.send();
    auto response = promise.wait(client_.getContext().waitScope);
    return common::Result<common::EchoResponse>(FromCapnp(response.getResponse()));
  } catch (const kj::Exception& e) {
    return common::Result<common::EchoResponse>(
        common::ErrorCode::UNKNOWN,
        std::string("Cap'n Proto error: ") + e.getDescription().cStr());
  }
}

void CapnProtoServiceAdapter::EchoAsync(
    const common::EchoRequest& request,
    common::ResponseCallback<common::EchoResponse> callback) {
  auto req_builder = client_.echoRequest();
  ToCapnp(req_builder.initRequest(), request);

  auto promise = req_builder.send().then(
      [callback](capnp::Response<BenchmarkService::EchoResults>&& response) {
        callback(common::Result<common::EchoResponse>(
            FromCapnp(response.getResponse())));
      },
      [callback](kj::Exception&& e) {
        callback(common::Result<common::EchoResponse>(
            common::ErrorCode::UNKNOWN,
            std::string("Cap'n Proto error: ") + e.getDescription().cStr()));
      });

  // Note: In a real implementation, we'd need to properly manage the async context
  promise.detach([](kj::Exception&&) {});
}

void CapnProtoServiceAdapter::StreamData(
    const common::StreamRequest& request,
    common::StreamCallback<common::DataChunk> on_chunk,
    common::CompletionCallback on_complete) {
  // Cap'n Proto streaming requires special handling
  // For now, return not implemented
  on_complete(false, "Streaming not yet implemented for Cap'n Proto");
}

void CapnProtoServiceAdapter::UploadData(
    common::StreamCallback<common::DataChunk>& chunk_provider,
    common::ResponseCallback<common::UploadResponse> on_complete) {
  on_complete(common::Result<common::UploadResponse>(
      common::ErrorCode::UNIMPLEMENTED,
      "Client streaming not yet implemented for Cap'n Proto"));
}

void CapnProtoServiceAdapter::BidirectionalStream(
    common::StreamCallback<common::DataChunk>& chunk_provider,
    common::StreamCallback<common::DataChunk> on_chunk,
    common::CompletionCallback on_complete) {
  on_complete(false, "Bidirectional streaming not yet implemented for Cap'n Proto");
}

common::Result<common::BatchResponse> CapnProtoServiceAdapter::BatchProcess(
    const common::BatchRequest& request) {
  auto req_builder = client_.batchProcessRequest();
  ToCapnp(req_builder.initRequest(), request);

  try {
    auto promise = req_builder.send();
    auto response = promise.wait(client_.getContext().waitScope);
    return common::Result<common::BatchResponse>(FromCapnp(response.getResponse()));
  } catch (const kj::Exception& e) {
    return common::Result<common::BatchResponse>(
        common::ErrorCode::UNKNOWN,
        std::string("Cap'n Proto error: ") + e.getDescription().cStr());
  }
}

void CapnProtoServiceAdapter::BatchProcessAsync(
    const common::BatchRequest& request,
    common::ResponseCallback<common::BatchResponse> callback) {
  auto req_builder = client_.batchProcessRequest();
  ToCapnp(req_builder.initRequest(), request);

  auto promise = req_builder.send().then(
      [callback](capnp::Response<BenchmarkService::BatchProcessResults>&& response) {
        callback(common::Result<common::BatchResponse>(
            FromCapnp(response.getResponse())));
      },
      [callback](kj::Exception&& e) {
        callback(common::Result<common::BatchResponse>(
            common::ErrorCode::UNKNOWN,
            std::string("Cap'n Proto error: ") + e.getDescription().cStr()));
      });

  promise.detach([](kj::Exception&&) {});
}

// CapnProtoBenchmarkClient implementation
CapnProtoBenchmarkClient::CapnProtoBenchmarkClient()
    : connected_(false) {
}

CapnProtoBenchmarkClient::~CapnProtoBenchmarkClient() {
  Disconnect();
}

common::IBenchmarkService* CapnProtoBenchmarkClient::GetService() {
  return service_.get();
}

bool CapnProtoBenchmarkClient::Connect(const std::string& address) {
  try {
    // Parse address (format: "host:port")
    size_t colon_pos = address.find(':');
    if (colon_pos == std::string::npos) {
      return false;
    }

    std::string host = address.substr(0, colon_pos);
    uint port = std::stoul(address.substr(colon_pos + 1));

    // Create EzRpcClient and connect
    rpc_client_ = std::make_unique<capnp::EzRpcClient>(host, port);
    auto& wait_scope = rpc_client_->getWaitScope();

    // Get the bootstrap capability
    auto capability = rpc_client_->getMain<BenchmarkService>();

    // Create service adapter
    service_ = std::make_unique<CapnProtoServiceAdapter>(kj::mv(capability));
    connected_ = true;

    return true;
  } catch (...) {
    return false;
  }
}

void CapnProtoBenchmarkClient::Disconnect() {
  if (connected_) {
    service_.reset();
    rpc_client_.reset();
    connected_ = false;
  }
}

bool CapnProtoBenchmarkClient::IsConnected() const {
  return connected_;
}

// Factory implementation
class CapnProtoFactory : public common::IFrameworkFactory {
public:
  std::string GetName() const override {
    return "Cap'n Proto";
  }

  std::unique_ptr<common::IBenchmarkClient> CreateClient() override {
    return std::make_unique<CapnProtoBenchmarkClient>();
  }

  std::unique_ptr<common::IBenchmarkServer> CreateServer(
      std::shared_ptr<common::IBenchmarkService> service) override {
    return std::make_unique<CapnProtoBenchmarkServer>(service);
  }
};

std::unique_ptr<common::IFrameworkFactory> CreateCapnProtoFactory() {
  return std::make_unique<CapnProtoFactory>();
}

} // namespace capnp_impl
} // namespace benchmark
