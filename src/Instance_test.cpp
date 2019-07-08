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
    EXPECT_EQ(instance.votedFor, boost::none);
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
    instance.currentTerm = 1;
    instance.on_rpc("test1", make_vote_request("test1", 0));
    instance.update();
    EXPECT_EQ(instance.role, Role::FOLLOWER);

    EXPECT_FALSE(voted_for(service, "test", "test1"));
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

    EXPECT_EQ(instance.currentTerm, 2);
}

TEST(Candidate, BeginElection) {
    MockRPCService service;
    Instance instance("test", service.get_client("test"));

    set_test_clusters(instance);
    __tick = 0;
    instance.as_candidate();
    EXPECT_EQ(*instance.votedFor, "test");
    EXPECT_EQ(instance.currentTerm, 1);

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

    instance.on_rpc("test1", make_vote_reply("test1", instance.currentTerm));
    instance.on_rpc("test2", make_vote_reply("test2", instance.currentTerm));
    instance.on_rpc("test3", make_vote_reply("test3", instance.currentTerm));
    instance.on_rpc("test4", make_vote_reply("test4", instance.currentTerm));

    EXPECT_EQ(instance.role, LEADER);
}


TEST(Candidate, ShouldNotBecomeLeader) {
    MockRPCService service;
    Instance instance("test0", service.get_client("test0"));

    set_test_clusters(instance);
    instance.as_candidate();

    instance.on_rpc("test1", make_vote_reply("test1", instance.currentTerm));
    instance.on_rpc("test1", make_vote_reply("test1", instance.currentTerm));
    instance.on_rpc("test1", make_vote_reply("test1", instance.currentTerm));
    instance.on_rpc("test1", make_vote_reply("test1", instance.currentTerm));
    instance.on_rpc("test1", make_vote_reply("test1", instance.currentTerm));
    instance.on_rpc("test2", make_vote_reply("test2", instance.currentTerm, false));
    instance.on_rpc("test3", make_vote_reply("test3", instance.currentTerm, false));

    EXPECT_EQ(instance.role, CANDIDATE);
}

TEST(Candidate, ShouldFallbackToFollower) {
    MockRPCService service;
    Instance instance("test0", service.get_client("test0"));

    set_test_clusters(instance);
    instance.as_candidate();

    instance.on_rpc("test1", make_vote_reply("test1", 3, false));

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
}
