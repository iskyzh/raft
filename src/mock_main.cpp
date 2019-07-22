#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <queue>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include "core/MockRPCService.h"
#include "core/Instance.h"
#include "utils/utils.h"

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


string generate_message(int id) {
    char s[100];
    sprintf(s, "test%d", id);
    return s;
}

void append_entry(vector<unique_ptr<Instance>> &insts) {
    static unsigned message_id = 0;
    string msg = generate_message(message_id++);
    for (auto &&inst : insts) {
        if (inst->role == LEADER) {
            inst->append_entry(msg);
        }
    }
}

struct TestEvent {
    TICK should_be_triggered_at;
    std::string event;

    TestEvent(TICK should_be_triggered_at, const std::string &event)
            : should_be_triggered_at(should_be_triggered_at), event(event) {}

    friend bool operator<(const TestEvent &a, const TestEvent &b) {
        return a.should_be_triggered_at > b.should_be_triggered_at;
    }
};

void setup_test_sequence(priority_queue<TestEvent> &test_events) {
    test_events.push(TestEvent(get_tick() + 5000, "kick_leader"));
    test_events.push(TestEvent(get_tick() + 10000, "restore_leader"));
}

int start_event_loop(MockRPCService &service, vector<unique_ptr<Instance>> &insts) {
    priority_queue<RPCDeferMessage> mq;
    priority_queue<TestEvent> test_events;
    service.set_callback([&insts, &mq](const string &from, const string &to, shared_ptr<Message> message) {
        const double drop_rate = 0.0;
        const int delay = 1;
        if (1.0 * std::rand() / RAND_MAX <= drop_rate) {
            // BOOST_LOG_TRIVIAL(trace) << "rpc message dropped";
            return;
        }
        mq.push(RPCDeferMessage(get_tick() + std::rand() % delay, from, to, message));
    });
    setup_test_sequence(test_events);

    BOOST_LOG_TRIVIAL(info) << "starting event loop...";
    for (auto &&inst : insts) inst->start();
    auto lst_updated = get_tick();
    auto lst_append_entry = get_tick();

    bool block_leader = false;
    insts[0]->as_candidate();
    insts[0]->as_leader();

    while (true) {
        if (get_tick() - lst_updated >= 30) {
            lst_updated = get_tick();
            for (auto &&inst : insts) {
                inst->update();
            }
        }
        if (get_tick() - lst_append_entry >= 500) {
            lst_append_entry = get_tick();
            for (auto &&inst : insts) {
                BOOST_LOG_TRIVIAL(info) << inst->id << " " << inst->get_role_string() << " size: "
                                        << inst->logs.logs.size() << " commit: " << inst->commit_index;
            }
            for (int i = 0; i < 20; i++)
                append_entry(insts);
        }
        while (!mq.empty() && mq.top().should_be_sent_at < get_tick()) {
            auto rpc = mq.top();
            mq.pop();
            if (block_leader && (rpc.from == "test0" || rpc.to == "test0")) continue;
            send_message(insts, rpc.from, rpc.to, rpc.message);
        }
        while (!test_events.empty() && test_events.top().should_be_triggered_at < get_tick()) {
            auto event = test_events.top();
            test_events.pop();
            if (event.event == "kick_leader") {
                insts[0]->__debug_offline = true;
                block_leader = true;
                BOOST_LOG_TRIVIAL(info) << "leader kicked off";
            }
            if (event.event == "restore_leader") {
                insts[0]->__debug_offline = false;
                block_leader = false;
                BOOST_LOG_TRIVIAL(info) << "leader restored";
            }
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
