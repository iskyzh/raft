from raft import spawn_client_thread, find_leaders, find_followers, append_log, request_log, kick_off, request_all_logs, offline, online, append_logs
import raft
import pytest
import time

def test_sync_log(clusters):
    leaders = find_leaders(clusters)
    assert leaders != []
    leader = leaders[0]
    append_logs(leader, ["test1", "test2", "test3"])
    time.sleep(3)
    logs = request_all_logs(clusters)
    for (k, _) in clusters.items():
        assert logs[k].logs == ["test1", "test2", "test3"]

def test_sync_log_after_follower_kickoff(clusters):
    leaders = find_leaders(clusters)
    assert leaders != []
    leader = leaders[0]
    append_logs(leader, ["test1", "test2", "test3"])
    time.sleep(3)
    followers = find_followers(clusters)
    kicked_off_follower = followers[0]
    kick_off(kicked_off_follower)
    spawn_client_thread(kicked_off_follower)
    time.sleep(3)
    logs = request_all_logs(clusters)
    assert logs[leader].logs == ["test1", "test2", "test3"]
    for (k, _) in clusters.items():
        assert logs[k].logs == ["test1", "test2", "test3"]

def test_sync_log_after_leader_kickoff(clusters):
    leaders = find_leaders(clusters)
    assert len(leaders) == 1
    leader = leaders[0]
    append_logs(leader, ["test1", "test2", "test3"])
    time.sleep(3)
    logs = request_all_logs(clusters)
    for (k, _) in clusters.items():
        assert logs[k].logs == ["test1", "test2", "test3"]
    kick_off(leader)
    spawn_client_thread(leader)
    time.sleep(3)
    leaders = find_leaders(clusters)
    assert len(leaders) == 1
    leader = leaders[0]
    logs = request_all_logs(clusters)
    assert logs[leader].logs == ["test1", "test2", "test3"]
    for (k, _) in clusters.items():
        assert logs[k].logs == ["test1", "test2", "test3"]

def test_purge_log_after_leader_offline(clusters):
    leaders = find_leaders(clusters)
    assert len(leaders) == 1
    leader = leaders[0]
    append_logs(leader, ["test1", "test2", "test3"])
    time.sleep(3)
    offline(leader)
    append_logs(leader, ["test4", "test5", "test6"])
    time.sleep(3)
    online(leader)
    time.sleep(3)
    leaders = find_leaders(clusters)
    assert len(leaders) == 1
    leader = leaders[0]
    logs = request_all_logs(clusters)
    assert logs[leader].logs == ["test1", "test2", "test3"]
    for (k, _) in clusters.items():
        assert logs[k].logs == ["test1", "test2", "test3"]
