//
// Created by Alex Chi on 2019-07-08.
//

#include "gtest/gtest.h"

#include "Instance.h"

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
    instance.set_clusters({"test0", "test1", "test2", "test3", "test4"});
    EXPECT_EQ(instance.cluster_size(), 5);
}

// Follower tests

TEST(Follower, BeginAsFollower) {
    Instance instance("test", nullptr);
    EXPECT_EQ(instance.role, Role::FOLLOWER);
}

TEST(Follower, TransformToCandidate) {
    Instance instance("test", nullptr);
    instance.start(500);
    instance.update(600);
    EXPECT_EQ(instance.role, Role::FOLLOWER);
    instance.update(1000);
    EXPECT_EQ(instance.role, Role::CANDIDATE);
}

TEST(Candidate, BeginElection) {
    Instance instance("test", nullptr);
    instance.as_candidate(0);
    EXPECT_EQ(*instance.votedFor, "test");
    EXPECT_EQ(instance.currentTerm, 1);
}

