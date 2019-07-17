#!/usr/bin/env python3

import grpc
import raft_pb2
import raft_pb2_grpc
import logging
import time

def kick_off_rpc(addr):
    channel = grpc.insecure_channel(addr)
    stub = raft_pb2_grpc.ControlStub(channel)
    response = stub.Shutdown(raft_pb2.Void())
    channel.close()
    return response

def offline(addr):
    channel = grpc.insecure_channel(addr)
    stub = raft_pb2_grpc.ControlStub(channel)
    response = stub.Offline(raft_pb2.Void())
    channel.close()
    return response

def online(addr):
    channel = grpc.insecure_channel(addr)
    stub = raft_pb2_grpc.ControlStub(channel)
    response = stub.Online(raft_pb2.Void())
    channel.close()
    return response
    
def alive(addr):
    channel = grpc.insecure_channel(addr)
    stub = raft_pb2_grpc.ControlStub(channel)
    response = stub.Alive(raft_pb2.Void(), timeout=0.5)
    channel.close()
    return response

def is_alive(addr):
    try:
        alive(addr)
    except grpc.RpcError:
        return False
    return True

def wait_alive(addr):
    while True:
        if not is_alive(addr):
            logging.warning("%s not alive, retrying...", addr)

def wait_dead(addr):
    while True:
        try:
            alive(addr)
            logging.warning("%s still alive, retrying...", addr)
        except grpc.RpcError:
            break

def kick_off(addr, thread):
    if thread is None:
        logging.warning("already kicked off")
        return None
    if not thread.is_alive():
        logging.warning("already detached")
        return None
    try:
        kick_off_rpc(addr)
    except grpc.RpcError:
        logging.warning("sending quit to %s failed", addr)

    if thread.is_alive():
        thread.join()  # wait until the thread end

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
