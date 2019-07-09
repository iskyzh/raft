// Generated by the gRPC C++ plugin.
// If you make any local change, they will be lost.
// source: raft.proto

#include "raft.pb.h"
#include "raft.grpc.pb.h"

#include <functional>
#include <grpcpp/impl/codegen/async_stream.h>
#include <grpcpp/impl/codegen/async_unary_call.h>
#include <grpcpp/impl/codegen/channel_interface.h>
#include <grpcpp/impl/codegen/client_unary_call.h>
#include <grpcpp/impl/codegen/client_callback.h>
#include <grpcpp/impl/codegen/method_handler_impl.h>
#include <grpcpp/impl/codegen/rpc_service_method.h>
#include <grpcpp/impl/codegen/server_callback.h>
#include <grpcpp/impl/codegen/service_type.h>
#include <grpcpp/impl/codegen/sync_stream.h>

static const char* Raft_method_names[] = {
  "/Raft/RequestVote",
  "/Raft/OnRequestVote",
  "/Raft/AppendEntries",
  "/Raft/OnAppendEntries",
};

std::unique_ptr< Raft::Stub> Raft::NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options) {
  (void)options;
  std::unique_ptr< Raft::Stub> stub(new Raft::Stub(channel));
  return stub;
}

Raft::Stub::Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel)
  : channel_(channel), rpcmethod_RequestVote_(Raft_method_names[0], ::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_OnRequestVote_(Raft_method_names[1], ::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_AppendEntries_(Raft_method_names[2], ::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_OnAppendEntries_(Raft_method_names[3], ::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  {}

::grpc::Status Raft::Stub::RequestVote(::grpc::ClientContext* context, const ::RequestVoteRequest& request, ::Void* response) {
  return ::grpc::internal::BlockingUnaryCall(channel_.get(), rpcmethod_RequestVote_, context, request, response);
}

void Raft::Stub::experimental_async::RequestVote(::grpc::ClientContext* context, const ::RequestVoteRequest* request, ::Void* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall(stub_->channel_.get(), stub_->rpcmethod_RequestVote_, context, request, response, std::move(f));
}

void Raft::Stub::experimental_async::RequestVote(::grpc::ClientContext* context, const ::grpc::ByteBuffer* request, ::Void* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall(stub_->channel_.get(), stub_->rpcmethod_RequestVote_, context, request, response, std::move(f));
}

void Raft::Stub::experimental_async::RequestVote(::grpc::ClientContext* context, const ::RequestVoteRequest* request, ::Void* response, ::grpc::experimental::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create(stub_->channel_.get(), stub_->rpcmethod_RequestVote_, context, request, response, reactor);
}

void Raft::Stub::experimental_async::RequestVote(::grpc::ClientContext* context, const ::grpc::ByteBuffer* request, ::Void* response, ::grpc::experimental::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create(stub_->channel_.get(), stub_->rpcmethod_RequestVote_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::Void>* Raft::Stub::AsyncRequestVoteRaw(::grpc::ClientContext* context, const ::RequestVoteRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderFactory< ::Void>::Create(channel_.get(), cq, rpcmethod_RequestVote_, context, request, true);
}

::grpc::ClientAsyncResponseReader< ::Void>* Raft::Stub::PrepareAsyncRequestVoteRaw(::grpc::ClientContext* context, const ::RequestVoteRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderFactory< ::Void>::Create(channel_.get(), cq, rpcmethod_RequestVote_, context, request, false);
}

::grpc::Status Raft::Stub::OnRequestVote(::grpc::ClientContext* context, const ::RequestVoteReply& request, ::Void* response) {
  return ::grpc::internal::BlockingUnaryCall(channel_.get(), rpcmethod_OnRequestVote_, context, request, response);
}

void Raft::Stub::experimental_async::OnRequestVote(::grpc::ClientContext* context, const ::RequestVoteReply* request, ::Void* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall(stub_->channel_.get(), stub_->rpcmethod_OnRequestVote_, context, request, response, std::move(f));
}

void Raft::Stub::experimental_async::OnRequestVote(::grpc::ClientContext* context, const ::grpc::ByteBuffer* request, ::Void* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall(stub_->channel_.get(), stub_->rpcmethod_OnRequestVote_, context, request, response, std::move(f));
}

void Raft::Stub::experimental_async::OnRequestVote(::grpc::ClientContext* context, const ::RequestVoteReply* request, ::Void* response, ::grpc::experimental::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create(stub_->channel_.get(), stub_->rpcmethod_OnRequestVote_, context, request, response, reactor);
}

void Raft::Stub::experimental_async::OnRequestVote(::grpc::ClientContext* context, const ::grpc::ByteBuffer* request, ::Void* response, ::grpc::experimental::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create(stub_->channel_.get(), stub_->rpcmethod_OnRequestVote_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::Void>* Raft::Stub::AsyncOnRequestVoteRaw(::grpc::ClientContext* context, const ::RequestVoteReply& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderFactory< ::Void>::Create(channel_.get(), cq, rpcmethod_OnRequestVote_, context, request, true);
}

::grpc::ClientAsyncResponseReader< ::Void>* Raft::Stub::PrepareAsyncOnRequestVoteRaw(::grpc::ClientContext* context, const ::RequestVoteReply& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderFactory< ::Void>::Create(channel_.get(), cq, rpcmethod_OnRequestVote_, context, request, false);
}

::grpc::Status Raft::Stub::AppendEntries(::grpc::ClientContext* context, const ::AppendEntriesRequest& request, ::Void* response) {
  return ::grpc::internal::BlockingUnaryCall(channel_.get(), rpcmethod_AppendEntries_, context, request, response);
}

void Raft::Stub::experimental_async::AppendEntries(::grpc::ClientContext* context, const ::AppendEntriesRequest* request, ::Void* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall(stub_->channel_.get(), stub_->rpcmethod_AppendEntries_, context, request, response, std::move(f));
}

void Raft::Stub::experimental_async::AppendEntries(::grpc::ClientContext* context, const ::grpc::ByteBuffer* request, ::Void* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall(stub_->channel_.get(), stub_->rpcmethod_AppendEntries_, context, request, response, std::move(f));
}

void Raft::Stub::experimental_async::AppendEntries(::grpc::ClientContext* context, const ::AppendEntriesRequest* request, ::Void* response, ::grpc::experimental::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create(stub_->channel_.get(), stub_->rpcmethod_AppendEntries_, context, request, response, reactor);
}

void Raft::Stub::experimental_async::AppendEntries(::grpc::ClientContext* context, const ::grpc::ByteBuffer* request, ::Void* response, ::grpc::experimental::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create(stub_->channel_.get(), stub_->rpcmethod_AppendEntries_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::Void>* Raft::Stub::AsyncAppendEntriesRaw(::grpc::ClientContext* context, const ::AppendEntriesRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderFactory< ::Void>::Create(channel_.get(), cq, rpcmethod_AppendEntries_, context, request, true);
}

::grpc::ClientAsyncResponseReader< ::Void>* Raft::Stub::PrepareAsyncAppendEntriesRaw(::grpc::ClientContext* context, const ::AppendEntriesRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderFactory< ::Void>::Create(channel_.get(), cq, rpcmethod_AppendEntries_, context, request, false);
}

::grpc::Status Raft::Stub::OnAppendEntries(::grpc::ClientContext* context, const ::AppendEntriesReply& request, ::Void* response) {
  return ::grpc::internal::BlockingUnaryCall(channel_.get(), rpcmethod_OnAppendEntries_, context, request, response);
}

void Raft::Stub::experimental_async::OnAppendEntries(::grpc::ClientContext* context, const ::AppendEntriesReply* request, ::Void* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall(stub_->channel_.get(), stub_->rpcmethod_OnAppendEntries_, context, request, response, std::move(f));
}

void Raft::Stub::experimental_async::OnAppendEntries(::grpc::ClientContext* context, const ::grpc::ByteBuffer* request, ::Void* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall(stub_->channel_.get(), stub_->rpcmethod_OnAppendEntries_, context, request, response, std::move(f));
}

void Raft::Stub::experimental_async::OnAppendEntries(::grpc::ClientContext* context, const ::AppendEntriesReply* request, ::Void* response, ::grpc::experimental::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create(stub_->channel_.get(), stub_->rpcmethod_OnAppendEntries_, context, request, response, reactor);
}

void Raft::Stub::experimental_async::OnAppendEntries(::grpc::ClientContext* context, const ::grpc::ByteBuffer* request, ::Void* response, ::grpc::experimental::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create(stub_->channel_.get(), stub_->rpcmethod_OnAppendEntries_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::Void>* Raft::Stub::AsyncOnAppendEntriesRaw(::grpc::ClientContext* context, const ::AppendEntriesReply& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderFactory< ::Void>::Create(channel_.get(), cq, rpcmethod_OnAppendEntries_, context, request, true);
}

::grpc::ClientAsyncResponseReader< ::Void>* Raft::Stub::PrepareAsyncOnAppendEntriesRaw(::grpc::ClientContext* context, const ::AppendEntriesReply& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderFactory< ::Void>::Create(channel_.get(), cq, rpcmethod_OnAppendEntries_, context, request, false);
}

Raft::Service::Service() {
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      Raft_method_names[0],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< Raft::Service, ::RequestVoteRequest, ::Void>(
          std::mem_fn(&Raft::Service::RequestVote), this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      Raft_method_names[1],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< Raft::Service, ::RequestVoteReply, ::Void>(
          std::mem_fn(&Raft::Service::OnRequestVote), this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      Raft_method_names[2],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< Raft::Service, ::AppendEntriesRequest, ::Void>(
          std::mem_fn(&Raft::Service::AppendEntries), this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      Raft_method_names[3],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< Raft::Service, ::AppendEntriesReply, ::Void>(
          std::mem_fn(&Raft::Service::OnAppendEntries), this)));
}

Raft::Service::~Service() {
}

::grpc::Status Raft::Service::RequestVote(::grpc::ServerContext* context, const ::RequestVoteRequest* request, ::Void* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status Raft::Service::OnRequestVote(::grpc::ServerContext* context, const ::RequestVoteReply* request, ::Void* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status Raft::Service::AppendEntries(::grpc::ServerContext* context, const ::AppendEntriesRequest* request, ::Void* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status Raft::Service::OnAppendEntries(::grpc::ServerContext* context, const ::AppendEntriesReply* request, ::Void* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}


