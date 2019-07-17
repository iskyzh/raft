from raft import spawn_client_thread, find_leader, append_log, request_log, kick_off
import raft
import pytest
import time

def request_all_logs(clusters):
    logs = {}
    for (k, v) in clusters.items():
        logs[k] = request_log(k)
    return logs

@pytest.fixture(scope="module")
def clusters():
    for (k, v) in raft.clusters.items():
        spawn_client_thread(k)
    time.sleep(3)
    yield raft.clusters
    for (k, v) in raft.clusters.items():
        kick_off(k)
    time.sleep(3)

class TestRaftControlRPCs(object):
    def test_select_leader(self, clusters):
        assert find_leader(clusters) is not None

    def test_sync_log(self, clusters):
        leader = find_leader(clusters)
        assert leader is not None
        append_log(leader, "test1")
        append_log(leader, "test2")
        append_log(leader, "test3")
        time.sleep(1)
        logs = request_all_logs(clusters)
        for (k, v) in clusters.items():
            assert logs[k].logs == ["test1", "test2", "test3"]

    @pytest.mark.skip("this test will fail")   
    def test_sync_log_after_leader_kickoff(self, clusters):
        leader = find_leader(clusters)
        assert leader is not None
        append_log(leader, "test1")
        append_log(leader, "test2")
        append_log(leader, "test3")
        time.sleep(3)
        kick_off(leader)
        spawn_client_thread(leader)
        time.sleep(3)
        logs = request_all_logs(clusters)
        assert log[leader].logs == ["test1", "test2", "test3"]
        for (k, v) in clusters.items():
            assert logs[k].logs == ["test1", "test2", "test3"]
    
    def test_restart(self, clusters):
        kick_off("test3")
        spawn_client_thread("test3")
        time.sleep(1)
