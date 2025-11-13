# Common library - framework-agnostic types and utilities
add_library(benchmark_common
  src/benchmark_utils.cpp
)

target_include_directories(benchmark_common
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(benchmark_common
  PUBLIC
    Threads::Threads
)

# Set compiler warnings
target_compile_options(benchmark_common PRIVATE
  $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
  $<$<CXX_COMPILER_ID:MSVC>:/W4>
)
