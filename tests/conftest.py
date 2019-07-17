from raft import spawn_client_thread, find_leaders, find_followers, append_log, request_log, kick_off, request_all_logs
import raft
import pytest
import time

@pytest.fixture
def clusters(): 
    for (id, _) in raft.clusters.items():
        spawn_client_thread(id)
    time.sleep(1)
    yield raft.clusters
    for (id, _) in raft.clusters.items():
        kick_off(id)
