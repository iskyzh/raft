from raft import spawn_client_thread, find_leaders, find_followers, append_log, request_log, kick_off, request_all_logs
import raft
import pytest
import time

def test_select_leader(clusters):
    assert find_leaders(clusters) != []

def count_role(logs, role):
    cnt = 0
    for (_, log) in logs.items():
        if log.role == role:
            cnt = cnt + 1
    return cnt

def test_new_leader_after_kick_off(clusters):
    leaders = find_leaders(clusters)
    assert len(leaders) == 1
    leader = leaders[0]
    kick_off(leader)
    time.sleep(3)
    logs = request_all_logs(clusters)

    leader_cnt = count_role(logs, "leader")
    candidate_cnt = count_role(logs, "candidate")
    follower_cnt = count_role(logs, "follower")

    assert candidate_cnt == 0
    assert follower_cnt == 3
    assert leader_cnt == 1

def test_no_leader_after_kick_off(clusters):
    leaders = find_leaders(clusters)
    followers = find_followers(clusters)
    kick_off(leaders[0])
    kick_off(followers[0])
    kick_off(followers[1])
    time.sleep(3)
    logs = request_all_logs(clusters)

    leader_cnt = count_role(logs, "leader")
    
    assert leader_cnt == 0
