# Cap'n Proto Implementation TODO

## Current Status
- Schema defined (benchmark.capnp)
- Client adapter implemented (capnproto_client.h/cpp)
- Server adapter implemented (capnproto_server.h/cpp)
- Server executable created (server_main.cpp)
- CMakeLists.txt configured

## Known Issues

### Build Dependencies
Cap'n Proto CMake configuration requires libatomic but has detection issues:
- `libatomic1` is installed but CMake's `check_library_exists` fails to detect it
- Error: "libatomic not found" from CapnProtoConfig.cmake:108
- Attempted fix: Created symlink `/usr/lib/x86_64-linux-gnu/libatomic.so` -> `libatomic.so.1`
- Still failing - may need different CMake configuration approach

### Possible Solutions
1. Try older version of Cap'n Proto with different CMake detection
2. Build Cap'n Proto from source with custom CMake flags
3. Set CMake variables to override library detection
4. Use FetchContent to build Cap'n Proto as part of project

## Implementation Notes
- Cap'n Proto uses promise-based async model (KJ library)
- EzRpcClient/EzRpcServer provide simplified RPC setup
- Streaming implementation needs special attention (different from gRPC model)
- Currently only Echo and BatchProcess methods fully implemented
- Streaming methods return UNIMPLEMENTED status

## Next Steps
1. Resolve libatomic detection issue
2. Test basic Echo functionality
3. Implement streaming methods properly using Cap'n Proto pipelines
4. Add server executable to benchmark runner
5. Compare performance against gRPC and in-process implementations
