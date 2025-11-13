// Benchmark runner main program
// This executable runs benchmarks against all available frameworks

#include "benchmark_scenario.h"
#include "benchmark_service.h"
#include <iostream>
#include <memory>
#include <vector>
#include <string>

using namespace benchmark;

// Forward declarations for scenario factory functions
namespace scenarios {
extern std::unique_ptr<BenchmarkScenario> CreateEchoBenchmark();
extern std::unique_ptr<BenchmarkScenario> CreateThroughputBenchmark();
extern std::unique_ptr<BenchmarkScenario> CreateReliabilityBenchmark();
}

// TODO: Add framework factory registration when implementations are complete
// #ifdef HAS_GRPC
// extern std::unique_ptr<common::IFrameworkFactory> CreateGrpcFactory();
// #endif
// #ifdef HAS_CAPNPROTO
// extern std::unique_ptr<common::IFrameworkFactory> CreateCapnProtoFactory();
// #endif
// #ifdef HAS_TRPC
// extern std::unique_ptr<common::IFrameworkFactory> CreateTrpcFactory();
// #endif

void PrintUsage(const char* program_name) {
  std::cout << "Usage: " << program_name << " [options]\n"
            << "\nOptions:\n"
            << "  --framework <name>     Framework to benchmark (grpc|capnproto|trpc|all)\n"
            << "  --scenario <name>      Scenario to run (echo|throughput|reliability|all)\n"
            << "  --duration <seconds>   Test duration (default: 10)\n"
            << "  --message-size <bytes> Message size (default: 1024)\n"
            << "  --address <addr>       Server address (default: localhost:50051)\n"
            << "  --output <file>        Output JSON results to file\n"
            << "  --verbose              Enable verbose output\n"
            << "  --help                 Show this help message\n"
            << std::endl;
}

int main(int argc, char* argv[]) {
  std::cout << "proto-bench: RPC Framework Benchmark\n" << std::endl;

  // Parse command line arguments
  scenarios::BenchmarkConfig config;
  std::string framework = "all";
  std::string scenario = "echo";

  for (int i = 1; i < argc; i++) {
    std::string arg = argv[i];

    if (arg == "--help" || arg == "-h") {
      PrintUsage(argv[0]);
      return 0;
    } else if (arg == "--framework" && i + 1 < argc) {
      framework = argv[++i];
    } else if (arg == "--scenario" && i + 1 < argc) {
      scenario = argv[++i];
    } else if (arg == "--duration" && i + 1 < argc) {
      config.duration_seconds = std::stoi(argv[++i]);
    } else if (arg == "--message-size" && i + 1 < argc) {
      config.message_size = std::stoul(argv[++i]);
    } else if (arg == "--address" && i + 1 < argc) {
      config.server_address = argv[++i];
    } else if (arg == "--output" && i + 1 < argc) {
      config.output_file = argv[++i];
    } else if (arg == "--verbose") {
      config.verbose = true;
    } else {
      std::cerr << "Unknown argument: " << arg << std::endl;
      PrintUsage(argv[0]);
      return 1;
    }
  }

  // Create benchmark scenarios
  std::vector<std::unique_ptr<scenarios::BenchmarkScenario>> scenarios_list;

  if (scenario == "echo" || scenario == "all") {
    scenarios_list.push_back(scenarios::CreateEchoBenchmark());
  }
  if (scenario == "throughput" || scenario == "all") {
    scenarios_list.push_back(scenarios::CreateThroughputBenchmark());
  }
  if (scenario == "reliability" || scenario == "all") {
    scenarios_list.push_back(scenarios::CreateReliabilityBenchmark());
  }

  if (scenarios_list.empty()) {
    std::cerr << "Error: No valid scenarios specified" << std::endl;
    return 1;
  }

  // Register available frameworks
  std::vector<std::unique_ptr<common::IFrameworkFactory>> factories;

  // TODO: Register factories when implementations are complete
  // #ifdef HAS_GRPC
  //   if (framework == "grpc" || framework == "all") {
  //     factories.push_back(CreateGrpcFactory());
  //   }
  // #endif
  // #ifdef HAS_CAPNPROTO
  //   if (framework == "capnproto" || framework == "all") {
  //     factories.push_back(CreateCapnProtoFactory());
  //   }
  // #endif
  // #ifdef HAS_TRPC
  //   if (framework == "trpc" || framework == "all") {
  //     factories.push_back(CreateTrpcFactory());
  //   }
  // #endif

  if (factories.empty()) {
    std::cerr << "Error: No frameworks available. Framework implementations are not yet complete.\n"
              << "Please implement the framework-specific client/server code." << std::endl;
    return 1;
  }

  // Run benchmarks
  std::cout << "Configuration:" << std::endl;
  std::cout << "  Frameworks: " << framework << std::endl;
  std::cout << "  Scenarios: " << scenario << std::endl;
  std::cout << "  Duration: " << config.duration_seconds << " seconds" << std::endl;
  std::cout << "  Message size: " << config.message_size << " bytes" << std::endl;
  std::cout << "  Server address: " << config.server_address << std::endl;
  std::cout << std::endl;

  std::vector<scenarios::BenchmarkResults> all_results;

  for (auto& factory : factories) {
    std::cout << "Testing framework: " << factory->GetName() << std::endl;

    for (auto& bench : scenarios_list) {
      std::cout << "  Running scenario: " << bench->GetName() << std::endl;

      auto client = factory->CreateClient();
      if (!client) {
        std::cerr << "    Failed to create client" << std::endl;
        continue;
      }

      auto results = bench->Run(client.get(), config);
      results.framework_name = factory->GetName();
      results.Print();

      all_results.push_back(results);
    }
  }

  // Output results to file if requested
  if (!config.output_file.empty()) {
    // TODO: Write JSON output to file
    std::cout << "JSON output not yet implemented" << std::endl;
  }

  std::cout << "Benchmark complete!" << std::endl;
  return 0;
}
