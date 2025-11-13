#include "benchmark_scenario.h"
#include <iostream>
#include <thread>
#include <atomic>
#include <vector>

namespace benchmark {
namespace scenarios {

class EchoBenchmark : public BenchmarkScenario {
public:
  EchoBenchmark() : BenchmarkScenario("Echo Latency") {}

  BenchmarkResults Run(
      common::IBenchmarkClient* client,
      const BenchmarkConfig& config) override {

    BenchmarkResults results;
    results.scenario_name = name_;
    results.framework_name = "unknown";  // Set by caller

    if (!client->Connect(config.server_address)) {
      std::cerr << "Failed to connect to server" << std::endl;
      return results;
    }

    auto* service = client->GetService();
    if (!service) {
      std::cerr << "Failed to get service" << std::endl;
      return results;
    }

    // Warm-up phase
    if (config.warmup_seconds > 0) {
      if (config.verbose) {
        std::cout << "Warming up for " << config.warmup_seconds << " seconds..." << std::endl;
      }

      auto warmup_end = std::chrono::steady_clock::now() +
                        std::chrono::seconds(config.warmup_seconds);

      while (std::chrono::steady_clock::now() < warmup_end) {
        common::EchoRequest req;
        req.message = "warmup";
        req.timestamp = common::utils::GetTimestampNanos();
        service->Echo(req);
      }
    }

    // Main benchmark phase
    if (config.verbose) {
      std::cout << "Running benchmark..." << std::endl;
    }

    auto start_time = std::chrono::steady_clock::now();
    auto end_time = start_time + std::chrono::seconds(config.duration_seconds);

    uint32_t sequence_number = 0;
    std::string test_message(config.message_size, 'x');

    while (std::chrono::steady_clock::now() < end_time) {
      common::EchoRequest request;
      request.message = test_message;
      request.timestamp = common::utils::GetTimestampNanos();
      request.sequence_number = sequence_number++;

      auto call_start = common::utils::GetTimestampNanos();
      auto result = service->Echo(request);
      auto call_end = common::utils::GetTimestampNanos();

      if (result.ok()) {
        int64_t latency = call_end - call_start;
        results.latency_stats.AddSample(latency);
        results.successful_requests++;
        results.total_bytes += request.message.size() + result.value.message.size();
      } else {
        results.failed_requests++;
      }

      results.total_requests++;
    }

    auto actual_end = std::chrono::steady_clock::now();
    results.total_duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
        actual_end - start_time
    ).count();

    // Calculate metrics
    results.requests_per_second = common::utils::CalculateRequestsPerSecond(
        results.successful_requests,
        results.total_duration_ns
    );

    results.throughput_mbps = common::utils::CalculateThroughputMBps(
        results.total_bytes,
        results.total_duration_ns
    );

    results.success_rate = results.total_requests > 0
        ? (double)results.successful_requests / results.total_requests
        : 0.0;

    client->Disconnect();

    return results;
  }
};

// Factory function
std::unique_ptr<BenchmarkScenario> CreateEchoBenchmark() {
  return std::make_unique<EchoBenchmark>();
}

} // namespace scenarios
} // namespace benchmark
