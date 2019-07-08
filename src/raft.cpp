//
// Created by Alex Chi on 2019-07-04.
//

#include <memory>
#include <iostream>
#include <string>
#include <memory>
#include <grpcpp/grpcpp.h>

#include "raft.h"
#include "Instance.h"

using std::unique_ptr;
using std::make_unique;

int raft::run_server() {
    auto instance = std::make_unique<Instance>("test", nullptr);
    return instance->run();
}
