// Cap'n Proto server implementation
#include "capnproto_server.h"
#include "capnproto_client.h"  // For conversion functions
#include <iostream>

namespace benchmark {
namespace capnp_impl {

// BenchmarkServiceImpl implementation
BenchmarkServiceImpl::BenchmarkServiceImpl(std::shared_ptr<common::IBenchmarkService> service)
    : service_(service) {
}

kj::Promise<void> BenchmarkServiceImpl::echo(EchoContext context) {
  auto params = context.getParams();
  auto request = params.getRequest();

  // Convert from Cap'n Proto to common format
  auto common_request = FromCapnp(request);

  // Call the actual service implementation
  auto result = service_->Echo(common_request);

  // Convert response back to Cap'n Proto format
  auto response_builder = context.getResults().initResponse();
  if (result.IsSuccess()) {
    ToCapnp(response_builder, result.GetValue());
  } else {
    // For errors, return empty response (Cap'n Proto uses exceptions for errors)
    // In production, we'd throw a proper KJ exception
    response_builder.setMessage(result.GetError());
  }

  return kj::READY_NOW;
}

kj::Promise<void> BenchmarkServiceImpl::streamData(StreamDataContext context) {
  // Streaming in Cap'n Proto is complex and requires pipeline implementation
  // For now, return not implemented
  return kj::READY_NOW;
}

kj::Promise<void> BenchmarkServiceImpl::uploadData(UploadDataContext context) {
  // Client streaming not yet implemented
  return kj::READY_NOW;
}

kj::Promise<void> BenchmarkServiceImpl::bidirectionalStream(BidirectionalStreamContext context) {
  // Bidirectional streaming not yet implemented
  return kj::READY_NOW;
}

kj::Promise<void> BenchmarkServiceImpl::batchProcess(BatchProcessContext context) {
  auto params = context.getParams();
  auto request = params.getRequest();

  // Convert from Cap'n Proto to common format
  auto common_request = FromCapnp(request);

  // Call the actual service implementation
  auto result = service_->BatchProcess(common_request);

  // Convert response back to Cap'n Proto format
  auto response_builder = context.getResults().initResponse();
  if (result.IsSuccess()) {
    ToCapnp(response_builder, result.GetValue());
  } else {
    // For errors, return empty response
    response_builder.setTotalProcessed(0);
    response_builder.setTotalFailed(0);
  }

  return kj::READY_NOW;
}

// CapnProtoBenchmarkServer implementation
CapnProtoBenchmarkServer::CapnProtoBenchmarkServer(std::shared_ptr<common::IBenchmarkService> service)
    : service_(service), running_(false) {
}

CapnProtoBenchmarkServer::~CapnProtoBenchmarkServer() {
  Stop();
}

bool CapnProtoBenchmarkServer::Start(const std::string& address) {
  if (running_) {
    return false;
  }

  try {
    running_ = true;
    server_thread_ = std::make_unique<std::thread>(
        &CapnProtoBenchmarkServer::RunServer, this, address);

    // Give server time to start
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    return true;
  } catch (...) {
    running_ = false;
    return false;
  }
}

void CapnProtoBenchmarkServer::RunServer(const std::string& address) {
  try {
    // Parse address (format: "host:port" or "*:port")
    size_t colon_pos = address.find(':');
    if (colon_pos == std::string::npos) {
      std::cerr << "Invalid address format: " << address << std::endl;
      running_ = false;
      return;
    }

    std::string host = address.substr(0, colon_pos);
    uint port = std::stoul(address.substr(colon_pos + 1));

    // Create the service implementation
    auto service_impl = kj::heap<BenchmarkServiceImpl>(service_);

    // Create and run the server
    if (host == "*" || host == "0.0.0.0") {
      // Listen on all interfaces
      rpc_server_ = std::make_unique<capnp::EzRpcServer>(
          kj::mv(service_impl), "*", port);
    } else {
      // Listen on specific interface
      rpc_server_ = std::make_unique<capnp::EzRpcServer>(
          kj::mv(service_impl), host, port);
    }

    // Wait for the server to be stopped
    auto& wait_scope = rpc_server_->getWaitScope();
    auto promise = kj::NEVER_DONE;

    // Keep the server running
    while (running_) {
      wait_scope.poll();
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
  } catch (const std::exception& e) {
    std::cerr << "Cap'n Proto server error: " << e.what() << std::endl;
    running_ = false;
  }
}

void CapnProtoBenchmarkServer::Stop() {
  if (running_) {
    running_ = false;
    if (server_thread_ && server_thread_->joinable()) {
      server_thread_->join();
    }
    rpc_server_.reset();
  }
}

void CapnProtoBenchmarkServer::Wait() {
  if (server_thread_ && server_thread_->joinable()) {
    server_thread_->join();
  }
}

bool CapnProtoBenchmarkServer::IsRunning() const {
  return running_;
}

} // namespace capnp_impl
} // namespace benchmark
