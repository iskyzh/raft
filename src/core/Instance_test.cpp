//
// Created by Alex Chi on 2019-07-08.
//

#include "gtest/gtest.h"

#include "Instance.h"
#include "MockRPCService.h"
#include "raft.pb.h"

extern TICK __tick;

using std::make_shared;
using std::dynamic_pointer_cast;

void set_test_clusters(Instance &instance) {
    instance.set_clusters({"test0", "test1", "test2", "test3", "test4"});
}

TEST(Instance, Construct) {
    Instance instance("test", nullptr);
    EXPECT_EQ(instance.id, "test");
    EXPECT_EQ(instance.voted_for, boost::none);
}

TEST(Instance, GenerateTimeout) {
    Instance instance("test", nullptr);
    EXPECT_LE(instance.generate_timeout(), 300);
    EXPECT_GT(instance.generate_timeout(), 150);
}

TEST(Instance, SetCluster) {
    Instance instance("test0", nullptr);
    set_test_clusters(instance);
    EXPECT_EQ(instance.cluster_size(), 5);
    EXPECT_EQ(instance.clusters.size(), 4);
}

// Follower tests

TEST(Follower, BeginAsFollower) {
    Instance instance("test", nullptr);
    EXPECT_EQ(instance.role, Role::FOLLOWER);
}

shared_ptr<Message> make_vote_request(const string &from, int term = 0) {
    auto message = make_shared<RequestVoteRequest>();
    message->set_candidateid(from);
    message->set_lastlogindex(-1);
    message->set_lastlogterm(0);
    message->set_term(term);
    return message;
}

shared_ptr<Message> make_vote_reply(const string &from, int term = 0, bool vote_granted = true) {
    auto message = make_shared<RequestVoteReply>();
    message->set_votegranted(vote_granted);
    message->set_term(term);
    message->set_from(from);
    return message;
}

shared_ptr<Message> make_append_entry(const string &from, int term = 0, int prevLogIndex = 0, int prevLogTerm = 0) {
    auto message = make_shared<AppendEntriesRequest>();
    message->set_term(term);
    message->set_leaderid(from);
    message->set_prevlogindex(prevLogIndex);
    message->set_prevlogterm(prevLogTerm);
    message->set_leadercommit(false);
    return message;
}

bool voted_for(MockRPCService &service, const string &from, const string &req) {
    for (auto &&message : service.message_queue) {
        if (message.from == from && message.to == req) {
            auto rpc = dynamic_pointer_cast<RequestVoteReply>(message.message);
            if (rpc && rpc->votegranted()) {
                return true;
            }
        }
    }
    return false;
}

TEST(Follower, VoteForCandidate) {
    MockRPCService service;
    Instance instance("test", service.get_client("test"));

    __tick = 500;
    instance.start();
    __tick = 600;
    instance.update();
    EXPECT_EQ(instance.role, Role::FOLLOWER);
    __tick = 1000;
    instance.on_rpc("test1", make_vote_request("test1"));
    instance.update();
    EXPECT_EQ(instance.role, Role::FOLLOWER);

    EXPECT_TRUE(voted_for(service, "test", "test1"));
}

TEST(Follower, NotVoteForCandidateOfLowerTerm) {
    MockRPCService service;
    Instance instance("test", service.get_client("test"));

    __tick = 500;
    instance.start();
    __tick = 600;
    instance.update();
    EXPECT_EQ(instance.role, Role::FOLLOWER);
    __tick = 1000;
    instance.current_term = 1;
    instance.on_rpc("test1", make_vote_request("test1", 0));
    instance.update();
    EXPECT_EQ(instance.role, Role::FOLLOWER);

    EXPECT_FALSE(voted_for(service, "test", "test1"));
}

TEST(Follower, NotVoteTwice) {
    MockRPCService service;
    Instance instance("test", service.get_client("test"));

    __tick = 500;
    instance.start();
    __tick = 600;
    instance.update();
    EXPECT_EQ(instance.role, Role::FOLLOWER);
    __tick = 1000;
    instance.on_rpc("test1", make_vote_request("test1"));
    instance.on_rpc("test2", make_vote_request("test2"));
    instance.update();
    EXPECT_EQ(instance.role, Role::FOLLOWER);

    EXPECT_TRUE(voted_for(service, "test", "test1"));
    EXPECT_FALSE(voted_for(service, "test", "test2"));
}

// TODO: not vote for candidate with fewer log

TEST(Follower, TransformToCandidate) {
    MockRPCService service;
    Instance instance("test", service.get_client("test"));

    __tick = 500;
    instance.start();
    __tick = 600;
    instance.update();
    EXPECT_EQ(instance.role, Role::FOLLOWER);
    __tick = 1000;
    instance.update();
    EXPECT_EQ(instance.role, Role::CANDIDATE);
}

// Candidate tests
TEST(Candidate, ShouldRestartElection) {
    MockRPCService service;
    Instance instance("test0", service.get_client("test0"));
    set_test_clusters(instance);

    instance.as_candidate();
    __tick = 500;
    instance.update();

    EXPECT_EQ(instance.current_term, 2);
}

