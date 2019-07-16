//
// Created by Alex Chi on 2019-07-09.
//

#ifndef RAFT_GRPC_CLIENT_HPP
#define RAFT_GRPC_CLIENT_HPP

#include <iostream>
#include <string>
#include <memory>
#include <map>
#include <thread>
#include <grpcpp/grpcpp.h>
#include <grpc/support/log.h>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/lockfree/queue.hpp>
#include "../core/RPCClient.h"
#include "../core/MockRPCService.h"

#include "raft.grpc.pb.h"

using std::unique_ptr;
using std::make_shared;
using std::string;
using std::shared_ptr;
using std::map;
using std::dynamic_pointer_cast;
using std::thread;
using grpc::Server;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerCompletionQueue;
using grpc::Status;
using grpc::Channel;
using grpc::ClientAsyncResponseReader;
using grpc::ClientContext;
using grpc::CompletionQueue;
using google::protobuf::Message;
using boost::lockfree::queue;

struct Event {
    virtual ~Event() {}
};

class RaftRPCClient final : public Raft::Service, public RPCClient {
public:
    struct RPCMessage : public Event {
        const string from;
        shared_ptr<Message> message;

        RPCMessage(const string &from, shared_ptr<Message> message)
                : from(from), message(message) {}

        static RPCMessage *build(const string &from, shared_ptr<Message> message) {
            return new RPCMessage(from, message);
        }

        ~RPCMessage() {}
    };

    queue<Event *> q;

    Status RequestVote(ServerContext *context, const RequestVoteRequest *request, Void *response) override {
        q.push(RPCMessage::build(context->peer(), make_shared<RequestVoteRequest>(*request)));
        return Status::OK;
    }

    Status OnRequestVote(ServerContext *context, const RequestVoteReply *request, Void *response) override {
        q.push(RPCMessage::build(context->peer(), make_shared<RequestVoteReply>(*request)));
        return Status::OK;
    }

    Status AppendEntries(ServerContext *context, const AppendEntriesRequest *request, Void *response) override {
        q.push(RPCMessage::build(context->peer(), make_shared<AppendEntriesRequest>(*request)));
        return Status::OK;
    }

    Status OnAppendEntries(ServerContext *context, const AppendEntriesReply *request, Void *response) override {
        q.push(RPCMessage::build(context->peer(), make_shared<AppendEntriesReply>(*request)));
        return Status::OK;
    }

    RaftRPCClient(const string& id, const string &server_addr, const map<string, string> &clusters)
            : id(id), server_addr(server_addr), q(65536), clusters(clusters) {
        for (auto &&kv : clusters) {
            auto channel = grpc::CreateChannel(
                    kv.second,
                    grpc::InsecureChannelCredentials());
            stub[kv.first] = Raft::NewStub(channel);
        }
    }

    RaftRPCClient() : RaftRPCClient("test", "127.0.0.1:23333", map<string, string>()) {}

    shared_ptr<Server> make_server(Control::Service *control_service) {
        ServerBuilder builder;
        builder.AddListeningPort(server_addr, grpc::InsecureServerCredentials());
        builder.RegisterService(this);
        if (control_service != nullptr) builder.RegisterService(control_service);
        shared_ptr<Server> server(builder.BuildAndStart());
        return server;
    }

    void dispatch_send(const string to, shared_ptr<Message> message) {
        ClientContext context;
        Void reply;
        Status status;
        if (stub.find(to) == stub.end()) {
            BOOST_LOG_TRIVIAL(warning) << "rpc destination unreachable";
            return;
        }
        if (auto req_vote = dynamic_pointer_cast<RequestVoteRequest>(message)) {
            status = stub[to]->RequestVote(&context, *req_vote, &reply);
        } else if (auto res_vote = dynamic_pointer_cast<RequestVoteReply>(message)) {
            status = stub[to]->OnRequestVote(&context, *res_vote, &reply);
        } else if (auto req_app = dynamic_pointer_cast<AppendEntriesRequest>(message)) {
            status = stub[to]->AppendEntries(&context, *req_app, &reply);
        } else if (auto res_app = dynamic_pointer_cast<AppendEntriesReply>(message)) {
            status = stub[to]->OnAppendEntries(&context, *res_app, &reply);
        }
        if (!status.ok()) {
            BOOST_LOG_TRIVIAL(warning) << "rpc call failed";
        }
    }

    void send(const string &to, shared_ptr<Message> message) override {
        thread send_thread(&RaftRPCClient::dispatch_send, this, to, message);
        send_thread.detach();
    }

    string server_addr, id;
    map<string, unique_ptr<Raft::Stub>> stub;
    map<string, string> clusters;
};

#endif //RAFT_GRPC_CLIENT_HPP
