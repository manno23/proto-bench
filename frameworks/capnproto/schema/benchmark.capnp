@0xb8a1f2c3d4e5f6a7;

# Service definition for benchmarking RPC frameworks
interface BenchmarkService {
  # Simple request/response for latency testing
  echo @0 (request: EchoRequest) -> (response: EchoResponse);

  # Server streaming for throughput testing
  streamData @1 (request: StreamRequest) -> (stream: DataChunkStream);

  # Client streaming for upload throughput testing
  uploadData @2 (stream: DataChunkStream) -> (response: UploadResponse);

  # Bidirectional streaming
  bidirectionalStream @3 (stream: DataChunkStream) -> (stream: DataChunkStream);

  # Batch processing for reliability testing
  batchProcess @4 (request: BatchRequest) -> (response: BatchResponse);
}

struct EchoRequest {
  message @0 :Text;
  timestamp @1 :Int64;
  sequenceNumber @2 :UInt32;
}

struct EchoResponse {
  message @0 :Text;
  clientTimestamp @1 :Int64;
  serverTimestamp @2 :Int64;
  sequenceNumber @3 :UInt32;
}

struct StreamRequest {
  chunkSize @0 :UInt32;
  chunkCount @1 :UInt32;
  delayMs @2 :UInt32;
}

struct DataChunk {
  sequenceNumber @0 :UInt32;
  data @1 :Data;
  checksum @2 :UInt32;
  timestamp @3 :Int64;
}

struct UploadResponse {
  totalBytes @0 :UInt64;
  chunkCount @1 :UInt32;
  durationNs @2 :Int64;
  checksumValid @3 :Bool;
}

struct BatchRequest {
  items @0 :List(BatchItem);
  failOnError @1 :Bool;
}

struct BatchItem {
  id @0 :Text;
  operation @1 :Text;
  data @2 :Data;
}

struct BatchResponse {
  results @0 :List(BatchResult);
  totalProcessed @1 :UInt32;
  totalFailed @2 :UInt32;
}

struct BatchResult {
  id @0 :Text;
  success @1 :Bool;
  errorMessage @2 :Text;
  resultData @3 :Data;
}

# Helper interface for streaming
interface DataChunkStream {
  write @0 (chunk: DataChunk) -> ();
  done @1 () -> ();
}