TEST(Candidate, BeginElection) {
    MockRPCService service;
    Instance instance("test", service.get_client("test"));

    set_test_clusters(instance);
    __tick = 0;
    instance.as_candidate();
    EXPECT_EQ(*instance.voted_for, "test");
    EXPECT_EQ(instance.current_term, 1);

    bool request_vote_sent = false;
    for (auto &&message : service.message_queue) {
        if (message.from == "test" && message.to == "test1" &&
            dynamic_pointer_cast<RequestVoteRequest>(message.message)) {
            request_vote_sent = true;
        }
    }

    EXPECT_TRUE(request_vote_sent);
}


TEST(Candidate, ShouldBecomeLeader) {
    MockRPCService service;
    Instance instance("test0", service.get_client("test0"));

    set_test_clusters(instance);
    instance.as_candidate();

    instance.on_rpc("test1", make_vote_reply("test1", instance.current_term));
    instance.on_rpc("test2", make_vote_reply("test2", instance.current_term));
    instance.on_rpc("test3", make_vote_reply("test3", instance.current_term));
    instance.on_rpc("test4", make_vote_reply("test4", instance.current_term));

    EXPECT_EQ(instance.role, LEADER);
}


TEST(Candidate, ShouldNotBecomeLeader) {
    MockRPCService service;
    Instance instance("test0", service.get_client("test0"));

    set_test_clusters(instance);
    instance.as_candidate();

    instance.on_rpc("test1", make_vote_reply("test1", instance.current_term));
    instance.on_rpc("test1", make_vote_reply("test1", instance.current_term));
    instance.on_rpc("test1", make_vote_reply("test1", instance.current_term));
    instance.on_rpc("test1", make_vote_reply("test1", instance.current_term));
    instance.on_rpc("test1", make_vote_reply("test1", instance.current_term));
    instance.on_rpc("test2", make_vote_reply("test2", instance.current_term, false));
    instance.on_rpc("test3", make_vote_reply("test3", instance.current_term, false));

    EXPECT_EQ(instance.role, CANDIDATE);
}

TEST(Candidate, ShouldFallbackToFollower) {
    MockRPCService service;
    Instance instance("test0", service.get_client("test0"));

    set_test_clusters(instance);
    instance.as_candidate();

    instance.on_rpc("test1", make_vote_reply("test1", 3, false));

    EXPECT_EQ(instance.role, FOLLOWER);
    EXPECT_EQ(instance.current_term, 3);
}

TEST(Candidate, ShouldFallbackToFollowerWhenAppend) {
    MockRPCService service;
    Instance instance("test0", service.get_client("test0"));

    set_test_clusters(instance);
    instance.as_candidate();

    instance.on_rpc("test1", make_append_entry("test0", instance.current_term));

    EXPECT_EQ(instance.role, FOLLOWER);
}

// Leader tests
TEST(Leader, ShouldFallbackToFollower) {
    MockRPCService service;
    Instance instance("test0", service.get_client("test0"));

    set_test_clusters(instance);
    instance.as_leader();

    instance.on_rpc("test1", make_vote_reply("test1", 3, false));

    EXPECT_EQ(instance.role, FOLLOWER);
    EXPECT_EQ(instance.current_term, 3);
}

bool append_entry_to(MockRPCService &service, const string &to, const string &from) {
    for (auto &&message : service.message_queue) {
        if (message.from == from && message.to == to) {
            auto rpc = dynamic_pointer_cast<AppendEntriesRequest>(message.message);
            if (rpc) {
                return true;
            }
        }
    }
    return false;
}

TEST(Leader, ShouldHeartbeatUponElection) {
    MockRPCService service;
    Instance instance("test0", service.get_client("test0"));
    set_test_clusters(instance);
    instance.as_leader();
    EXPECT_TRUE(append_entry_to(service, "test1", "test0"));
    EXPECT_TRUE(append_entry_to(service, "test2", "test0"));
    EXPECT_TRUE(append_entry_to(service, "test3", "test0"));
    EXPECT_TRUE(append_entry_to(service, "test4", "test0"));
}

shared_ptr<AppendEntriesReply> make_append_entries_reply(std::string from, Index idx, Term term) {
    auto msg = make_shared<AppendEntriesReply>();
    msg->set_term(term);
    msg->set_from(from);
    msg->set_success(true);
    msg->set_lastagreedindex(idx);
    return msg;
}

TEST(Leader, ShouldCommit) {
    MockRPCService service;
    Instance instance("test0", service.get_client("test0"));
    set_test_clusters(instance);
    instance.as_leader();
    instance.on_rpc("", make_append_entries_reply("test1", 3, instance.current_term));
    instance.on_rpc("", make_append_entries_reply("test2", 3, instance.current_term));
    instance.on_rpc("", make_append_entries_reply("test3", 3, instance.current_term));
    instance.on_rpc("", make_append_entries_reply("test4", 3, instance.current_term));
    EXPECT_EQ(instance.commit_index, 3);
}

TEST(Leader, ShouldNotCommit) {
    MockRPCService service;
    Instance instance("test0", service.get_client("test0"));
    set_test_clusters(instance);
    instance.as_leader();
    instance.on_rpc("", make_append_entries_reply("test1", 3, instance.current_term));
    instance.on_rpc("", make_append_entries_reply("test2", 3, instance.current_term));
    EXPECT_EQ(instance.commit_index, -1);
}
