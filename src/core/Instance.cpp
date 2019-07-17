//
// Created by Alex Chi on 2019-07-08.
//

#include <iostream>
#include <memory>
#include <ctime>
#include <cstdlib>
#include <boost/log/trivial.hpp>

#include "Instance.h"

#include "raft.pb.h"
#include "raft.grpc.pb.h"

using boost::none;
using std::string;
using std::make_shared;
using std::dynamic_pointer_cast;

Instance::Instance(const string &id, shared_ptr<RPCClient> rpc) :
        role(FOLLOWER), voted_for(none), id(id), rpc(rpc),
        current_term(0), commit_index(0), last_applied(0) {
    std::srand(std::time(nullptr));
}

void Instance::update() {
    if (role == FOLLOWER) {
        if (get_tick() - follower_begin > follower_timeout) {
            as_candidate();
        }
    } else if (role == CANDIDATE) {
        if (get_tick() - election_begin > election_timeout) {
            BOOST_LOG_TRIVIAL(info) << id << " election timeout";
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
    BOOST_LOG_TRIVIAL(info) << id << " become follower";
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
    BOOST_LOG_TRIVIAL(info) << id << " become candidate";
    role = CANDIDATE;
    begin_election();
}

void Instance::begin_election() {
    election_begin = get_tick();
    election_timeout = generate_timeout();
    current_term++;
    voted_for.emplace(id);
    voted_for_self.clear();
    voted_for_self[id] = true;
    election_vote_cnt = 1;

    for (auto &&cluster : clusters) {
        auto req_vote = make_shared<RequestVoteRequest>();
        req_vote->set_term(current_term);
        req_vote->set_candidateid(id);
        req_vote->set_lastlogterm(logs.last_log_term());
        req_vote->set_lastlogindex(logs.last_log_index());
        rpc->send(cluster, req_vote);
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

void Instance::on_rpc(const string &, shared_ptr<Message> message) {
    auto message_term = get_term(message);
    if (message_term > current_term) {
        current_term = message_term;
        as_follower();
    }
    if (role == FOLLOWER) {
        follower_begin = get_tick();
        if (auto req_vote = dynamic_pointer_cast<RequestVoteRequest>(message)) {
            bool grant_vote = true;
            if (req_vote->term() < this->current_term) grant_vote = false;
                // not voted, or same candidate?
            else if (voted_for != none && *voted_for != req_vote->candidateid()) grant_vote = false;
                // at least as up-to-date?
            else if (req_vote->lastlogindex() < this->logs.last_log_index()) grant_vote = false;
            auto res_vote = make_shared<RequestVoteReply>();
            res_vote->set_term(current_term);
            res_vote->set_votegranted(grant_vote);
            res_vote->set_from(id);
            if (grant_vote) voted_for.emplace(req_vote->candidateid());
            rpc->send(req_vote->candidateid(), res_vote);
        } else if (auto req_app = dynamic_pointer_cast<AppendEntriesRequest>(message)) {
            bool succeed = true;
            unsigned int lst_index = req_app->prevlogindex();
            if (req_app->term() < this->current_term) succeed = false;
                // log doesn't match
            else if (!logs.probe_log(req_app->prevlogindex(), req_app->prevlogterm())) succeed = false;
            else {
                auto next_idx = req_app->prevlogindex() + 1;
                if (logs.exists(next_idx)) {
                    if (logs.logs[next_idx].first != req_app->term()) logs.purge(next_idx);
                }
                auto entries = req_app->entries();
                if (entries != "" && !logs.exists(next_idx)) logs.append_log(make_pair(req_app->term(), req_app->entries()));
                if (req_app->leadercommit() > commit_index) {
                    commit_index = req_app->leadercommit();
                    // TODO: apply to state machine
                }
                lst_index = logs.last_log_index();
            }
            auto res_app = make_shared<AppendEntriesReply>();
            res_app->set_term(current_term);
            res_app->set_success(succeed);
            res_app->set_lastagreedindex(lst_index);
            res_app->set_from(id);
            rpc->send(req_app->leaderid(), res_app);
        }
    } else if (role == CANDIDATE) {
        if (auto res_vote = dynamic_pointer_cast<RequestVoteReply>(message)) {
            if (res_vote->votegranted()) {
                if (!voted_for_self[res_vote->from()]) {
                    voted_for_self[res_vote->from()] = true;
                    ++election_vote_cnt;
                    if (election_vote_cnt > cluster_size() / 2) {
                        as_leader();
                    }
                }
            }
        } else if (auto req_app = dynamic_pointer_cast<AppendEntriesRequest>(message)) {
            if (req_app->term() == current_term) {
                as_follower();
                // TODO: reply when fallback to follower
            } else {
                auto res_app = make_shared<AppendEntriesReply>();
                res_app->set_term(current_term);
                res_app->set_lastagreedindex(logs.last_log_index());
                res_app->set_success(false);
                res_app->set_from(id);
                rpc->send(req_app->leaderid(), res_app);
            }
        }
    } else if (role == LEADER) {
        if (auto res_app = dynamic_pointer_cast<AppendEntriesReply>(message)) {
            if (res_app->success()) {
                match_index[res_app->from()] = res_app->lastagreedindex();
                next_index[res_app->from()] = res_app->lastagreedindex() + 1;
            } else {
                next_index[res_app->from()] = res_app->lastagreedindex();
            }
        }
    }
}

unsigned int Instance::get_term(shared_ptr<Message> message) {
    if (auto req_vote = dynamic_pointer_cast<RequestVoteRequest>(message)) return req_vote->term();
    else if (auto res_vote = dynamic_pointer_cast<RequestVoteReply>(message)) return res_vote->term();
    else if (auto req_app = dynamic_pointer_cast<AppendEntriesRequest>(message)) return req_app->term();
    else if (auto res_app = dynamic_pointer_cast<AppendEntriesReply>(message)) return res_app->term();
    assert(false);
    return -1;
}

void Instance::as_leader() {
    BOOST_LOG_TRIVIAL(info) << id << " become leader";
    role = LEADER;
    next_index.clear();
    match_index.clear();
    for (auto &&cluster : clusters) {
        next_index[cluster] = logs.last_log_index() + 1;
        match_index[cluster] = 0;
    }
    sync_log();
}

void Instance::sync_log() {
    for (auto &&cluster : clusters) {
        auto req_app = make_shared<AppendEntriesRequest>();
        req_app->set_term(current_term);
        req_app->set_leaderid(id);
        auto log_idx = next_index[cluster] - 1;
        auto next_idx = next_index[cluster];
        req_app->set_prevlogindex(log_idx);
        req_app->set_prevlogterm(log_idx == -1 ? 0 : logs.logs[log_idx].first);
        if (next_idx >= logs.logs.size()) req_app->set_entries("");
        else req_app->set_entries(logs.logs[next_idx].second);
        req_app->set_leadercommit(commit_index);
        rpc->send(cluster, req_app);
    }
}

void Instance::append_entry(const string &entry) {
    logs.append_log(make_pair(current_term, entry));
}

string Instance::get_role_string() {
    if (role == LEADER) return "leader";
    if (role == CANDIDATE) return "candidate";
    if (role == FOLLOWER) return "follower";
    return "";
}


