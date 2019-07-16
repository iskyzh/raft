#!/bin/bash

GRPC_CPP_PLUGIN=$(which grpc_cpp_plugin)
GRPC_PY_PLUGIN=$(which grpc_python_plugin)

$VCPKGROOT/installed/x64-osx/tools/protobuf/protoc-3.8.0.0 --grpc_out=. --cpp_out=. -I . --plugin=protoc-gen-grpc=$GRPC_CPP_PLUGIN  raft.proto
$VCPKGROOT/installed/x64-osx/tools/protobuf/protoc-3.8.0.0 --grpc_out=../tests --python_out=../tests -I . --plugin=protoc-gen-grpc=$GRPC_PY_PLUGIN raft.proto
