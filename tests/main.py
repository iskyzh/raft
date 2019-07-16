#!/usr/bin/env python3

import logging
import unittest
import os
import subprocess
import toml
import signal
import time
import threading
import utils

verbose = False
if os.environ.get("VERBOSE"):
    verbose = True
    
logging.basicConfig(level=os.environ.get("LOGLEVEL", "INFO"))
if not 'RAFT_EXECUTABLE' in os.environ:
    logging.error('raft service executable not found')
    exit()

executable = os.path.abspath(os.environ['RAFT_EXECUTABLE'])

logging.info('using raft service executable at \'%s\'' % executable)

logging.info('generating config...')

clusters = {"test1": "23333",
            "test2": "23334",
            "test3": "23335",
            "test4": "23336",
            "test5": "23337" }

config = { "server": {}, "clusters": [] }

for (k, v) in clusters.items():
    config["clusters"].append({"name": k, "addr": "127.0.0.1:%s" % v})

def get_addr(id):
    return "127.0.0.1:%s" % clusters[id]

def generate_config():
    for (k, v) in clusters.items():
        config["server"]["name"] = k
        config["server"]["addr"] = "127.0.0.1:%s" % v
        dump_config = toml.dumps(config)
        f = open("%s_config.toml" % k, "w")
        f.write(dump_config)
        f.close()
        logging.debug('config for %s generated' % k)

    logging.info('config generation complete')

raft_threads = {}

def bootstrap_client(instance_name, config_path):
    logging.debug("%s started" % instance_name)
    args = [executable, config_path]
    if verbose:
        args = [executable, config_path, "--verbose"]
    subprocess.run(args)
    logging.debug("%s detached" % instance_name)

def spawn_client_thread(id):
    config_path = os.path.abspath("%s_config.toml" % id)
    logging.debug('running %s with config %s' % (id, config_path))
    thread = threading.Thread(target=bootstrap_client, args=(id, config_path,))
    thread.start()
    raft_threads[id] = thread

def kick_off(id):
    utils.kick_off(get_addr(id), raft_threads[id])

def request_log(id):
    return utils.request_log(get_addr(id))

def append_log(id, log):
    return utils.append_log(get_addr(id), log)

def find_leader():
    for (k, v) in clusters.items():
        log = request_log(k)
        if log.role == "leader":
            return k
    return None

class TestRaftControlRPCs(unittest.TestCase):
    def setUp(self):
        for (k, v) in clusters.items():
            spawn_client_thread(k)
        time.sleep(3)

    def tearDown(self):
        for (k, v) in clusters.items():
            kick_off(k)
        time.sleep(3)
        
    def test_select_leader(self):
        self.assertIsNotNone(find_leader())

    def test_sync_log(self):
        leader = find_leader()
        self.assertIsNotNone(leader)
        append_log(leader, "test1")
        append_log(leader, "test2")
        append_log(leader, "test3")
        time.sleep(1)
        log1 = request_log("test1").logs
        log2 = request_log("test2").logs
        log3 = request_log("test3").logs
        log4 = request_log("test4").logs
        log5 = request_log("test5").logs
        self.assertEqual(log1, ["test1", "test2", "test3"])
        self.assertEqual(log2, ["test1", "test2", "test3"])
        self.assertEqual(log3, ["test1", "test2", "test3"])
        self.assertEqual(log4, ["test1", "test2", "test3"])
        self.assertEqual(log5, ["test1", "test2", "test3"])

    @unittest.skip("this test will fail")   
    def test_sync_log_after_leader_kickoff(self):
        leader = find_leader()
        self.assertIsNotNone(leader)
        append_log(leader, "test1")
        append_log(leader, "test2")
        append_log(leader, "test3")
        time.sleep(3)
        kick_off(leader)
        spawn_client_thread(leader)
        time.sleep(3)
        log1 = request_log("test1").logs
        log2 = request_log("test2").logs
        log3 = request_log("test3").logs
        log4 = request_log("test4").logs
        log5 = request_log("test5").logs
        self.assertEqual(leader, ["test1", "test2", "test3"])
        self.assertEqual(log1, ["test1", "test2", "test3"])
        self.assertEqual(log2, ["test1", "test2", "test3"])
        self.assertEqual(log3, ["test1", "test2", "test3"])
        self.assertEqual(log4, ["test1", "test2", "test3"])
        self.assertEqual(log5, ["test1", "test2", "test3"])
        
    def test_restart(self):
        kick_off("test3")
        spawn_client_thread("test3")
        time.sleep(1)


if __name__ == '__main__':
    generate_config()

    unittest.main()
    