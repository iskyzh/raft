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
    subprocess.run([executable, config_path])
    logging.debug("%s detached" % instance_name)

def spawn_client_thread(id):
    config_path = os.path.abspath("%s_config.toml" % id)
    logging.debug('running %s with config %s' % (id, config_path))
    thread = threading.Thread(target=bootstrap_client, args=(id, config_path,))
    thread.start()
    raft_threads[id] = thread

def kick_off(id):
    utils.kick_off("127.0.0.1:%s" % clusters[id], raft_threads[id])

class TestRaftSetupAndTeardown(unittest.TestCase):
    def setUp(self):
        for (k, v) in clusters.items():
            spawn_client_thread(k)
        time.sleep(1)

    def tearDown(self):
        for (k, v) in clusters.items():
            kick_off(k)
        time.sleep(1)
        
    def test_restart(self):
        kick_off("test3")
        spawn_client_thread("test3")
        time.sleep(3)

if __name__ == '__main__':
    generate_config()

    unittest.main()
    