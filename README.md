# Raft

[![Build Status](https://travis-ci.com/skyzh/raft.svg?branch=master)](https://travis-ci.com/skyzh/raft)

The Raft Consensus Algorithm implemented in C++. Refer to [https://raft.github.io/](https://raft.github.io/) for original paper.

## Usage

1. Use vcpkg or other tools to install dependencies including `grpc`, `protobuf`, `boost`, `gtest`.
2. Generate protobuf header with protos/generate.sh
3. Build and run

## Design

Some changes were made on Raft RPC described in original paper. This implementation only uses gRPC as a communication tool, which means that we could not correspond a response to a request. Therefore, for `AppendEntries` RPC, prevLogTerm in request should be known when sending AppendEntries reply. Therefore a `lastAgreedLogIndex` field was added.

Personally I would like to break down Raft protocol to many small parts as it is more convenient to test.

This branch now only contains core Raft algorithm. All RPCs are simulated with callbacks. Later I'll add gRPC and make system tests with real world network and application.
