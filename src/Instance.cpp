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
using std::dynamic_pointer_cast;

Instance::Instance(const string &id, shared_ptr<MockRPCClient> rpc) :
        role(FOLLOWER), voted_for(none), id(id), rpc(rpc),
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

void Instance::update() {
    if (role == FOLLOWER) {
        if (get_tick() - follower_begin > follower_timeout) {
            as_candidate();
        }
    } else if (role == CANDIDATE) {
        if (get_tick() - election_begin > election_timeout) {
            begin_election();
        }
    } else if (role == LEADER) {
        sync_log();
    }
}

TICK Instance::generate_timeout() {
    return std::rand() % 150 + 150;
}

void Instance::as_follower() {
    role = FOLLOWER;
    follower_timeout = generate_timeout();
    follower_begin = get_tick();
    // TODO: not sure
    voted_for = none;
}

void Instance::start() {
    as_follower();
}

void Instance::as_candidate() {
    role = CANDIDATE;
    begin_election();
}

void Instance::begin_election() {
    election_begin = get_tick();
    election_timeout = generate_timeout();
    currentTerm++;
    voted_for.emplace(id);
    voted_for_self.clear();
    voted_for_self[id] = true;
    election_vote_cnt = 1;

    for (auto &&cluster : clusters) {
        auto rpc_message = make_shared<RequestVoteRequest>();
        rpc_message->set_term(currentTerm);
        rpc_message->set_candidateid(id);
        rpc_message->set_lastlogterm(logs.last_log_term());
        rpc_message->set_lastlogindex(logs.last_log_index());
        rpc->send(cluster, rpc_message);
    }
}

void Instance::set_clusters(const vector<Cluster> &clusters) {
    this->clusters = clusters;
    auto iter = std::find(this->clusters.begin(), this->clusters.end(), id);
    if (iter != this->clusters.end()) {
        this->clusters.erase(iter);
    }
    this->clusters_including_self = this->clusters;
    this->clusters_including_self.push_back(id);
}

unsigned Instance::cluster_size() {
    return clusters_including_self.size();
}

void Instance::on_rpc(const string &from, shared_ptr<Message> message) {
    auto message_term = get_term(message);
    if (message_term > currentTerm) {
        currentTerm = message_term;
        as_follower();
    }
    if (role == FOLLOWER) {
        follower_timeout = get_tick();
        if (auto req_vote = dynamic_pointer_cast<RequestVoteRequest>(message)) {
            bool grant_vote = true;
            if (req_vote->term() < this->currentTerm) grant_vote = false;
                // not voted, or same candidate?
            else if (voted_for != none && *voted_for != req_vote->candidateid()) grant_vote = false;
                // at least as up-to-date?
            else if (req_vote->lastlogindex() < this->logs.last_log_index()) grant_vote = false;

            auto vote_reply = make_shared<RequestVoteReply>();
            vote_reply->set_term(currentTerm);
            vote_reply->set_votegranted(grant_vote);

            rpc->send(req_vote->candidateid(), vote_reply);
        }

        // TODO: Append Entries RPC
    } else if (role == CANDIDATE) {
        if (auto res_vote = dynamic_pointer_cast<RequestVoteReply>(message)) {
            if (res_vote->votegranted()) {
                if (!voted_for_self[from]) {
                    voted_for_self[from] = true;
                    ++election_vote_cnt;
                    if (election_vote_cnt > cluster_size() / 2) {
                        as_leader();
                    }
                }
            }
        } else if (auto req_app = dynamic_pointer_cast<AppendEntriesRequest>(message)) {
            if (req_app->term() == currentTerm) {
                as_follower();
                // TODO: reply when fallback to follower
            } else {
                auto res_app = make_shared<AppendEntriesReply>();
                res_app->set_term(currentTerm);
                res_app->set_lastagreedlogindex(logs.last_log_index());
                res_app->set_success(false);
                rpc->send(req_app->leaderid(), res_app);
            }
        }
    }
}

unsigned int Instance::get_term(shared_ptr<Message> message) {
    unsigned int term = -1;
    if (auto req_vote = dynamic_pointer_cast<RequestVoteRequest>(message)) {
        term = req_vote->term();
    } else if (auto res_vote = dynamic_pointer_cast<RequestVoteReply>(message)) {
        term = res_vote->term();
    } else if (auto req_app = dynamic_pointer_cast<AppendEntriesRequest>(message)) {
        term = req_app->term();
    } else if (auto res_app = dynamic_pointer_cast<AppendEntriesReply>(message)) {
        term = res_app->term();
    }
    if (term == -1) {
        assert(false);
    }
    return term;
}

void Instance::as_leader() {
    role = LEADER;
    next_index.clear();
    match_index.clear();
    sync_log();
}

void Instance::sync_log() {
    for (auto &&cluster : clusters) {
        auto rpc_message = make_shared<AppendEntriesRequest>();
        rpc->send(cluster, rpc_message);
    }
}
