//
// Created by Alex Chi on 2019-07-08.
//

#ifndef RAFT_INSTANCE_H
#define RAFT_INSTANCE_H

#include <string>
#include <map>
#include <google/protobuf/message.h>
#include <boost/optional.hpp>

#include "../utils/utils.h"
#include "LogStorage.h"
#include "RPCClient.h"

using std::shared_ptr;
using std::unique_ptr;
using std::vector;
using boost::optional;
using google::protobuf::Message;
using std::string;
using std::map;
using std::ostream;

enum Role {
    FOLLOWER = 0, CANDIDATE, LEADER
};

using Cluster = string;

class Instance {
public:
    static const unsigned MAX_LOG_TRANSFER = 50;
    Term current_term;
    optional<const string> voted_for;
    LogStorage logs;
    Role role;
    const string id;
    shared_ptr<RPCClient> rpc;
    vector<Cluster> clusters;
    vector<Cluster> clusters_including_self;
    TICK follower_timeout;
    TICK follower_begin;
    TICK election_timeout;
    TICK election_begin;
    unsigned election_vote_cnt;
    map<Cluster, bool> voted_for_self;
    map<Cluster, Index> next_index;
    map<Cluster, Index> match_index;
    Index commit_index;
    Index last_applied;
    bool membership_change_in_progress;

    bool __debug_offline;

    /**
     * Constructor
     * id is node id, rpc is an instance of RPCClient
     * @param id
     * @param rpc
     */
    Instance(const string &id, shared_ptr<RPCClient> rpc);

    /**
     * Initialize raft node
     */
    void start();

    /**
     * Run routine tasks
     */
    void update();

    /**
     * Generate election timeout
     * @return election timeout between 150 and 300
     */
    static TICK generate_timeout();

    /**
     * Increase term number and begin election
     */
    void begin_election();

    /**
     * Send RPCs to nodes in cluster to synchronize logs
     */
    void sync_log();

    /**
     * Transform to follower
     */
    void as_follower();

    /**
     * Transform to candidate
     */
    void as_candidate();

    /**
     * Transform to leader
     */
    void as_leader();

    /**
     * Get corresponding role string
     * @return string of role
     */
    string get_role_string();

    /**
     * Update clusters
     * @param clusters
     */
    void set_clusters(const vector<Cluster> &clusters);

    /**
     * Get cluster size
     * @return cluster size
     */
    unsigned cluster_size();

    // TODO: remove from parameter as it is no longer used
    /**
     * Process RPC message
     * @param from (deprecated)
     * @param message
     */
    void on_rpc(const string &from, shared_ptr<Message> message);

    /**
     * Get term number in message
     * @param message
     * @return term number
     */
    static Term get_term(shared_ptr<Message> message);

    /**
     * Get sender in message
     * @param message
     * @return sender string
     */
    static string get_from(shared_ptr<Message> message);

    /**
     * Append entry to leader
     * @param entry
     */
    void append_entry(const string &entry);

    /**
     * Check whether a message is membership change
     * @param entry
     */
    void try_membership_change(const string &entry);

    /**
     * After old-new cluster config is committed, commit new cluster config
     * (WIP)
     */
    void resolve_membership_change();

    /**
     * Check if a node is in current cluster
     * @param c node id
     * @return if in cluster, true, else false
     */
    bool check_in_cluster(const Cluster &c);
};


#endif //RAFT_INSTANCE_H
