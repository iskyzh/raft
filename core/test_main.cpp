//
// Created by Alex Chi on 2019-07-08.
//
#include "gtest/gtest.h"
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

int main(int argc, char **argv) {
    namespace logging = boost::log;

    logging::core::get()->set_filter(logging::trivial::severity >= logging::trivial::error);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
