//
// Created by Alex Chi on 2019-07-08.
//

#include <iostream>
#include <memory>
#include <ctime>
#include <cstdlib>
#include <queue>
#include <boost/log/trivial.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "Instance.h"

#include "raft.pb.h"
#include "raft.grpc.pb.h"

using boost::none;
using boost::property_tree::ptree;
using boost::property_tree::ptree_error;
using boost::property_tree::read_json;
using boost::property_tree::write_json;
using std::string;
using std::make_shared;
using std::dynamic_pointer_cast;
using std::priority_queue;

Instance::Instance(const string &id, shared_ptr<RPCClient> rpc) :
        role(FOLLOWER), voted_for(none), id(id), rpc(rpc),
        current_term(0), commit_index(-1), last_applied(-1),
        __debug_offline(false), membership_change_in_progress(false) {
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
    if (__debug_offline) return;
    auto message_term = get_term(message);
    auto message_from = get_from(message);
    if (!check_in_cluster(message_from)) return;
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
            Index lst_index = req_app->prevlogindex();
            if (req_app->term() < this->current_term) succeed = false;
                // log doesn't match
            else if (!logs.probe_log(req_app->prevlogindex(), req_app->prevlogterm())) succeed = false;
            else {
                for (Index next_idx = req_app->prevlogindex() + 1, cnt = 0;
                     cnt < req_app->entries_size(); cnt++, next_idx++) {
                    if (logs.exists(next_idx)) {
                        if (logs.logs[next_idx].first != req_app->term()) logs.purge(next_idx);
                    }
                    if (!logs.exists(next_idx)) {
                        auto entry = req_app->entries(cnt);
                        logs.append_log(make_pair(req_app->entries_term(cnt), entry));
                        try_membership_change(entry);
                    }
                }
                lst_index = logs.last_log_index();
                if (req_app->leadercommit() > commit_index) {
                    commit_index = req_app->leadercommit();
                    // TODO: apply to state machine
                }
            }
            auto res_app = make_shared<AppendEntriesReply>();
            res_app->set_term(current_term);
            res_app->set_success(succeed);
            res_app->set_lastagreedindex(std::min(lst_index, logs.last_log_index()));
            res_app->set_from(id);
            rpc->send(req_app->leaderid(), res_app);
        }
    } else if (role == CANDIDATE) {
        if (auto res_vote = dynamic_pointer_cast<RequestVoteReply>(message)) {
            if (res_vote->votegranted()) {
                auto from = res_vote->from();
                if (!voted_for_self[from]) {
                    voted_for_self[from] = true;
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
                next_index[res_app->from()] = std::max((Index) res_app->lastagreedindex(), (Index) 0);
            }
            priority_queue<Index> match_index_heap;
            for (auto &&kv : match_index) {
                auto idx = kv.second;
                if (logs.exists(idx) && logs.logs[idx].first == current_term) match_index_heap.push(idx);
            }
            for (int i = 0; i < cluster_size() / 2; i++) {
                if (!match_index_heap.empty()) match_index_heap.pop(); else break;
            }
            if (!match_index_heap.empty()) commit_index = std::max(match_index_heap.top(), commit_index);
        }
    }
}

Term Instance::get_term(shared_ptr<Message> message) {
    if (auto req_vote = dynamic_pointer_cast<RequestVoteRequest>(message)) return req_vote->term();
    else if (auto res_vote = dynamic_pointer_cast<RequestVoteReply>(message)) return res_vote->term();
    else if (auto req_app = dynamic_pointer_cast<AppendEntriesRequest>(message)) return req_app->term();
    else if (auto res_app = dynamic_pointer_cast<AppendEntriesReply>(message)) return res_app->term();
    assert(false);
    return -1;
}

void Instance::as_leader() {
    BOOST_LOG_TRIVIAL(info) << id << " become leader in clusters of " << clusters_including_self.size();
    role = LEADER;
    next_index.clear();
    match_index.clear();
    for (auto &&cluster : clusters) {
        next_index[cluster] = logs.last_log_index() + 1;
        match_index[cluster] = -1;
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

        for (int cnt = 0; cnt < MAX_LOG_TRANSFER && next_idx < logs.logs.size(); cnt++, next_idx++) {
            req_app->add_entries_term(logs.logs[next_idx].first);
            req_app->add_entries(logs.logs[next_idx].second);
        }

        req_app->set_leadercommit(commit_index);
        rpc->send(cluster, req_app);
    }
}

void Instance::append_entry(const string &entry) {
    logs.append_log(make_pair(current_term, entry));
    try_membership_change(entry);
}

string Instance::get_role_string() {
    if (role == LEADER) return "leader";
    if (role == CANDIDATE) return "candidate";
    if (role == FOLLOWER) return "follower";
    return "";
}

void Instance::try_membership_change(const string &entry) {
    try {
        ptree pt;
        std::stringstream ss(entry);
        read_json(ss, pt);
        if (pt.get<string>("type") == "membership_change") {
            auto clusters = pt.get_child("clusters");
            std::map<string, string> rpc_clusters;
            std::vector<Cluster> known_nodes;
            for (auto &v : clusters) {
                rpc_clusters[v.first] = v.second.get_value<string>();
                known_nodes.push_back(v.first);
            }
            rpc->update_clusters(rpc_clusters);
            set_clusters(known_nodes);
            membership_change_in_progress = true;
        }
    } catch (...) { return; }
}

void Instance::resolve_membership_change() {
    membership_change_in_progress = false;
}

bool Instance::check_in_cluster(const Cluster &c) {
    return std::find(clusters.begin(), clusters.end(), c) != clusters.end();
}

string Instance::get_from(shared_ptr<Message> message) {
    if (auto req_vote = dynamic_pointer_cast<RequestVoteRequest>(message)) return req_vote->candidateid();
    else if (auto res_vote = dynamic_pointer_cast<RequestVoteReply>(message)) return res_vote->from();
    else if (auto req_app = dynamic_pointer_cast<AppendEntriesRequest>(message)) return req_app->leaderid();
    else if (auto res_app = dynamic_pointer_cast<AppendEntriesReply>(message)) return res_app->from();
    assert(false);
    return "";
}
