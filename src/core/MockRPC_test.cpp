//
// Created by Alex Chi on 2019-07-08.
//

#include "gtest/gtest.h"
#include "MockRPCService.h"
#include "MockRPCClient.h"
#include <string>
#include <memory>

class MockInstance {
public:
    void on_rpc(const string& sender, shared_ptr<Message> message) {

    }
};

using std::bind;
using std::string;

TEST(MockRPC, Construct) {
    using namespace std::placeholders;
    MockRPCService service;
    MockInstance inst;
    auto cb = bind(&MockInstance::on_rpc, &inst, _1, _2);
    EXPECT_NE(service.get_client("test0"), nullptr);
    auto client = dynamic_cast<MockRPCClient*>(&(*service.get_client("test0")));
    EXPECT_EQ(client->id, "test0");
}

