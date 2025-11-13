# Benchmark scenarios and executables

# Create a benchmark library with common scenarios
add_library(benchmark_scenarios
  scenarios/benchmark_results.cpp
  scenarios/echo_benchmark.cpp
  scenarios/throughput_benchmark.cpp
  scenarios/reliability_benchmark.cpp
)

target_include_directories(benchmark_scenarios
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/scenarios
)

target_link_libraries(benchmark_scenarios
  PUBLIC
    benchmark_common
)

# Benchmark runner executable
add_executable(benchmark_runner
  benchmark_runner.cpp
)

target_link_libraries(benchmark_runner
  PRIVATE
    benchmark_common
    benchmark_scenarios
)

# Link framework-specific implementations if available
if(HAS_GRPC)
  target_link_libraries(benchmark_runner PRIVATE
    benchmark_grpc_client
    benchmark_grpc_server
  )
  target_compile_definitions(benchmark_runner PRIVATE HAS_GRPC)
endif()

if(HAS_CAPNPROTO)
  target_link_libraries(benchmark_runner PRIVATE
    benchmark_capnproto_client
    benchmark_capnproto_server
  )
  target_compile_definitions(benchmark_runner PRIVATE HAS_CAPNPROTO)
endif()

if(HAS_TRPC)
  target_link_libraries(benchmark_runner PRIVATE
    benchmark_trpc_client
    benchmark_trpc_server
  )
  target_compile_definitions(benchmark_runner PRIVATE HAS_TRPC)
endif()
