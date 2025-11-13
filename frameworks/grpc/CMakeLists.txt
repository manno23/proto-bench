# gRPC implementation
if(GRPC_USE_PKG_CONFIG)
  # Using pkg-config - variables are already set
  message(STATUS "Using gRPC via pkg-config")
else()
  find_package(gRPC CONFIG REQUIRED)
  find_package(Protobuf CONFIG REQUIRED)
endif()

# Generate protobuf and gRPC sources
set(PROTO_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/schema/benchmark.proto
)

# Get protobuf and grpc plugin paths
find_program(PROTOBUF_PROTOC protoc)
if(NOT PROTOBUF_PROTOC)
  get_target_property(PROTOBUF_PROTOC protobuf::protoc IMPORTED_LOCATION_RELEASE)
  if(NOT PROTOBUF_PROTOC)
    get_target_property(PROTOBUF_PROTOC protobuf::protoc IMPORTED_LOCATION_DEBUG)
  endif()
  if(NOT PROTOBUF_PROTOC)
    get_target_property(PROTOBUF_PROTOC protobuf::protoc IMPORTED_LOCATION)
  endif()
endif()

find_program(GRPC_CPP_PLUGIN grpc_cpp_plugin)
if(NOT GRPC_CPP_PLUGIN)
  message(FATAL_ERROR "grpc_cpp_plugin not found")
endif()

# Generated source files
set(PROTO_SRCS "${CMAKE_CURRENT_BINARY_DIR}/benchmark.pb.cc")
set(PROTO_HDRS "${CMAKE_CURRENT_BINARY_DIR}/benchmark.pb.h")
set(GRPC_SRCS "${CMAKE_CURRENT_BINARY_DIR}/benchmark.grpc.pb.cc")
set(GRPC_HDRS "${CMAKE_CURRENT_BINARY_DIR}/benchmark.grpc.pb.h")

# Custom command to generate protobuf and gRPC sources
add_custom_command(
  OUTPUT ${PROTO_SRCS} ${PROTO_HDRS} ${GRPC_SRCS} ${GRPC_HDRS}
  COMMAND ${PROTOBUF_PROTOC}
  ARGS --cpp_out=${CMAKE_CURRENT_BINARY_DIR}
       --grpc_out=${CMAKE_CURRENT_BINARY_DIR}
       --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN}
       -I${CMAKE_CURRENT_SOURCE_DIR}/schema
       ${PROTO_FILES}
  DEPENDS ${PROTO_FILES}
  COMMENT "Generating gRPC protocol buffer sources"
)

# Library with generated sources
add_library(benchmark_grpc_proto
  ${PROTO_SRCS}
  ${GRPC_SRCS}
)

target_include_directories(benchmark_grpc_proto
  PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}
)

if(GRPC_USE_PKG_CONFIG)
  target_include_directories(benchmark_grpc_proto
    PUBLIC
      ${GRPC_INCLUDE_DIRS}
      ${PROTOBUF_INCLUDE_DIRS}
  )
  target_link_libraries(benchmark_grpc_proto
    PUBLIC
      ${GRPC_LIBRARIES}
      ${PROTOBUF_LIBRARIES}
  )
else()
  target_link_libraries(benchmark_grpc_proto
    PUBLIC
      gRPC::grpc++
      protobuf::libprotobuf
  )
endif()

# gRPC client implementation
add_library(benchmark_grpc_client
  client/grpc_client.cpp
)

target_include_directories(benchmark_grpc_client
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/client
)

target_link_libraries(benchmark_grpc_client
  PUBLIC
    benchmark_common
    benchmark_grpc_proto
)

# gRPC server implementation
add_library(benchmark_grpc_server
  server/grpc_server.cpp
)

target_include_directories(benchmark_grpc_server
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/server
)

target_link_libraries(benchmark_grpc_server
  PUBLIC
    benchmark_common
    benchmark_grpc_proto
    benchmark_grpc_client
)

# gRPC server executable
add_executable(grpc_server
  server/server_main.cpp
)

target_link_libraries(grpc_server
  PRIVATE
    benchmark_common
    benchmark_grpc_server
)
