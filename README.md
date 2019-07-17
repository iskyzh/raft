# Raft

[![Build Status](https://travis-ci.com/skyzh/raft.svg?branch=master)](https://travis-ci.com/skyzh/raft)

The Raft Consensus Algorithm implemented in C++. Refer to [https://raft.github.io/](https://raft.github.io/) for original paper.

## Usage

1. Use vcpkg or other tools to install dependencies including `grpc`, `protobuf`, `boost`, `gtest`.
2. Generate protobuf header with protos/generate.sh
3. Build and run
4. Use python to run system test

```bash
export RAFT_EXECUTABLE=$(pwd)/RaftMain
pytest tests/
```

## Design

Some changes were made on Raft RPC described in original paper. This implementation only uses gRPC as a communication tool, which means that we could not correspond a response to a request. Therefore, for `AppendEntries` RPC, prevLogTerm in request should be known when sending AppendEntries reply. Therefore a `lastAgreedLogIndex` field was added.

Personally I would like to break down Raft protocol to many small parts as it is more convenient to test.

`core/` contains core Raft algorithm.
`rpc/` contains one RPC implementation with gRPC.

In `src/`, `mock_main.cpp` can be used to mock RPC network with events. 
You may adjust `drop_rate` and `delay` to mock an unstable network.
You may add 

`grpc_main.cpp` uses gRPC for communication between clients.

`service_main.cpp` is a real raft client.
