# Common Schema Definition

This document defines the common message schema that all framework implementations must support.

## Service Interface

### BenchmarkService

A service for benchmarking RPC framework performance.

#### Methods

1. **Echo** - Simple request/response for latency testing
   - Request: `EchoRequest`
   - Response: `EchoResponse`
   - Description: Returns the input message immediately

2. **StreamData** - Server streaming for throughput testing
   - Request: `StreamRequest`
   - Response: Stream of `DataChunk`
   - Description: Streams a specified amount of data to the client

3. **UploadData** - Client streaming for upload throughput testing
   - Request: Stream of `DataChunk`
   - Response: `UploadResponse`
   - Description: Receives a stream of data from the client

4. **BidirectionalStream** - Bidirectional streaming
   - Request: Stream of `DataChunk`
   - Response: Stream of `DataChunk`
   - Description: Full duplex communication for complex scenarios

5. **BatchProcess** - Batch processing for reliability testing
   - Request: `BatchRequest`
   - Response: `BatchResponse`
   - Description: Process multiple items and return results with error handling

## Message Types

### EchoRequest
- `message` (string): The message to echo
- `timestamp` (int64): Client timestamp in nanoseconds
- `sequence_number` (uint32): Request sequence number

### EchoResponse
- `message` (string): The echoed message
- `client_timestamp` (int64): Original client timestamp
- `server_timestamp` (int64): Server processing timestamp
- `sequence_number` (uint32): Response sequence number

### StreamRequest
- `chunk_size` (uint32): Size of each data chunk in bytes
- `chunk_count` (uint32): Number of chunks to stream
- `delay_ms` (uint32): Delay between chunks in milliseconds (0 for no delay)

### DataChunk
- `sequence_number` (uint32): Chunk sequence number
- `data` (bytes): The actual data payload
- `checksum` (uint32): CRC32 checksum of the data
- `timestamp` (int64): Timestamp when chunk was created

### UploadResponse
- `total_bytes` (uint64): Total bytes received
- `chunk_count` (uint32): Number of chunks received
- `duration_ns` (int64): Processing duration in nanoseconds
- `checksum_valid` (bool): Whether all checksums were valid

### BatchRequest
- `items` (repeated BatchItem): Items to process
- `fail_on_error` (bool): Whether to stop on first error

### BatchItem
- `id` (string): Unique item identifier
- `operation` (string): Operation to perform
- `data` (bytes): Operation data

### BatchResponse
- `results` (repeated BatchResult): Results for each item
- `total_processed` (uint32): Total items processed
- `total_failed` (uint32): Total items that failed

### BatchResult
- `id` (string): Item identifier
- `success` (bool): Whether operation succeeded
- `error_message` (string): Error message if failed
- `result_data` (bytes): Result data if successful

## Performance Metrics

Each implementation should collect:
- Latency (p50, p95, p99)
- Throughput (requests/second, bytes/second)
- Error rate
- Connection establishment time
- Memory usage
- CPU usage

## Error Handling

All frameworks should implement consistent error codes:
- `OK` (0): Success
- `INVALID_ARGUMENT` (1): Invalid request parameters
- `DEADLINE_EXCEEDED` (2): Request timeout
- `NOT_FOUND` (3): Resource not found
- `INTERNAL` (4): Internal server error
- `UNAVAILABLE` (5): Service unavailable
