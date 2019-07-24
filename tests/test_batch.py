from raft import spawn_client_thread, find_leaders, find_followers, append_log, request_log, kick_off, request_all_logs, append_logs
import raft
import pytest
import time
import binascii
import os
import logging


def generate_random_logs(log_size):
    return list(map(lambda x: binascii.b2a_hex(os.urandom(15)).decode(), range(log_size)))


def test_1k_logs_sync(clusters):
    rand_logs = generate_random_logs(1000)
    leaders = find_leaders(clusters)
    assert len(leaders) == 1
    leader = leaders[0]
    append_logs(leader, rand_logs)
    time.sleep(10)
    logs = request_all_logs(clusters)
    for k in clusters:
        assert logs[k].logs == rand_logs


def test_10k_logs_sync(clusters):
    rand_logs = generate_random_logs(10000)
    leaders = find_leaders(clusters)
    assert len(leaders) == 1
    leader = leaders[0]
    for i in range(10):
        append_logs(leader, rand_logs[i*1000:i*1000+1000])
        time.sleep(1)
    time.sleep(30)
    logs = request_all_logs(clusters)
    for k in clusters:
        assert logs[k].logs == rand_logs
