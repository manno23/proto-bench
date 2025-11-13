#pragma once

#include "benchmark_service.h"
#include "benchmark_utils.h"
#include <string>
#include <memory>

namespace benchmark {
namespace scenarios {

// Configuration for a benchmark scenario
struct BenchmarkConfig {
  // Test duration or iteration count
  int duration_seconds = 10;
  int iterations = 0;  // 0 means use duration instead

  // Concurrency
  int num_clients = 1;
  int num_threads_per_client = 1;

  // Data sizes
  size_t message_size = 1024;
  size_t batch_size = 100;

  // Server address
  std::string server_address = "localhost:50051";

  // Warm-up period before measuring
  int warmup_seconds = 1;

  // Output settings
  bool verbose = false;
  std::string output_file;
};

// Results from a benchmark run
struct BenchmarkResults {
  std::string scenario_name;
  std::string framework_name;

  // Latency statistics
  common::utils::LatencyStats latency_stats;

  // Throughput
  uint64_t total_requests = 0;
  uint64_t total_bytes = 0;
  int64_t total_duration_ns = 0;
  double requests_per_second = 0.0;
  double throughput_mbps = 0.0;

  // Reliability
  uint64_t successful_requests = 0;
  uint64_t failed_requests = 0;
  double success_rate = 0.0;

  // Resource usage (if measured)
  double avg_cpu_percent = 0.0;
  uint64_t peak_memory_bytes = 0;

  // Print results
  void Print() const;

  // Export to JSON
  std::string ToJSON() const;
};

// Base class for benchmark scenarios
class BenchmarkScenario {
public:
  explicit BenchmarkScenario(const std::string& name) : name_(name) {}
  virtual ~BenchmarkScenario() = default;

  // Run the benchmark with the given client
  virtual BenchmarkResults Run(
      common::IBenchmarkClient* client,
      const BenchmarkConfig& config) = 0;

  const std::string& GetName() const { return name_; }

protected:
  std::string name_;
};

} // namespace scenarios
} // namespace benchmark
