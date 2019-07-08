#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <boost/log/trivial.hpp>

#include "MockRPCService.h"
#include "Instance.h"
#include "utils.h"

using std::vector;
using std::unique_ptr;
using std::make_unique;
using std::string;

string get_client_name(int id) {
    char c[100];
    sprintf(c, "test%d", id);
    return c;
}

vector<unique_ptr<Instance>> build_cluster(MockRPCService &service, int size) {
    vector<unique_ptr<Instance>> instances;
    vector<string> clusters;
    for (int i = 0; i < size; i++) {
        const auto instance_name = get_client_name(i);
        clusters.push_back(instance_name);
        instances.push_back(make_unique<Instance>(instance_name, service.get_client(instance_name)));
    }
    for (auto &&instance : instances) {
        instance->set_clusters(clusters);
    }
    return instances;
}

int start_event_loop(MockRPCService &service, vector<unique_ptr<Instance>> &insts) {
    service.set_callback([&insts](const string &from, const string &to, shared_ptr<Message> message) {
        for (auto &&inst : insts) {
            if (inst->id == to) {
                inst->on_rpc(from, message);
                return;
            }
        }
        BOOST_LOG_TRIVIAL(error) << "rpc destination unreachable";
    });
    BOOST_LOG_TRIVIAL(info) << "starting event loop...";
    auto lst_updated = get_tick();
    while (true) {
        if (get_tick() - lst_updated >= 10) {
            lst_updated = get_tick();
            for (auto &&inst : insts) {
                inst->update();
            }
        }
    }
}

int main() {
    MockRPCService service;
    auto insts = build_cluster(service, 5);
    for (auto &&inst : insts) inst->start();
    BOOST_LOG_TRIVIAL(info) << "cluster generation complete";
    start_event_loop(service, insts);
    return 0;
}
