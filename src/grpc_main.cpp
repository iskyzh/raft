//
// Created by Alex Chi on 2019-07-09.
//

#include "../rpc/grpc_client.hpp"
#include "../core/Instance.h"
#include "../utils/utils.h"

string generate_message(int id) {
    char s[100];
    sprintf(s, "test%d", id);
    return s;
}

int start_event_loop(shared_ptr<Instance> inst, shared_ptr<RaftRPCClient> client) {
    BOOST_LOG_TRIVIAL(info) << "starting event loop...";
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
            // inst->logs.dump();

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

int main() {
    map<string, string> route = {
            {"test1", "127.0.0.1:23333"},
            {"test2", "127.0.0.1:23334"},
            {"test3", "127.0.0.1:23335"},
            {"test4", "127.0.0.1:23336"},
            {"test5", "127.0.0.1:23337"}
    };
    vector<string> clusters;
    for (auto &&kv : route) clusters.push_back(kv.first);
    map<string, shared_ptr<RaftRPCClient>> clients;
    map<string, shared_ptr<Instance>> instances;
    vector<thread> threads;
    for (auto &&kv : route) {
        auto &&client_name = kv.first;
        auto &&client_ip = kv.second;
        auto client = make_shared<RaftRPCClient>(client_ip, route);
        clients[client_name] = client;
        auto instance = make_shared<Instance>(client_name, client);
        instance->set_clusters(clusters);
        instances[client_name] = instance;
        thread server_thread([client]() { client->run_server(); });
        threads.push_back(std::move(server_thread));
    }
    for (auto &&instance : instances) {
        auto client = clients[instance.first];
        thread event_thread([instance, client] { start_event_loop(instance.second, client); });
        threads.push_back(std::move(event_thread));
    }
    for (auto &t : threads) {
        if (t.joinable())
            t.join();
    }
    return 0;
}
