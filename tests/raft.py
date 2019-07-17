import logging
import unittest
import os
import subprocess
import toml
import signal
import time
import threading
import utils
import tempfile

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

_config_dir = tempfile.TemporaryDirectory()
config_dir = _config_dir.name

raft_threads = {}

for (k, v) in clusters.items():
    config["clusters"].append({"name": k, "addr": "127.0.0.1:%s" % v})

def get_addr(id):
    return "127.0.0.1:%s" % clusters[id]

def config_file(id):
    return "%s/%s_config.toml" % (config_dir, id)
    
def generate_config():
    for (k, v) in clusters.items():
        config["server"]["name"] = k
        config["server"]["addr"] = "127.0.0.1:%s" % v
        dump_config = toml.dumps(config)
        f = open(config_file(k), "w")
        f.write(dump_config)
        f.close()
        logging.debug('config for %s generated' % k)

    logging.info('config generation complete')

def bootstrap_client(instance_name, config_path):
    logging.debug("%s started" % instance_name)
    args = [executable, config_path]
    if verbose:
        args = [executable, config_path, "--verbose"]
    subprocess.run(args)
    logging.debug("%s detached" % instance_name)

def spawn_client_thread(id):
    config_path = config_file(id)
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

def find_leaders(clusters):
    leaders = []
    for (k, v) in clusters.items():
        log = request_log(k)
        if log.role == "leader":
            leaders.append(k)
    return leaders

def find_followers(clusters):
    followers = []
    for (k, v) in clusters.items():
        log = request_log(k)
        if log.role == "follower":
            followers.append(k)
    return followers

generate_config()
