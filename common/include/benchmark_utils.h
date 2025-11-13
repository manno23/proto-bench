#pragma once

#include <cstdint>
#include <vector>
#include <chrono>
#include <string>

namespace benchmark {
namespace common {
namespace utils {

// Time utilities
inline int64_t GetTimestampNanos() {
  return std::chrono::duration_cast<std::chrono::nanoseconds>(
      std::chrono::high_resolution_clock::now().time_since_epoch()
  ).count();
}

inline int64_t GetTimestampMicros() {
  return std::chrono::duration_cast<std::chrono::microseconds>(
      std::chrono::high_resolution_clock::now().time_since_epoch()
  ).count();
}

// Simple CRC32 implementation for checksum
class CRC32 {
public:
  CRC32();
  uint32_t Calculate(const uint8_t* data, size_t length);
  uint32_t Calculate(const std::vector<uint8_t>& data);

private:
  uint32_t table_[256];
  void InitTable();
};

// Generate random data for testing
std::vector<uint8_t> GenerateRandomData(size_t size, uint32_t seed = 0);

// Format bytes to human-readable string (e.g., "1.5 MB")
std::string FormatBytes(uint64_t bytes);

// Format duration to human-readable string (e.g., "123.45 ms")
std::string FormatDuration(int64_t nanos);

// Calculate throughput in MB/s
double CalculateThroughputMBps(uint64_t bytes, int64_t duration_ns);

// Calculate requests per second
double CalculateRequestsPerSecond(uint64_t count, int64_t duration_ns);

// Simple statistics calculator for latency measurements
class LatencyStats {
public:
  void AddSample(int64_t latency_ns);
  void Reset();

  uint64_t GetCount() const { return count_; }
  double GetMean() const;
  int64_t GetMin() const { return min_; }
  int64_t GetMax() const { return max_; }
  int64_t GetP50() const;
  int64_t GetP95() const;
  int64_t GetP99() const;

  std::string ToString() const;

private:
  std::vector<int64_t> samples_;
  uint64_t count_ = 0;
  int64_t sum_ = 0;
  int64_t min_ = INT64_MAX;
  int64_t max_ = 0;
  bool sorted_ = false;

  void EnsureSorted();
  int64_t GetPercentile(double percentile) const;
};

} // namespace utils
} // namespace common
} // namespace benchmark
