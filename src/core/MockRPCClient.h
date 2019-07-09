//
// Created by Alex Chi on 2019-07-08.
//

#ifndef RAFT_MOCKRPCCLIENT_H
#define RAFT_MOCKRPCCLIENT_H

#include <string>

#include "RPC.h"
#include "MockRPCService.h"
#include "RPCClient.h"

using std::shared_ptr;
using std::unique_ptr;
using std::string;
using google::protobuf::Message;
using std::string;

class MockRPCService;

class MockRPCClient : public RPCClient {
public:
    MockRPCService *service;
    const string id;

    MockRPCClient(MockRPCService *service, const string &id);

    void send(const string &to, shared_ptr<Message> message) override ;
};

#endif //RAFT_MOCKRPCCLIENT_H
