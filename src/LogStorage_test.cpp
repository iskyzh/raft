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
