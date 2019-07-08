//
// Created by Alex Chi on 2019-07-08.
//

#include "MockRPCService.h"
#include "MockRPCClient.h"

unique_ptr<MockRPCClient> MockRPCService::get_client(const string &sender, on_rpc_cb callback) {
    return std::make_unique<MockRPCClient>(this, sender, callback);
}

