//
// Created by Alex Chi on 2019-07-09.
//

#ifndef RAFT_RPCCLIENT_H
#define RAFT_RPCCLIENT_H

#include <string>
#include <memory>
#include "RPC.h"

using std::shared_ptr;
using std::unique_ptr;
using std::string;
using google::protobuf::Message;

class RPCClient {
public:
    virtual void send(const string &to, shared_ptr<Message> message) = 0;
    virtual ~RPCClient() {}
};
#endif //RAFT_RPCCLIENT_H
