cmake_minimum_required(VERSION 3.14)
project(proto-bench VERSION 0.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Options for enabling/disabling frameworks
option(BUILD_GRPC "Build gRPC implementation" ON)
option(BUILD_CAPNPROTO "Build Cap'n Proto implementation" ON)
option(BUILD_TRPC "Build tRPC-cpp implementation" ON)
option(BUILD_ORPC "Build oRPC implementation" OFF)
option(BUILD_BENCHMARKS "Build benchmark executables" ON)
option(BUILD_TESTS "Build test executables" ON)

# Find common dependencies
find_package(Threads REQUIRED)

# Detect and report available frameworks
message(STATUS "=== Framework Detection ===")

# Detect gRPC
if(BUILD_GRPC)
  find_package(gRPC CONFIG)
  find_package(Protobuf CONFIG)
  if(gRPC_FOUND AND Protobuf_FOUND)
    message(STATUS "gRPC: FOUND")
    set(HAS_GRPC TRUE)
  else()
    message(STATUS "gRPC: NOT FOUND (will be skipped)")
    set(HAS_GRPC FALSE)
  endif()
endif()

# Detect Cap'n Proto
if(BUILD_CAPNPROTO)
  find_package(CapnProto CONFIG)
  if(CapnProto_FOUND)
    message(STATUS "Cap'n Proto: FOUND")
    set(HAS_CAPNPROTO TRUE)
  else()
    message(STATUS "Cap'n Proto: NOT FOUND (will be skipped)")
    set(HAS_CAPNPROTO FALSE)
  endif()
endif()

# Detect tRPC-cpp
if(BUILD_TRPC)
  find_package(trpc CONFIG)
  if(trpc_FOUND)
    message(STATUS "tRPC-cpp: FOUND")
    set(HAS_TRPC TRUE)
  else()
    message(STATUS "tRPC-cpp: NOT FOUND (will be skipped)")
    set(HAS_TRPC FALSE)
  endif()
endif()

# Detect oRPC (custom detection logic needed)
if(BUILD_ORPC)
  # TODO: Add oRPC detection logic
  message(STATUS "oRPC: NOT IMPLEMENTED YET")
  set(HAS_ORPC FALSE)
endif()

message(STATUS "===========================")

# Common library (always built)
add_subdirectory(common)

# Framework implementations (conditionally built)
if(HAS_GRPC)
  add_subdirectory(frameworks/grpc)
endif()

if(HAS_CAPNPROTO)
  add_subdirectory(frameworks/capnproto)
endif()

if(HAS_TRPC)
  add_subdirectory(frameworks/trpc-cpp)
endif()

if(HAS_ORPC)
  add_subdirectory(frameworks/orpc)
endif()

# Benchmarks (if at least one framework is available)
if(BUILD_BENCHMARKS AND (HAS_GRPC OR HAS_CAPNPROTO OR HAS_TRPC OR HAS_ORPC))
  add_subdirectory(benchmarks)
endif()

# Tests
if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

# Print summary
message(STATUS "")
message(STATUS "=== Build Configuration ===")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Frameworks:")
message(STATUS "  gRPC: ${HAS_GRPC}")
message(STATUS "  Cap'n Proto: ${HAS_CAPNPROTO}")
message(STATUS "  tRPC-cpp: ${HAS_TRPC}")
message(STATUS "  oRPC: ${HAS_ORPC}")
message(STATUS "Build Benchmarks: ${BUILD_BENCHMARKS}")
message(STATUS "Build Tests: ${BUILD_TESTS}")
message(STATUS "===========================")
