#!/usr/bin/env python3

import grpc
import raft_pb2
import raft_pb2_grpc

def kick_off(addr, thread):
  channel = grpc.insecure_channel(addr)
  stub = raft_pb2_grpc.ControlStub(channel)
  response = stub.Shutdown(raft_pb2.Void())
  if thread.is_alive():
    thread.join() # wait until the thread end
  channel.close()