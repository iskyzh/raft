//
// Created by Alex Chi on 2019-07-09.
//

#include "../rpc/grpc_client.hpp"
#include "../core/Instance.h"
#include "../utils/utils.h"

int start_event_loop(shared_ptr<Instance> inst) {
    BOOST_LOG_TRIVIAL(info) << "starting event loop...";
    inst->start();
    auto lst_updated = get_tick();
    auto lst_append_entry = get_tick();
    while (true) {
        if (get_tick() - lst_updated >= 30) {
            lst_updated = get_tick();
            inst->update();
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
        thread event_thread([instance] { start_event_loop(instance.second); });
        threads.push_back(std::move(event_thread));
    }
    for (auto &t : threads) {
        if (t.joinable())
            t.join();
    }
    return 0;
}
