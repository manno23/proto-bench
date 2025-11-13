#include "inprocess_framework.h"
#include <iostream>

namespace benchmark {
namespace inprocess {

// Static registry for in-process "servers"
std::mutex InProcessServer::registry_mutex_;
std::map<std::string, std::shared_ptr<common::IBenchmarkService>>
    InProcessServer::registry_;

// InProcessClient implementation
InProcessClient::InProcessClient() : connected_(false) {}

common::IBenchmarkService* InProcessClient::GetService() {
  return service_.get();
}

bool InProcessClient::Connect(const std::string& address) {
  std::lock_guard<std::mutex> lock(InProcessServer::registry_mutex_);

  auto it = InProcessServer::registry_.find(address);
  if (it != InProcessServer::registry_.end()) {
    service_ = it->second;
    connected_ = true;
    return true;
  }

  // If no server is registered, create a default reference service
  // This allows benchmarks to run without explicitly starting a server
  service_ = std::make_shared<reference::ReferenceServiceImpl>();
  connected_ = true;
  return true;
}

void InProcessClient::Disconnect() {
  service_.reset();
  connected_ = false;
}

bool InProcessClient::IsConnected() const {
  return connected_;
}

// InProcessServer implementation
InProcessServer::InProcessServer(
    std::shared_ptr<common::IBenchmarkService> service)
  : service_(service), running_(false) {}

bool InProcessServer::Start(const std::string& address) {
  std::lock_guard<std::mutex> lock(registry_mutex_);

  if (registry_.find(address) != registry_.end()) {
    std::cerr << "InProcessServer: Address " << address
              << " already in use" << std::endl;
    return false;
  }

  registry_[address] = service_;
  address_ = address;
  running_ = true;
  return true;
}

void InProcessServer::Stop() {
  std::lock_guard<std::mutex> lock(registry_mutex_);

  if (running_ && !address_.empty()) {
    registry_.erase(address_);
    running_ = false;
  }
}

bool InProcessServer::IsRunning() const {
  return running_;
}

void InProcessServer::Wait() {
  // In-process server doesn't block
  // In a real implementation, this would wait for shutdown signal
}

// InProcessFactory implementation
std::string InProcessFactory::GetName() const {
  return "InProcess (Reference)";
}

std::unique_ptr<common::IBenchmarkClient> InProcessFactory::CreateClient() {
  return std::make_unique<InProcessClient>();
}

std::unique_ptr<common::IBenchmarkServer> InProcessFactory::CreateServer(
    std::shared_ptr<common::IBenchmarkService> service) {
  return std::make_unique<InProcessServer>(service);
}

// Factory function
std::unique_ptr<common::IFrameworkFactory> CreateInProcessFactory() {
  return std::make_unique<InProcessFactory>();
}

} // namespace inprocess
} // namespace benchmark
