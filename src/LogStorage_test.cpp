//
// Created by Alex Chi on 2019-07-08.
//

#include "LogStorage.h"
#include "gtest/gtest.h"

TEST(LogStorage, Construct) {
    LogStorage logStorage;
}

TEST(LogStorage, LastLog) {
    LogStorage logStorage;
    EXPECT_EQ(logStorage.last_log_index(), -1);
    EXPECT_EQ(logStorage.last_log_term(), 0);
}

TEST(LogStorage, AppendLog) {
    LogStorage logStorage;
    logStorage.append_log(std::make_pair(233, "Log!"));
    EXPECT_EQ(logStorage.last_log_term(), 233);
    EXPECT_EQ(logStorage.last_log_index(), 0);
}

TEST(LogStorage, ProbeLog) {
    LogStorage logStorage;
    EXPECT_TRUE(logStorage.probe_log(-1, 0));
    EXPECT_FALSE(logStorage.probe_log(0, 1));
    logStorage.append_log(std::make_pair(233, "Log!"));
    EXPECT_FALSE(logStorage.probe_log(0, 232));
    EXPECT_TRUE(logStorage.probe_log(0, 233));
    EXPECT_FALSE(logStorage.probe_log(233, 232));
}

TEST(LogStorage, PurgeLog) {
    LogStorage logStorage;
    logStorage.append_log(std::make_pair(233, "Log!"));
    logStorage.append_log(std::make_pair(233, "Log!"));
    logStorage.append_log(std::make_pair(233, "Log!"));
    logStorage.append_log(std::make_pair(233, "Log!"));
    logStorage.append_log(std::make_pair(233, "Log!"));
    logStorage.purge(3);
    EXPECT_EQ(logStorage.logs.size(), 3);
}
