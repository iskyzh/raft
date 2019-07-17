#!/bin/bash

GRPC_CPP_PLUGIN=$VCPKG_ROOT/installed/$VCPKG_DEFAULT_TRIPLET/tools/grpc/grpc_cpp_plugin
GRPC_PY_PLUGIN=$VCPKG_ROOT/installed/$VCPKG_DEFAULT_TRIPLET/tools/grpc/grpc_python_plugin
PROTOC_PATH=$VCPKG_ROOT/installed/$VCPKG_DEFAULT_TRIPLET/tools/protobuf/protoc

$PROTOC_PATH --grpc_out=. --cpp_out=. -I . --plugin=protoc-gen-grpc=$GRPC_CPP_PLUGIN  raft.proto
$PROTOC_PATH --grpc_out=../tests --python_out=../tests -I . --plugin=protoc-gen-grpc=$GRPC_PY_PLUGIN raft.proto
