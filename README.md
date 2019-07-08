# Raft

[![Build Status](https://travis-ci.com/skyzh/raft.svg?token=szB6fz2m5vb2KyfAiZ3B&branch=master)](https://travis-ci.com/skyzh/raft)

The Raft Consensus Algorithm. This is an implementation in C++.

## Usage

1. Use vcpkg or other tools to install dependencies including `grpc`, `protobuf`, `boost`, `gtest`.
2. Generate protobuf header with protos/generate.sh
3. Build and run

## Design

Some changes are made on Raft RPC described in original paper.

This implementation only use gRPC as a communication tool, which means that we could not correspond a response to a request.

For `AppendEntries` RPC, since prevLogTerm in request should be known, I add `lastAgreedLogIndex` field to record that information.
