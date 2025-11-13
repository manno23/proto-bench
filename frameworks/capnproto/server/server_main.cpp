#include "capnproto_server.h"
#include "reference_service.h"
#include "capnproto_client.h"
#include <iostream>
#include <memory>
#include <signal.h>

std::unique_ptr<benchmark::common::IBenchmarkServer> server;

void SignalHandler(int signal) {
  if (server) {
    std::cout << "\nShutting down server..." << std::endl;
    server->Stop();
  }
  exit(0);
}

int main(int argc, char* argv[]) {
  std::string address = "0.0.0.0:50052";

  if (argc > 1) {
    address = argv[1];
  }

  signal(SIGINT, SignalHandler);
  signal(SIGTERM, SignalHandler);

  // Create reference service implementation
  auto service = std::make_shared<benchmark::reference::ReferenceServiceImpl>();

  // Create Cap'n Proto server
  server = std::make_unique<benchmark::capnp_impl::CapnProtoBenchmarkServer>(service);

  if (!server->Start(address)) {
    std::cerr << "Failed to start server on " << address << std::endl;
    return 1;
  }

  std::cout << "Cap'n Proto Benchmark Server running on " << address << std::endl;
  std::cout << "Press Ctrl+C to stop" << std::endl;

  server->Wait();

  return 0;
}
