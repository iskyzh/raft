//
// Created by Alex Chi on 2019-07-09.
//

#include <cpptoml.h>

#include "rpc/grpc_client.hpp"
#include "core/Instance.h"
#include "utils/utils.h"
#include "core/MockRPCService.h"

class RaftControl final : public Control::Service {
public:
    struct ControlEvent : public Event {
        std::string cmd;
        enum TYPE {
            SHUTDOWN, ONLINE, OFFLINE
        } type;

        ControlEvent(TYPE type, const std::string &cmd) : cmd(cmd), type(type) {}
    };

    struct LogAppendEvent : public Event {
        std::vector<std::string> logs;

        LogAppendEvent(const vector<string> &logs) : logs(logs) {}
    };

    Status AppendLog(grpc::ServerContext *context, const AppendLogRequest *request,
                     AppendLogReply *response) override {
        std::vector<std::string> logs;
        for (int i = 0; i < request->log_size(); i++) {
            logs.push_back(request->log(i));
        }
        client->q.push(new LogAppendEvent(logs));
        return Status::OK;
    }

    Status Alive(grpc::ServerContext *context, const Void *request, Void *response) override {
        return Status::OK;
    }

    Status RequestLog(grpc::ServerContext *context, const RequestLogRequest *request,
                      RequestLogReply *response) override {
        for (auto &&log : inst->logs.logs) {
            response->add_logs(log.second);
        }
        response->set_role(inst->get_role_string());
        response->set_commitindex(inst->commit_index);
        return Status::OK;
    }

    Status Shutdown(grpc::ServerContext *context, const Void *request, Void *response) override {
        client->q.push(new ControlEvent(ControlEvent::SHUTDOWN, ""));
        return Status::OK;
    }

    Status Offline(grpc::ServerContext *context, const Void *request, Void *response) override {
        client->__debug_supress_rpc_send = true;
        client->q.push(new ControlEvent(ControlEvent::OFFLINE, ""));
        return Status::OK;
    }

    Status Online(grpc::ServerContext *context, const Void *request, Void *response) override {
        client->__debug_supress_rpc_send = false;
        client->q.push(new ControlEvent(ControlEvent::ONLINE, ""));
        return Status::OK;
    }

    shared_ptr<RaftRPCClient> client;
    shared_ptr<Instance> inst;

    RaftControl(shared_ptr<RaftRPCClient> client, shared_ptr<Instance> inst)
            : client(client), inst(inst) {}


};

string generate_message(int id) {
    char s[100];
    sprintf(s, "test%d", id);
    return s;
}

int start_event_loop(shared_ptr<Instance> inst, shared_ptr<RaftRPCClient> client, shared_ptr<grpc::Server> server) {
    BOOST_LOG_TRIVIAL(info) << inst->id << " starting event loop...";
    inst->start();
    auto lst_updated = get_tick();
    auto lst_append_entry = get_tick();
    bool shutdown = false;
    while (!shutdown) {
        if (get_tick() - lst_updated >= 30) {
            lst_updated = get_tick();
            inst->update();
        }
        if (get_tick() - lst_append_entry >= 1000) {
            lst_append_entry = get_tick();
            BOOST_LOG_TRIVIAL(info) << inst->id << " " << inst->get_role_string() << " size: "
                                    << inst->logs.logs.size() << " commit: " << inst->commit_index;

        }
        Event *event;
        while (client->q.pop(event)) {
            if (auto rpc = dynamic_cast<RaftRPCClient::RPCMessage *>(event)) {
                log_message(rpc->from, inst->id, rpc->message);
                inst->on_rpc(rpc->from, rpc->message);
            } else if (auto control = dynamic_cast<RaftControl::ControlEvent *>(event)) {
                if (control->type == RaftControl::ControlEvent::SHUTDOWN) {
                    shutdown = true;
                    BOOST_LOG_TRIVIAL(info) << inst->id << " shutting down...";
                    server->Shutdown();
                    delete event;
                    break;
                } else if (control->type == RaftControl::ControlEvent::ONLINE) {
                    inst->__debug_offline = false;
                    BOOST_LOG_TRIVIAL(info) << inst->id << " node online";
                } else if (control->type == RaftControl::ControlEvent::OFFLINE) {
                    BOOST_LOG_TRIVIAL(info) << inst->id << " node offline";
                    inst->__debug_offline = true;
                }
            } else if (auto log_append = dynamic_cast<RaftControl::LogAppendEvent *> (event)) {
                if (inst->role == LEADER) {
                    for (auto &&entry : log_append->logs) {
                        inst->append_entry(entry);
                    }
                    BOOST_LOG_TRIVIAL(info) << inst->id << " has requested append " << log_append->logs.size()
                                            << " entries";
                    for (int i = 0; i < 5 && i < log_append->logs.size(); i++) {
                        BOOST_LOG_TRIVIAL(debug) << log_append->logs[i];
                    }
                }
            }
            delete event;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    return 0;
}

int main(int argc, char **argv) {
    namespace logging = boost::log;
    if (argc == 3 && strcmp(argv[2], "--verbose") == 0)
        logging::core::get()->set_filter(logging::trivial::severity >= logging::trivial::trace);
    else
        logging::core::get()->set_filter(logging::trivial::severity >= logging::trivial::error);

    const char *config_path = "config.toml";
    if (argc >= 2) { config_path = argv[1]; }

    BOOST_LOG_TRIVIAL(trace) << "loading config from " << config_path;

    auto config = cpptoml::parse_file(config_path);

    auto log_level = config->get_qualified_as<int>("server.log_level");
    auto tclusters = config->get_table_array("clusters");

    map<string, string> route;
    vector<string> clusters;

    for (const auto &cluster : *tclusters) {
        auto name = cluster->get_as<string>("name");
        auto addr = cluster->get_as<string>("addr");
        if (!name || !addr) continue;
        route[*name] = *addr;
        clusters.push_back(*name);
        BOOST_LOG_TRIVIAL(trace) << "add cluster " << *name << " at " << *addr;
    }

    auto server_name = config->get_qualified_as<string>("server.name");
    auto server_addr = config->get_qualified_as<string>("server.addr");

    if (!server_name || !server_addr) {
        BOOST_LOG_TRIVIAL(error) << "please specify server in config";
        return 0;
    }

    BOOST_LOG_TRIVIAL(trace) << "server " << *server_name << " at " << *server_addr;

    BOOST_LOG_TRIVIAL(info) << "config loaded from " << config_path;

    auto client = make_shared<RaftRPCClient>(*server_name, *server_addr, route);
    auto instance = make_shared<Instance>(*server_name, client);
    auto control = new RaftControl(client, instance);
    auto server = client->make_server(control);
    instance->set_clusters(clusters);

    thread server_thread([server]() {
        BOOST_LOG_TRIVIAL(info) << "server set up";
        server->Wait();
    });
    thread event_thread([instance, client, server] { start_event_loop(instance, client, server); });

    event_thread.join();
    server_thread.join();

    delete control;

    return 0;
}
