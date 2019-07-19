# Raft

[![Build Status](https://travis-ci.com/skyzh/raft.svg?branch=master)](https://travis-ci.com/skyzh/raft)

The Raft Consensus Algorithm implemented in C++. Refer to [https://raft.github.io/](https://raft.github.io/) for original paper.

## Usage

1. Use vcpkg or other tools to install build dependencies.
Use pip to install test dependencies.
```bash
vcpkg install gtest grpc protobuf cpptoml
pip3 install -r tests/requirements.txt
```
2. Generate protobuf header
```bash
export VCPKG_ROOT="$HOME/vcpkg"
export VCPKG_DEFAULT_TRIPLET="x64-osx"
cd protos && ./generate.sh
```
3. Build and run
4. Use python to run system test
```bash
export RAFT_EXECUTABLE=$(pwd)/RaftMain
pytest tests/
```

## Design

Some changes were made on Raft RPC described in original paper. This
implementation only uses gRPC for node communication, but I split request 
and reply to 2 RPCs. Therefore ww could not correspond a response to a request.
For `AppendEntries` RPC, prevLogTerm in request should be known when 
sending AppendEntries reply. Therefore a `lastAgreedLogIndex` field was added.
For other reply RPCs, we should add sender field.

Personally I would like to break down Raft protocol to many small 
parts as it is more convenient to test.

`core/` contains core Raft algorithm.

`rpc/` contains one RPC implementation with gRPC.

In `src/`, `mock_main.cpp` can be used to mock a Raft cluster with RPC 'events',
which means that there're no RPC requests and all RPC are simulated with
events and callbacks. You may adjust `drop_rate` and `delay` to mock an 
unstable network. You may add events to simulate events in network. 
Currently the mock main will kick off leader and restore it to test log consistency. 
It will generate `RaftMockMain` executable.

`grpc_main.cpp` uses gRPC for communication between clients. It also helps set
up a Raft cluster. It will generate `RaftMockRPC` executable.

`service_main.cpp` is a real Raft client with server control for system testing. 
It corresponds to `RaftService` executable.

System tests in `tests/` are written in Python. It will automatically run Raft service
executable, build a 5-node cluster and test it with different conditions.

# Todo

- [ ] Async log read and write
- [ ] Test section 5.4
- [ ] Cluster membership changes
