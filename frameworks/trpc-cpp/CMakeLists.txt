# tRPC-cpp implementation
find_package(trpc CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)

# Generate protobuf and tRPC sources
set(PROTO_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/schema/benchmark.proto
)

# tRPC uses protobuf with custom plugin
get_target_property(PROTOBUF_PROTOC protobuf::protoc IMPORTED_LOCATION_RELEASE)
if(NOT PROTOBUF_PROTOC)
  get_target_property(PROTOBUF_PROTOC protobuf::protoc IMPORTED_LOCATION_DEBUG)
endif()
if(NOT PROTOBUF_PROTOC)
  get_target_property(PROTOBUF_PROTOC protobuf::protoc IMPORTED_LOCATION)
endif()

# Find trpc plugin (adjust path as needed)
find_program(TRPC_CPP_PLUGIN trpc_cpp_plugin)

# Generated source files
set(PROTO_SRCS "${CMAKE_CURRENT_BINARY_DIR}/benchmark.pb.cc")
set(PROTO_HDRS "${CMAKE_CURRENT_BINARY_DIR}/benchmark.pb.h")

if(TRPC_CPP_PLUGIN)
  set(TRPC_SRCS "${CMAKE_CURRENT_BINARY_DIR}/benchmark.trpc.pb.cc")
  set(TRPC_HDRS "${CMAKE_CURRENT_BINARY_DIR}/benchmark.trpc.pb.h")

  add_custom_command(
    OUTPUT ${PROTO_SRCS} ${PROTO_HDRS} ${TRPC_SRCS} ${TRPC_HDRS}
    COMMAND ${PROTOBUF_PROTOC}
    ARGS --cpp_out=${CMAKE_CURRENT_BINARY_DIR}
         --trpc_out=${CMAKE_CURRENT_BINARY_DIR}
         --plugin=protoc-gen-trpc=${TRPC_CPP_PLUGIN}
         -I${CMAKE_CURRENT_SOURCE_DIR}/schema
         ${PROTO_FILES}
    DEPENDS ${PROTO_FILES}
    COMMENT "Generating tRPC protocol buffer sources"
  )
else()
  # Fallback to protobuf only
  message(WARNING "trpc_cpp_plugin not found, using protobuf only")
  add_custom_command(
    OUTPUT ${PROTO_SRCS} ${PROTO_HDRS}
    COMMAND ${PROTOBUF_PROTOC}
    ARGS --cpp_out=${CMAKE_CURRENT_BINARY_DIR}
         -I${CMAKE_CURRENT_SOURCE_DIR}/schema
         ${PROTO_FILES}
    DEPENDS ${PROTO_FILES}
    COMMENT "Generating tRPC protocol buffer sources (no plugin)"
  )
endif()

# Library with generated sources
add_library(benchmark_trpc_proto
  ${PROTO_SRCS}
  ${TRPC_SRCS}
)

target_include_directories(benchmark_trpc_proto
  PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(benchmark_trpc_proto
  PUBLIC
    trpc
    protobuf::libprotobuf
)

# tRPC client implementation
add_library(benchmark_trpc_client
  client/trpc_client.cpp
)

target_link_libraries(benchmark_trpc_client
  PUBLIC
    benchmark_common
    benchmark_trpc_proto
)

# tRPC server implementation
add_library(benchmark_trpc_server
  server/trpc_server.cpp
)

target_link_libraries(benchmark_trpc_server
  PUBLIC
    benchmark_common
    benchmark_trpc_proto
)
