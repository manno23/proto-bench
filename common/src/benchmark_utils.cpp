#include "benchmark_utils.h"
#include <algorithm>
#include <random>
#include <sstream>
#include <iomanip>
#include <cstring>

namespace benchmark {
namespace common {
namespace utils {

// CRC32 implementation
CRC32::CRC32() {
  InitTable();
}

void CRC32::InitTable() {
  for (uint32_t i = 0; i < 256; i++) {
    uint32_t crc = i;
    for (int j = 0; j < 8; j++) {
      if (crc & 1) {
        crc = (crc >> 1) ^ 0xEDB88320;
      } else {
        crc >>= 1;
      }
    }
    table_[i] = crc;
  }
}

uint32_t CRC32::Calculate(const uint8_t* data, size_t length) {
  uint32_t crc = 0xFFFFFFFF;
  for (size_t i = 0; i < length; i++) {
    uint8_t index = (crc ^ data[i]) & 0xFF;
    crc = (crc >> 8) ^ table_[index];
  }
  return crc ^ 0xFFFFFFFF;
}

uint32_t CRC32::Calculate(const std::vector<uint8_t>& data) {
  return Calculate(data.data(), data.size());
}

// Generate random data
std::vector<uint8_t> GenerateRandomData(size_t size, uint32_t seed) {
  std::vector<uint8_t> data(size);
  std::mt19937 gen(seed == 0 ? std::random_device{}() : seed);
  std::uniform_int_distribution<> dis(0, 255);

  for (size_t i = 0; i < size; i++) {
    data[i] = static_cast<uint8_t>(dis(gen));
  }

  return data;
}

// Format bytes
std::string FormatBytes(uint64_t bytes) {
  const char* units[] = {"B", "KB", "MB", "GB", "TB"};
  int unit_index = 0;
  double size = static_cast<double>(bytes);

  while (size >= 1024.0 && unit_index < 4) {
    size /= 1024.0;
    unit_index++;
  }

  std::ostringstream oss;
  oss << std::fixed << std::setprecision(2) << size << " " << units[unit_index];
  return oss.str();
}

// Format duration
std::string FormatDuration(int64_t nanos) {
  if (nanos < 1000) {
    return std::to_string(nanos) + " ns";
  } else if (nanos < 1000000) {
    double micros = nanos / 1000.0;
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << micros << " μs";
    return oss.str();
  } else if (nanos < 1000000000) {
    double millis = nanos / 1000000.0;
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << millis << " ms";
    return oss.str();
  } else {
    double secs = nanos / 1000000000.0;
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << secs << " s";
    return oss.str();
  }
}

// Calculate throughput
double CalculateThroughputMBps(uint64_t bytes, int64_t duration_ns) {
  if (duration_ns == 0) return 0.0;
  double seconds = duration_ns / 1e9;
  double megabytes = bytes / (1024.0 * 1024.0);
  return megabytes / seconds;
}

// Calculate requests per second
double CalculateRequestsPerSecond(uint64_t count, int64_t duration_ns) {
  if (duration_ns == 0) return 0.0;
  double seconds = duration_ns / 1e9;
  return count / seconds;
}

// LatencyStats implementation
void LatencyStats::AddSample(int64_t latency_ns) {
  samples_.push_back(latency_ns);
  count_++;
  sum_ += latency_ns;
  min_ = std::min(min_, latency_ns);
  max_ = std::max(max_, latency_ns);
  sorted_ = false;
}

void LatencyStats::Reset() {
  samples_.clear();
  count_ = 0;
  sum_ = 0;
  min_ = INT64_MAX;
  max_ = 0;
  sorted_ = false;
}

double LatencyStats::GetMean() const {
  if (count_ == 0) return 0.0;
  return static_cast<double>(sum_) / count_;
}

void LatencyStats::EnsureSorted() {
  if (!sorted_ && !samples_.empty()) {
    std::sort(samples_.begin(), samples_.end());
    sorted_ = true;
  }
}

int64_t LatencyStats::GetPercentile(double percentile) const {
  if (samples_.empty()) return 0;
  const_cast<LatencyStats*>(this)->EnsureSorted();

  size_t index = static_cast<size_t>(percentile * samples_.size());
  if (index >= samples_.size()) {
    index = samples_.size() - 1;
  }
  return samples_[index];
}

int64_t LatencyStats::GetP50() const {
  return GetPercentile(0.50);
}

int64_t LatencyStats::GetP95() const {
  return GetPercentile(0.95);
}

int64_t LatencyStats::GetP99() const {
  return GetPercentile(0.99);
}

std::string LatencyStats::ToString() const {
  std::ostringstream oss;
  oss << "Count: " << count_ << ", "
      << "Mean: " << FormatDuration(static_cast<int64_t>(GetMean())) << ", "
      << "Min: " << FormatDuration(min_) << ", "
      << "Max: " << FormatDuration(max_) << ", "
      << "P50: " << FormatDuration(GetP50()) << ", "
      << "P95: " << FormatDuration(GetP95()) << ", "
      << "P99: " << FormatDuration(GetP99());
  return oss.str();
}

} // namespace utils
} // namespace common
} // namespace benchmark
