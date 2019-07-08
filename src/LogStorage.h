//
// Created by Alex Chi on 2019-07-08.
//

#ifndef RAFT_LOGSTORAGE_H
#define RAFT_LOGSTORAGE_H

#include <vector>
#include <string>

using Log = std::pair<unsigned int, std::string>;

class LogStorage {
public:
    std::vector<Log> logs;

    unsigned int last_log_index() { return logs.size() - 1; }
    unsigned int last_log_term() {
        if (logs.empty()) return 0;
        else return logs.back().first;
    }
    void append_log(const Log& log) {
        logs.push_back(log);
    }
};


#endif //RAFT_LOGSTORAGE_H
