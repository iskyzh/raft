//
// Created by Alex Chi on 2019-07-08.
//

#ifndef RAFT_INSTANCE_H
#define RAFT_INSTANCE_H

#include <string>
#include <google/protobuf/message.h>
#include <boost/optional.hpp>

#include "LogStorage.h"
#include "MockRPCClient.h"

using std::shared_ptr;
using std::unique_ptr;
using std::vector;
using boost::optional;
using google::protobuf::Message;
using std::string;

enum Role {
    FOLLOWER = 0, CANDIDATE, LEADER
};

using TICK = unsigned long long;
using Cluster = string;

class Instance {
public:
    unsigned int currentTerm;
    optional<const string> votedFor;
    LogStorage logs;
    Role role;
    const string id;
    shared_ptr<MockRPCClient> rpc;
    vector<Cluster> clusters;
    TICK follower_timeout;
    TICK follower_begin;
    TICK election_begin;
    unsigned election_vote_cnt;

    Instance(const string &id, shared_ptr<MockRPCClient> rpc);

    int run();

    void start(TICK tick);

    void update(TICK tick);

    TICK generate_timeout();

    void begin_election(TICK tick);

    void as_follower(TICK tick);

    void as_candidate(TICK tick);

    void set_clusters(const vector<Cluster>& clusters);

    unsigned cluster_size();

    unique_ptr<Message> on_rpc();
};


#endif //RAFT_INSTANCE_H
