#pragma once

#include "benchmark_service.h"
#include "reference_service.h"
#include <memory>
#include <mutex>
#include <map>

namespace benchmark {
namespace inprocess {

// In-process framework for testing without network overhead
// This implements the benchmark interfaces but runs everything in-process
// Useful for baseline measurements and testing the benchmark framework itself

class InProcessClient : public common::IBenchmarkClient {
public:
  InProcessClient();
  ~InProcessClient() override = default;

  common::IBenchmarkService* GetService() override;
  bool Connect(const std::string& address) override;
  void Disconnect() override;
  bool IsConnected() const override;

private:
  std::shared_ptr<common::IBenchmarkService> service_;
  bool connected_;
};

class InProcessServer : public common::IBenchmarkServer {
public:
  explicit InProcessServer(std::shared_ptr<common::IBenchmarkService> service);
  ~InProcessServer() override = default;

  bool Start(const std::string& address) override;
  void Stop() override;
  bool IsRunning() const override;
  void Wait() override;

private:
  std::shared_ptr<common::IBenchmarkService> service_;
  std::string address_;
  bool running_;

  static std::mutex registry_mutex_;
  static std::map<std::string, std::shared_ptr<common::IBenchmarkService>> registry_;

  friend class InProcessClient;
};

class InProcessFactory : public common::IFrameworkFactory {
public:
  std::string GetName() const override;
  std::unique_ptr<common::IBenchmarkClient> CreateClient() override;
  std::unique_ptr<common::IBenchmarkServer> CreateServer(
      std::shared_ptr<common::IBenchmarkService> service) override;
};

// Factory function
std::unique_ptr<common::IFrameworkFactory> CreateInProcessFactory();

} // namespace inprocess
} // namespace benchmark
