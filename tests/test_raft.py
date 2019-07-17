from raft import spawn_client_thread, find_leaders, find_followers, append_log, request_log, kick_off
import raft
import pytest
import time

def request_all_logs(clusters):
    logs = {}
    for (k, v) in clusters.items():
        logs[k] = request_log(k)
    return logs

@pytest.fixture
def clusters():
    for (k, v) in raft.clusters.items():
        spawn_client_thread(k)
    time.sleep(1)
    yield raft.clusters
    for (k, v) in raft.clusters.items():
        kick_off(k)
    time.sleep(1)

def test_select_leader(clusters):
    assert find_leaders(clusters) != []

def test_sync_log(clusters):
    leader = find_leaders(clusters)
    assert leader != []
    leader = leader[0]
    append_log(leader, "test1")
    append_log(leader, "test2")
    append_log(leader, "test3")
    time.sleep(1)
    logs = request_all_logs(clusters)
    for (k, v) in clusters.items():
        assert logs[k].logs == ["test1", "test2", "test3"]

def test_sync_log_after_follower_kickoff(clusters):
    leader = find_leaders(clusters)
    assert leader != []
    leader = leader[0]
    append_log(leader, "test1")
    append_log(leader, "test2")
    append_log(leader, "test3")
    followers = find_followers(clusters)
    kicked_off_follower = followers[0]
    kick_off(kicked_off_follower)
    spawn_client_thread(kicked_off_follower)
    time.sleep(3)
    logs = request_all_logs(clusters)
    assert logs[leader].logs == ["test1", "test2", "test3"]
    for (k, v) in clusters.items():
        assert logs[k].logs == ["test1", "test2", "test3"]
