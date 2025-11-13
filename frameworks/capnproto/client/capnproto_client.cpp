// Cap'n Proto client implementation
// This file will contain the Cap'n Proto-specific implementation of IBenchmarkClient

#include "benchmark_service.h"
#include "benchmark_types.h"

// TODO: Include generated Cap'n Proto headers
// #include "benchmark.capnp.h"

namespace benchmark {
namespace capnp_impl {

// TODO: Implement Cap'n Proto client wrapper that implements common::IBenchmarkClient
// Cap'n Proto uses a different RPC model than gRPC
// Reference: https://capnproto.org/cxxrpc.html

// class CapnProtoBenchmarkClient : public common::IBenchmarkClient {
// public:
//   CapnProtoBenchmarkClient();
//   ~CapnProtoBenchmarkClient() override;
//
//   common::IBenchmarkService* GetService() override;
//   bool Connect(const std::string& address) override;
//   void Disconnect() override;
//   bool IsConnected() const override;
//
// private:
//   capnp::EzRpcClient client_;
//   BenchmarkService::Client capability_;
//   // ... other members
// };

} // namespace capnp_impl
} // namespace benchmark
