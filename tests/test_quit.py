#!/usr/bin/env python3

import grpc
import raft_pb2
import raft_pb2_grpc

def main():
  channel = grpc.insecure_channel('localhost:23337')
  stub = raft_pb2_grpc.ControlStub(channel)
  response = stub.Shutdown(raft_pb2.Void())
  print(response)

if __name__ == "__main__":
    main()
