#!/usr/bin/env python3

import grpc
import raft_pb2
import raft_pb2_grpc


def kick_off(addr, thread):
    channel = grpc.insecure_channel(addr)
    stub = raft_pb2_grpc.ControlStub(channel)
    response = stub.Shutdown(raft_pb2.Void())
    if thread.is_alive():
        thread.join()  # wait until the thread end
    channel.close()
    return response


def request_log(addr):
    channel = grpc.insecure_channel(addr)
    stub = raft_pb2_grpc.ControlStub(channel)
    response = stub.RequestLog(raft_pb2.Void())
    channel.close()
    return response


def append_log(addr, log):
    channel = grpc.insecure_channel(addr)
    stub = raft_pb2_grpc.ControlStub(channel)
    req = raft_pb2.AppendLogRequest()
    req.log = log
    response = stub.AppendLog(req)
    channel.close()
    return response
