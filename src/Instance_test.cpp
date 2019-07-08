//
// Created by Alex Chi on 2019-07-08.
//

#include "gtest/gtest.h"

#include "Instance.h"
#include "MockRPCService.h"
#include "raft.pb.h"

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
    Instance instance("test", nullptr);
    set_test_clusters(instance);
    EXPECT_EQ(instance.cluster_size(), 5);
}

// Follower tests

TEST(Follower, BeginAsFollower) {
    Instance instance("test", nullptr);
    EXPECT_EQ(instance.role, Role::FOLLOWER);
}

TEST(Follower, TransformToCandidate) {
    MockRPCService service;
    Instance instance("test", service.get_client("test"));

    instance.start(500);
    instance.update(600);
    EXPECT_EQ(instance.role, Role::FOLLOWER);
    instance.update(1000);
    EXPECT_EQ(instance.role, Role::CANDIDATE);
}

using std::dynamic_pointer_cast;

TEST(Candidate, BeginElection) {
    MockRPCService service;
    Instance instance("test", service.get_client("test"));

    set_test_clusters(instance);
    instance.as_candidate(0);
    EXPECT_EQ(*instance.votedFor, "test");
    EXPECT_EQ(instance.currentTerm, 1);

    bool request_vote_sent = false;
    for (auto &&message : service.message_queue) {
        if (message.from == "test" && message.to == "test1" && dynamic_pointer_cast<RequestVoteRequest>(message.message)) {
            request_vote_sent = true;
        }
    }

    EXPECT_TRUE(request_vote_sent);
}
