#include "reference_service.h"
#include <thread>
#include <chrono>

namespace benchmark {
namespace reference {

common::Result<common::EchoResponse> ReferenceServiceImpl::Echo(
    const common::EchoRequest& request) {

  common::EchoResponse response;
  response.message = request.message;
  response.client_timestamp = request.timestamp;
  response.server_timestamp = common::utils::GetTimestampNanos();
  response.sequence_number = request.sequence_number;

  return common::Result<common::EchoResponse>(response);
}

void ReferenceServiceImpl::EchoAsync(
    const common::EchoRequest& request,
    common::ResponseCallback<common::EchoResponse> callback) {

  auto result = Echo(request);
  callback(result);
}

void ReferenceServiceImpl::StreamData(
    const common::StreamRequest& request,
    common::StreamCallback<common::DataChunk> on_chunk,
    common::CompletionCallback on_complete) {

  // Generate and stream chunks
  for (uint32_t i = 0; i < request.chunk_count; i++) {
    common::DataChunk chunk;
    chunk.sequence_number = i;
    chunk.data = common::utils::GenerateRandomData(request.chunk_size, i);
    chunk.checksum = crc32_.Calculate(chunk.data);
    chunk.timestamp = common::utils::GetTimestampNanos();

    on_chunk(chunk);

    // Apply delay if requested
    if (request.delay_ms > 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(request.delay_ms));
    }
  }

  on_complete(common::ErrorCode::OK, "");
}

void ReferenceServiceImpl::UploadData(
    common::StreamCallback<common::DataChunk>& chunk_provider,
    common::ResponseCallback<common::UploadResponse> on_complete) {

  common::UploadResponse response;
  response.total_bytes = 0;
  response.chunk_count = 0;
  response.checksum_valid = true;

  auto start_time = common::utils::GetTimestampNanos();

  // This is a simplified version - in reality, chunk_provider would be called
  // repeatedly until the stream ends. For now, we'll indicate this is not
  // fully implemented in this reference version.

  auto end_time = common::utils::GetTimestampNanos();
  response.duration_ns = end_time - start_time;

  on_complete(common::Result<common::UploadResponse>(response));
}

void ReferenceServiceImpl::BidirectionalStream(
    common::StreamCallback<common::DataChunk>& chunk_provider,
    common::StreamCallback<common::DataChunk> on_chunk,
    common::CompletionCallback on_complete) {

  // Simplified bidirectional streaming
  // In a real implementation, this would process incoming chunks
  // and send responses simultaneously

  on_complete(common::ErrorCode::OK, "");
}

common::Result<common::BatchResponse> ReferenceServiceImpl::BatchProcess(
    const common::BatchRequest& request) {

  common::BatchResponse response;
  response.total_processed = 0;
  response.total_failed = 0;

  for (const auto& item : request.items) {
    common::BatchResult result;
    result.id = item.id;

    // Simple operation processing
    if (item.operation == "echo") {
      result.success = true;
      result.result_data = item.data;
    } else if (item.operation == "reverse") {
      result.success = true;
      result.result_data = item.data;
      std::reverse(result.result_data.begin(), result.result_data.end());
    } else if (item.operation == "fail") {
      result.success = false;
      result.error_message = "Requested failure";
      response.total_failed++;
    } else {
      result.success = false;
      result.error_message = "Unknown operation: " + item.operation;
      response.total_failed++;
    }

    response.results.push_back(result);
    response.total_processed++;

    // Stop on first error if requested
    if (!result.success && request.fail_on_error) {
      break;
    }
  }

  return common::Result<common::BatchResponse>(response);
}

void ReferenceServiceImpl::BatchProcessAsync(
    const common::BatchRequest& request,
    common::ResponseCallback<common::BatchResponse> callback) {

  auto result = BatchProcess(request);
  callback(result);
}

} // namespace reference
} // namespace benchmark
