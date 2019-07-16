#!/usr/bin/env python3

import logging
import unittest
import os
import subprocess
import toml
import signal
import time
import threading

logging.basicConfig(level=logging.DEBUG)

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
    logging.info("%s started" % instance_name)
    subprocess.run([executable, config_path, "--verbose"])
    logging.info("%s detached" % instance_name)


for (k, v) in clusters.items():
    config_path = os.path.abspath("%s_config.toml" % k)
    logging.info('running %s with config %s' % (k, config_path))
    thread = threading.Thread(target=bootstrap_client, args=(k, config_path,))
    thread.start()
    raft_threads[k] = thread

try:
    for (k, thread) in raft_threads.items():
        thread.join()
except KeyboardInterrupt:
    pass

class TestRaftSetupAndTeardown(unittest.TestCase):

    def test_upper(self):
        self.assertEqual('foo'.upper(), 'FOO')


if __name__ == '__main__':
    unittest.main()
    