//
// Created by Alex Chi on 2019-07-08.
//

#include "MockRPCService.h"
#include "MockRPCClient.h"

unique_ptr<MockRPCClient> MockRPCService::get_client(const string &sender) {
    return std::make_unique<MockRPCClient>(this, sender);
}

void MockRPCService::send(const string &from, const string &to, shared_ptr<Message> message) {
    message_queue.push_back(RPCMessage(from, to, message));
}

