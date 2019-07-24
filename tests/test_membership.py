from raft import spawn_client_thread, find_leaders, find_followers, append_log, request_log, kick_off, request_all_logs
import raft
import pytest
import time
import json

def test_extra_nodes(clusters):
    spawn_client_thread("test6")
    time.sleep(1)
    kick_off("test6")

def new_config():
    config = { "type": "membership_change", "clusters": {} }
    for (k, v) in raft.clusters.items():
        config["clusters"][k] = "127.0.0.1:%s" % v
    return json.dumps(config)

def test_change_ownership(clusters):
    leaders = find_leaders(clusters)
    assert len(leaders) == 1
    leader = leaders[0]
    
    spawn_client_thread("test6")
    spawn_client_thread("test7")
    spawn_client_thread("test8")
    spawn_client_thread("test9")
    time.sleep(1)

    append_log(leader, new_config())
    time.sleep(10)
    
    leaders = find_leaders(clusters)
    assert len(leaders) == 1
    append_log(leader, "test")

    time.sleep(1)

    d_logs = request_all_logs(raft.default_clusters)
    e_logs = request_all_logs(raft.extra_clusters)
    for k in raft.default_clusters:
        assert d_logs[k].logs[1] == "test"
        if k != leader:
            assert d_logs[k].role == "follower"
    for k in raft.extra_clusters:
        assert e_logs[k].logs[1] == "test"
        if k != leader:
            assert e_logs[k].role == "follower"