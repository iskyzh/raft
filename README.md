# Raft

[![Build Status](https://travis-ci.com/skyzh/raft.svg?branch=master)](https://travis-ci.com/skyzh/raft)

Raft Consensus Algorithm implemented in C++. Refer to [https://raft.github.io/](https://raft.github.io/) for original paper.

## Usage

1. Use vcpkg or other tools to install build dependencies (boost, gtest, grpc, protobuf, cpptoml).
Use pip to install test dependencies.
```bash
apt install libboost-all-dev
vcpkg install gtest grpc protobuf cpptoml
pip3 install -r tests/requirements.txt
```
2. Generate protobuf header
```bash
export VCPKG_ROOT="$HOME/vcpkg"
export VCPKG_DEFAULT_TRIPLET="x64-osx"
cd protos && ./generate.sh
```
3. Build and run unit tests
```bash
cmake -H. -Bbuild -DCMAKE_BUILD_TYPE=RELEASE -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
cd build && cmake --build .
./RaftTest
export RAFT_EXECUTABLE=$(pwd)/RaftService
```
4. Use python to run system tests
```bash
pytest tests/
```

On macOS, there'll be some problems with launching subprocess in Python. Testing script
will retry launching until server is reachable.

## Design

Some changes were made on Raft RPC described in original paper. This
implementation uses gRPC for node communication, but I make request 
and reply into 2 separate RPCs. Therefore it's not possible to correspond
a response to a request. For `AppendEntries` RPC, prevLogTerm in request
should be known when sending AppendEntries reply. Therefore a 
`lastAgreedLogIndex` field was added. For other reply RPCs, sender field is added.

Personally I would like to break down Raft protocol to many small 
parts as it is more convenient to test.

`core/` contains core Raft algorithm. The core algorithm (`Instance.cpp`) contains
only 200+ lines of code.

`rpc/` contains one RPC implementation with gRPC. Here I choose single-thread asynchronized
implementation. All RPCs requests are pushed into a lock-free queue, and then are processed
in the event-loop thread. This choice leads to the split of RPC request and response message
described above. Therefore, thread lock usage is eliminated.

In `src/`, `mock_main.cpp` can be used to mock a Raft cluster with 'events',
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
