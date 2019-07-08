#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <queue>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include "MockRPCService.h"
#include "Instance.h"
#include "utils.h"

using std::vector;
using std::priority_queue;
using std::unique_ptr;
using std::make_shared;
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

void send_message(vector<unique_ptr<Instance>> &insts,
                  const string &from, const string &to, shared_ptr<Message> message) {
    log_message(from, to, message);
    for (auto &&inst : insts) {
        if (inst->id == to) {
            inst->on_rpc(from, message);
            return;
        }
    }
    BOOST_LOG_TRIVIAL(error) << "rpc destination unreachable: " << to;
}

struct RPCDeferMessage {
    TICK should_be_sent_at;
    string from, to;
    shared_ptr<Message> message;

    RPCDeferMessage(TICK should_be_sent_at, const string &from, const string &to, shared_ptr<Message> message)
            : should_be_sent_at(should_be_sent_at), from(from), to(to) {
        this->message = message;
    }

    friend bool operator<(const RPCDeferMessage &a, const RPCDeferMessage &b) {
        return a.should_be_sent_at > b.should_be_sent_at;
    }
};

int start_event_loop(MockRPCService &service, vector<unique_ptr<Instance>> &insts) {
    priority_queue<RPCDeferMessage> mq;
    service.set_callback([&insts, &mq](const string &from, const string &to, shared_ptr<Message> message) {
        const double drop_rate = 0.3;
        const int delay = 200;
        if (1.0 * std::rand() / RAND_MAX <= drop_rate) {
            BOOST_LOG_TRIVIAL(trace) << "rpc message dropped";
            return;
        }
        mq.push(RPCDeferMessage(get_tick() + std::rand() % delay, from, to, message));
    });
    BOOST_LOG_TRIVIAL(info) << "starting event loop...";
    for (auto &&inst : insts) inst->start();
    auto lst_updated = get_tick();
    while (true) {
        if (get_tick() - lst_updated >= 5) {
            lst_updated = get_tick();
            for (auto &&inst : insts) {
                inst->update();
            }
        }
        while (!mq.empty() && mq.top().should_be_sent_at < get_tick()) {
            auto rpc = mq.top();
            mq.pop();
            send_message(insts, rpc.from, rpc.to, rpc.message);
        }
    }
    return 0;
}

int main() {
    namespace logging = boost::log;

    logging::core::get()->set_filter(logging::trivial::severity >= logging::trivial::info);
    MockRPCService service;
    auto insts = build_cluster(service, 5);
    BOOST_LOG_TRIVIAL(info) << "cluster generation complete";
    start_event_loop(service, insts);
    return 0;
}
