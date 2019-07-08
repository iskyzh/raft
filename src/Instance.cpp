//
// Created by Alex Chi on 2019-07-08.
//

#include <iostream>
#include <memory>
#include <ctime>
#include <cstdlib>

#include "Instance.h"

#include "raft.pb.h"
#include "raft.grpc.pb.h"

using boost::none;
using std::string;
using std::make_shared;

Instance::Instance(const string &id, shared_ptr<MockRPCClient> rpc) :
    role(FOLLOWER), votedFor(none), id(id), rpc(rpc),
    currentTerm(0) {
    std::srand(std::time(nullptr));
}

int Instance::run() {
    auto request_vote_msg = std::make_unique<RequestVoteRequest>();
    request_vote_msg->set_candidateid("test_0");
    request_vote_msg->set_term(0);
    request_vote_msg->set_lastlogindex(0);
    request_vote_msg->set_lastlogterm(0);
    return 0;
}

void Instance::update(TICK tick) {
    if (role == FOLLOWER) {
        if (tick - follower_begin > follower_timeout) {
            as_candidate(tick);
        }
    }
}

unique_ptr<Message> Instance::on_rpc() {
    return unique_ptr<Message>();
}

TICK Instance::generate_timeout() {
    return std::rand() % 150 + 150;
}

void Instance::as_follower(TICK tick) {
    role = FOLLOWER;
    follower_timeout = generate_timeout();
    follower_begin = tick;
}

void Instance::start(TICK tick) {
    as_follower(tick);
}

void Instance::as_candidate(TICK tick) {
    role = CANDIDATE;
    begin_election(tick);
}

void Instance::begin_election(TICK tick) {
    election_begin = tick;
    currentTerm++;
    votedFor.emplace(id);
    election_vote_cnt = 1;

    for (auto&& cluster : clusters) {
        auto rpc_message = make_shared<RequestVoteRequest>();
        rpc_message->set_term(currentTerm);
        rpc_message->set_candidateid(id);
        rpc_message->set_lastlogterm(logs.last_log_term());
        rpc_message->set_lastlogindex(logs.last_log_index());
        rpc->send(cluster, rpc_message);
    }
}

void Instance::set_clusters(const vector<Cluster>& clusters) {
    this->clusters = clusters;
}

unsigned Instance::cluster_size() {
    return clusters.size();
}
