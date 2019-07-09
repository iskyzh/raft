//
// Created by Alex Chi on 2019-07-09.
//

#include "../rpc/grpc_client.hpp"

int main() {
    RaftRPCClient client;
    client.run_server();
    return 0;
}