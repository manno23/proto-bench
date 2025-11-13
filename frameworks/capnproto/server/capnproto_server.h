#pragma once

#include "benchmark_service.h"
#include "benchmark.capnp.h"
#include <capnp/ez-rpc.h>
#include <kj/async.h>
#include <memory>
#include <thread>

namespace benchmark {
namespace capnp_impl {

// Cap'n Proto server implementation
class BenchmarkServiceImpl final : public BenchmarkService::Server {
public:
  explicit BenchmarkServiceImpl(std::shared_ptr<common::IBenchmarkService> service);
  ~BenchmarkServiceImpl() override = default;

  // Implement Cap'n Proto service methods
  kj::Promise<void> echo(EchoContext context) override;
  kj::Promise<void> streamData(StreamDataContext context) override;
  kj::Promise<void> uploadData(UploadDataContext context) override;
  kj::Promise<void> bidirectionalStream(BidirectionalStreamContext context) override;
  kj::Promise<void> batchProcess(BatchProcessContext context) override;

private:
  std::shared_ptr<common::IBenchmarkService> service_;
};

// Cap'n Proto server wrapper
class CapnProtoBenchmarkServer : public common::IBenchmarkServer {
public:
  explicit CapnProtoBenchmarkServer(std::shared_ptr<common::IBenchmarkService> service);
  ~CapnProtoBenchmarkServer() override;

  bool Start(const std::string& address) override;
  void Stop() override;
  void Wait() override;
  bool IsRunning() const override;

private:
  void RunServer(const std::string& address);

  std::shared_ptr<common::IBenchmarkService> service_;
  std::unique_ptr<std::thread> server_thread_;
  std::atomic<bool> running_;
  std::unique_ptr<capnp::EzRpcServer> rpc_server_;
};

} // namespace capnp_impl
} // namespace benchmark
