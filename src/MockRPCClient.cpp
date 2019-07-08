//
// Created by Alex Chi on 2019-07-08.
//

#include "MockRPCClient.h"
#include "MockRPCService.h"

MockRPCClient::MockRPCClient(MockRPCService *service, const string &id)
        : service(service), id(id) {}

void MockRPCClient::send(const string &to, shared_ptr<Message> message) {
    service->send(id, to, message);
}
