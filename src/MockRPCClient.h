//
// Created by Alex Chi on 2019-07-08.
//

#ifndef RAFT_MOCKRPCCLIENT_H
#define RAFT_MOCKRPCCLIENT_H

#include <string>

#include "MockRPC.h"

using std::string;

class MockRPCService;

class MockRPCClient {
public:
    MockRPCService* service;
    const string id;
    on_rpc_cb callback;

    MockRPCClient(MockRPCService* service, const string& id, on_rpc_cb callback);
};


#endif //RAFT_MOCKRPCCLIENT_H
