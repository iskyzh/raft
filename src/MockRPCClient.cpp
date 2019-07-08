//
// Created by Alex Chi on 2019-07-08.
//

#include "MockRPCClient.h"
#include "MockRPCService.h"

MockRPCClient::MockRPCClient(MockRPCService* service, const string& id, on_rpc_cb callback)
    : service(service), id(id), callback(callback) {}
