from raft import spawn_client_thread, find_leaders, find_followers, append_log, request_log, kick_off, request_all_logs
import raft
import pytest
import time

@pytest.fixture
def clusters(): 
    for id in raft.default_clusters:
        spawn_client_thread(id)
    time.sleep(1)
    yield raft.default_clusters
    for (id, _) in raft.clusters.items():
        kick_off(id)
