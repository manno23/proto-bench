#pragma once

#include "benchmark_service.h"
#include "benchmark_types.h"
#include "benchmark_utils.h"
#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>

namespace benchmark {
namespace reference {

// Reference implementation of the benchmark service
// This provides a simple, correct implementation for testing and baseline comparison
class ReferenceServiceImpl : public common::IBenchmarkService {
public:
  ReferenceServiceImpl() = default;
  ~ReferenceServiceImpl() override = default;

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
  common::utils::CRC32 crc32_;
};

} // namespace reference
} // namespace benchmark
