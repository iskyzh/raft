//
// Created by Alex Chi on 2019-07-08.
//

#include <boost/log/trivial.hpp>
#include <google/protobuf/text_format.h>

#include "raft.pb.h"

#include "MockRPCService.h"
#include "MockRPCClient.h"

using std::dynamic_pointer_cast;
using google::protobuf::TextFormat;

shared_ptr<RPCClient> MockRPCService::get_client(const string &sender) {
    return std::make_shared<MockRPCClient>(this, sender);
}

void MockRPCService::send(const string &from, const string &to, shared_ptr<Message> message) {
    message_queue.push_back(RPCMessage(from, to, message));
    if (callback) callback(from, to, message);
}

void MockRPCService::set_callback(on_rpc_cb callback) {
    this->callback = callback;
}

MockRPCService::MockRPCService() : callback({}) {}

void log_message(const string &from, const string &to, shared_ptr<Message> message) {
#ifndef NDEBUG
    string result;
    TextFormat::PrintToString(*message, &result);
    std::replace(result.begin(), result.end(), '\n', ' ');
    if (auto req_vote = dynamic_pointer_cast<RequestVoteRequest>(message)) {
        BOOST_LOG_TRIVIAL(trace) << from << "->" << to << " request vote " << result;
    } else if (auto res_vote = dynamic_pointer_cast<RequestVoteReply>(message)) {
        BOOST_LOG_TRIVIAL(trace) << from << "->" << to << " request vote reply " << result;
    } else if (auto req_app = dynamic_pointer_cast<AppendEntriesRequest>(message)) {
        BOOST_LOG_TRIVIAL(trace) << from << "->" << to << " append entry " << result << " entry_size: " << req_app->entries_size();
    } else if (auto res_app = dynamic_pointer_cast<AppendEntriesReply>(message)) {
        BOOST_LOG_TRIVIAL(trace) << from << "->" << to << " append entry reply " << result;
    } else
        BOOST_LOG_TRIVIAL(error) << from << "->" << to << " unknown message " << result;
#endif
}
