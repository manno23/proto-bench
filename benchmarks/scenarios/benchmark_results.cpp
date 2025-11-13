#include "benchmark_scenario.h"
#include <iostream>
#include <sstream>
#include <iomanip>

namespace benchmark {
namespace scenarios {

void BenchmarkResults::Print() const {
  std::cout << "\n========================================" << std::endl;
  std::cout << "Benchmark Results" << std::endl;
  std::cout << "========================================" << std::endl;
  std::cout << "Scenario: " << scenario_name << std::endl;
  std::cout << "Framework: " << framework_name << std::endl;
  std::cout << std::endl;

  std::cout << "Throughput:" << std::endl;
  std::cout << "  Requests/sec: " << std::fixed << std::setprecision(2)
            << requests_per_second << std::endl;
  std::cout << "  Throughput: " << std::fixed << std::setprecision(2)
            << throughput_mbps << " MB/s" << std::endl;
  std::cout << "  Total requests: " << total_requests << std::endl;
  std::cout << "  Total bytes: " << common::utils::FormatBytes(total_bytes) << std::endl;
  std::cout << "  Duration: " << common::utils::FormatDuration(total_duration_ns) << std::endl;
  std::cout << std::endl;

  std::cout << "Latency:" << std::endl;
  std::cout << "  " << latency_stats.ToString() << std::endl;
  std::cout << std::endl;

  std::cout << "Reliability:" << std::endl;
  std::cout << "  Successful: " << successful_requests << std::endl;
  std::cout << "  Failed: " << failed_requests << std::endl;
  std::cout << "  Success rate: " << std::fixed << std::setprecision(2)
            << (success_rate * 100.0) << "%" << std::endl;
  std::cout << std::endl;

  if (peak_memory_bytes > 0 || avg_cpu_percent > 0) {
    std::cout << "Resources:" << std::endl;
    if (peak_memory_bytes > 0) {
      std::cout << "  Peak memory: " << common::utils::FormatBytes(peak_memory_bytes) << std::endl;
    }
    if (avg_cpu_percent > 0) {
      std::cout << "  Avg CPU: " << std::fixed << std::setprecision(1)
                << avg_cpu_percent << "%" << std::endl;
    }
    std::cout << std::endl;
  }

  std::cout << "========================================\n" << std::endl;
}

std::string BenchmarkResults::ToJSON() const {
  std::ostringstream json;
  json << std::fixed << std::setprecision(2);

  json << "{\n";
  json << "  \"scenario\": \"" << scenario_name << "\",\n";
  json << "  \"framework\": \"" << framework_name << "\",\n";
  json << "  \"throughput\": {\n";
  json << "    \"requests_per_second\": " << requests_per_second << ",\n";
  json << "    \"throughput_mbps\": " << throughput_mbps << ",\n";
  json << "    \"total_requests\": " << total_requests << ",\n";
  json << "    \"total_bytes\": " << total_bytes << ",\n";
  json << "    \"duration_ns\": " << total_duration_ns << "\n";
  json << "  },\n";
  json << "  \"latency\": {\n";
  json << "    \"count\": " << latency_stats.GetCount() << ",\n";
  json << "    \"mean_ns\": " << static_cast<int64_t>(latency_stats.GetMean()) << ",\n";
  json << "    \"min_ns\": " << latency_stats.GetMin() << ",\n";
  json << "    \"max_ns\": " << latency_stats.GetMax() << ",\n";
  json << "    \"p50_ns\": " << latency_stats.GetP50() << ",\n";
  json << "    \"p95_ns\": " << latency_stats.GetP95() << ",\n";
  json << "    \"p99_ns\": " << latency_stats.GetP99() << "\n";
  json << "  },\n";
  json << "  \"reliability\": {\n";
  json << "    \"successful_requests\": " << successful_requests << ",\n";
  json << "    \"failed_requests\": " << failed_requests << ",\n";
  json << "    \"success_rate\": " << success_rate << "\n";
  json << "  }";

  if (peak_memory_bytes > 0 || avg_cpu_percent > 0) {
    json << ",\n  \"resources\": {\n";
    if (peak_memory_bytes > 0) {
      json << "    \"peak_memory_bytes\": " << peak_memory_bytes;
      if (avg_cpu_percent > 0) json << ",";
      json << "\n";
    }
    if (avg_cpu_percent > 0) {
      json << "    \"avg_cpu_percent\": " << avg_cpu_percent << "\n";
    }
    json << "  }";
  }

  json << "\n}";
  return json.str();
}

} // namespace scenarios
} // namespace benchmark
