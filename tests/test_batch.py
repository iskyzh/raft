from raft import spawn_client_thread, find_leaders, find_followers, append_log, request_log, kick_off, request_all_logs
import raft
import pytest
import time
import binascii
import os
import logging

def generate_random_logs():
    return list(map(lambda x: binascii.b2a_hex(os.urandom(15)).decode(), range(1000)))

def test_1k_logs_sync(clusters):
    rand_logs = generate_random_logs()
    leaders = find_leaders(clusters)
    assert len(leaders) == 1
    leader = leaders[0]
    for i, log in enumerate(rand_logs):
        if i % 100 == 0:
            logging.info("processing (%d/%d) %s" % (i + 1, len(rand_logs), rand_logs[i]))
        append_log(leader, log)
    time.sleep(30)
    logs = request_all_logs(clusters)
    for (k, _) in clusters.items():
        assert logs[k].logs == rand_logs
