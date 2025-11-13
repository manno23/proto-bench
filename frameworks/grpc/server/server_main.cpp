#include "grpc_server.h"
#include "reference_service.h"
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
  std::string address = "0.0.0.0:50051";

  if (argc > 1) {
    address = argv[1];
  }

  signal(SIGINT, SignalHandler);
  signal(SIGTERM, SignalHandler);

  // Create reference service implementation
  auto service = std::make_shared<benchmark::reference::ReferenceServiceImpl>();

  // Create gRPC server
  auto factory = benchmark::grpc_impl::CreateGrpcFactory();
  server = factory->CreateServer(service);

  if (!server->Start(address)) {
    std::cerr << "Failed to start server on " << address << std::endl;
    return 1;
  }

  std::cout << "gRPC Benchmark Server running..." << std::endl;
  std::cout << "Press Ctrl+C to stop" << std::endl;

  server->Wait();

  return 0;
}
