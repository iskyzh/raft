//
// Created by Alex Chi on 2019-07-09.
//

#include <cpptoml.h>

#include "../rpc/grpc_client.hpp"
#include "../core/Instance.h"
#include "../utils/utils.h"

string generate_message(int id) {
    char s[100];
    sprintf(s, "test%d", id);
    return s;
}

int start_event_loop(shared_ptr<Instance> inst, shared_ptr<RaftRPCClient> client) {
    BOOST_LOG_TRIVIAL(info) << inst->id << " starting event loop...";
    inst->start();
    auto lst_updated = get_tick();
    auto lst_append_entry = get_tick();
    while (true) {
        if (get_tick() - lst_updated >= 30) {
            lst_updated = get_tick();
            inst->update();
        }
        if (get_tick() - lst_append_entry >= 1000) {
            lst_append_entry = get_tick();
            BOOST_LOG_TRIVIAL(info) << inst->id << " " << inst->get_role_string() << " size: "
                                    << inst->logs.logs.size();

            if (inst->role == LEADER) {
                static unsigned message_id = 0;
                string msg = generate_message(message_id++);
                inst->append_entry(msg);
                BOOST_LOG_TRIVIAL(info) << inst->id << " has requested append entry";
            }
        }
        RaftRPCClient::RPCMessage *rpc;
        while (client->q.pop(rpc)) {
            inst->on_rpc(rpc->from, rpc->message);
            delete rpc;
        }
    }
    return 0;
}

int main(int argc, char **argv) {
    const char *config_path = "config.toml";
    if (argc == 2) { config_path = argv[1]; }

    BOOST_LOG_TRIVIAL(trace) << "loading config from " << config_path;

    auto config = cpptoml::parse_file(config_path);
    auto tclusters = config->get_table_array("clusters");

    map<string, string> route;
    vector<string> clusters;

    for (const auto &cluster : *tclusters) {
        auto name = cluster->get_as<string>("name");
        auto addr = cluster->get_as<string>("addr");
        if (!name || !addr) continue;
        route[*name] = *addr;
        clusters.push_back(*name);
        BOOST_LOG_TRIVIAL(trace) << "add cluster " << *name << " at " << *addr;
    }

    auto server_name = config->get_qualified_as<string>("server.name");
    auto server_addr = config->get_qualified_as<string>("server.addr");

    if (!server_name || !server_addr) {
        BOOST_LOG_TRIVIAL(error) << "please specify server in config";
        return 0;
    }

    BOOST_LOG_TRIVIAL(trace) << "server " << *server_name << " at " << *server_addr;

    BOOST_LOG_TRIVIAL(info) << "config loaded from " << config_path;

    auto client = make_shared<RaftRPCClient>(*server_name, *server_addr, route);
    auto instance = make_shared<Instance>(*server_name, client);

    thread server_thread([client]() { client->run_server(); });
    thread event_thread([instance, client] { start_event_loop(instance, client); });

    server_thread.join();
    event_thread.join();

    return 0;
}
