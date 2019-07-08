//
// Created by Alex Chi on 2019-07-08.
//

#include <boost/log/trivial.hpp>

#include "raft.pb.h"

#include "MockRPCService.h"
#include "MockRPCClient.h"

using std::dynamic_pointer_cast;

unique_ptr<MockRPCClient> MockRPCService::get_client(const string &sender) {
    return std::make_unique<MockRPCClient>(this, sender);
}

void MockRPCService::send(const string &from, const string &to, shared_ptr<Message> message) {
    message_queue.push_back(RPCMessage(from, to, message));
    if (callback) callback(from, to, message);
}

void MockRPCService::set_callback(on_rpc_cb callback) {
    this->callback = callback;
}

MockRPCService::MockRPCService() : callback({}) {}

void log_message(const string& from, const string& to, shared_ptr<Message> message) {
    if (auto req_vote = dynamic_pointer_cast<RequestVoteRequest>(message)) {
        BOOST_LOG_TRIVIAL(trace) << from << "->" << to << " request vote";
    }
    else if (auto req_vote = dynamic_pointer_cast<RequestVoteReply>(message)) {
        BOOST_LOG_TRIVIAL(trace) << from << "->" << to << " request vote reply";
    }
    else if (auto req_vote = dynamic_pointer_cast<AppendEntriesRequest>(message)) {
        BOOST_LOG_TRIVIAL(trace) << from << "->" << to << " append entry";
    }
    else if (auto req_vote = dynamic_pointer_cast<AppendEntriesReply>(message)) {
        BOOST_LOG_TRIVIAL(trace) << from << "->" << to << " append entry reply";
    }
    else BOOST_LOG_TRIVIAL(error) << from << "->" << to << " unknown message";
};
