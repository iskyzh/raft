//
// Created by Alex Chi on 2019-07-08.
//

#ifndef RAFT_MOCKRPCSERVICE_H
#define RAFT_MOCKRPCSERVICE_H

#include <memory>
#include <string>
#include <vector>

#include "MockRPC.h"

using std::shared_ptr;
using std::unique_ptr;
using std::string;
using std::vector;
using std::pair;
using google::protobuf::Message;

class MockRPCClient;

class MockRPCService {
public:
    using message = pair<string, shared_ptr<Message>>;
    vector<message> message_queue;

    unique_ptr<MockRPCClient> get_client(const string &sender, on_rpc_cb callback);

    void send(const string &sender, shared_ptr<Message> message);
};


#endif //RAFT_MOCKRPCSERVICE_H
