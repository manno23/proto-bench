# Cap'n Proto implementation
find_package(CapnProto CONFIG REQUIRED)

# Generate Cap'n Proto sources
set(CAPNP_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/schema/benchmark.capnp
)

capnp_generate_cpp(CAPNP_SRCS CAPNP_HDRS ${CAPNP_FILES})

# Library with generated sources
add_library(benchmark_capnproto_schema
  ${CAPNP_SRCS}
)

target_include_directories(benchmark_capnproto_schema
  PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(benchmark_capnproto_schema
  PUBLIC
    CapnProto::capnp-rpc
)

# Cap'n Proto client implementation
add_library(benchmark_capnproto_client
  client/capnproto_client.cpp
  client/capnproto_client.h
)

target_include_directories(benchmark_capnproto_client
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/client
)

target_link_libraries(benchmark_capnproto_client
  PUBLIC
    benchmark_common
    benchmark_capnproto_schema
)

# Cap'n Proto server implementation
add_library(benchmark_capnproto_server
  server/capnproto_server.cpp
  server/capnproto_server.h
)

target_include_directories(benchmark_capnproto_server
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/server
)

target_link_libraries(benchmark_capnproto_server
  PUBLIC
    benchmark_common
    benchmark_capnproto_schema
    benchmark_capnproto_client
)

# Cap'n Proto server executable
add_executable(capnproto_server
  server/server_main.cpp
)

target_link_libraries(capnproto_server
  PRIVATE
    benchmark_common
    benchmark_capnproto_server
)
