#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>

namespace benchmark {
namespace common {

// Error codes consistent across all frameworks
enum class ErrorCode {
  OK = 0,
  INVALID_ARGUMENT = 1,
  DEADLINE_EXCEEDED = 2,
  NOT_FOUND = 3,
  INTERNAL = 4,
  UNAVAILABLE = 5
};

// Forward declarations
struct EchoRequest;
struct EchoResponse;
struct StreamRequest;
struct DataChunk;
struct UploadResponse;
struct BatchRequest;
struct BatchResponse;
struct BatchItem;
struct BatchResult;

// Echo messages
struct EchoRequest {
  std::string message;
  int64_t timestamp;
  uint32_t sequence_number;

  EchoRequest() : timestamp(0), sequence_number(0) {}
};

struct EchoResponse {
  std::string message;
  int64_t client_timestamp;
  int64_t server_timestamp;
  uint32_t sequence_number;

  EchoResponse()
    : client_timestamp(0),
      server_timestamp(0),
      sequence_number(0) {}
};

// Streaming messages
struct StreamRequest {
  uint32_t chunk_size;
  uint32_t chunk_count;
  uint32_t delay_ms;

  StreamRequest() : chunk_size(0), chunk_count(0), delay_ms(0) {}
};

struct DataChunk {
  uint32_t sequence_number;
  std::vector<uint8_t> data;
  uint32_t checksum;
  int64_t timestamp;

  DataChunk() : sequence_number(0), checksum(0), timestamp(0) {}
};

struct UploadResponse {
  uint64_t total_bytes;
  uint32_t chunk_count;
  int64_t duration_ns;
  bool checksum_valid;

  UploadResponse()
    : total_bytes(0),
      chunk_count(0),
      duration_ns(0),
      checksum_valid(true) {}
};

// Batch processing messages
struct BatchItem {
  std::string id;
  std::string operation;
  std::vector<uint8_t> data;
};

struct BatchResult {
  std::string id;
  bool success;
  std::string error_message;
  std::vector<uint8_t> result_data;

  BatchResult() : success(true) {}
};

struct BatchRequest {
  std::vector<BatchItem> items;
  bool fail_on_error;

  BatchRequest() : fail_on_error(false) {}
};

struct BatchResponse {
  std::vector<BatchResult> results;
  uint32_t total_processed;
  uint32_t total_failed;

  BatchResponse() : total_processed(0), total_failed(0) {}
};

// Result wrapper for error handling
template<typename T>
struct Result {
  T value;
  ErrorCode error_code;
  std::string error_message;

  Result() : error_code(ErrorCode::OK) {}
  explicit Result(const T& val) : value(val), error_code(ErrorCode::OK) {}
  Result(ErrorCode code, const std::string& msg)
    : error_code(code), error_message(msg) {}

  bool ok() const { return error_code == ErrorCode::OK; }
};

} // namespace common
} // namespace benchmark
