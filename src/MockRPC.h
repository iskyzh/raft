//
// Created by Alex Chi on 2019-07-08.
//

#ifndef RAFT_MOCKRPC_H
#define RAFT_MOCKRPC_H

#include <google/protobuf/message.h>
#include <string>
#include <memory>
#include <functional>
using on_rpc_cb = std::function<void(const std::string&, const std::string&, std::shared_ptr<google::protobuf::Message>)>;

#endif //RAFT_MOCKRPC_H
