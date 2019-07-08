#!/bin/bash

GRPC_PLUGIN=$(which grpc_cpp_plugin)

~/Work/vcpkg/installed/x64-osx/tools/protobuf/protoc-3.8.0.0 --grpc_out . --cpp_out . -I . --plugin=protoc-gen-grpc=$GRPC_PLUGIN raft.proto
