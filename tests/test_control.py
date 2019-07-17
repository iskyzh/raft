from raft import spawn_client_thread, find_leaders, find_followers, append_log, request_log, kick_off
import raft
import pytest
import time

def test_restart(clusters):
    kick_off("test3")
    spawn_client_thread("test3")
