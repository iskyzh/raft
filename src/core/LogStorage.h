//
// Created by Alex Chi on 2019-07-08.
//

#ifndef RAFT_LOGSTORAGE_H
#define RAFT_LOGSTORAGE_H

#include <vector>
#include <string>
#include <boost/log/trivial.hpp>
#include <iostream>

using Log = std::pair<unsigned int, std::string>;

class LogStorage {
public:
    std::vector<Log> logs;

    unsigned int last_log_index() { return logs.size() - 1; }

    unsigned int last_log_term() {
        if (logs.empty()) return 0;
        else return logs.back().first;
    }

    void append_log(const Log &log) {
        logs.push_back(log);
    }

    bool probe_log(unsigned int log_index, unsigned int log_term) {
        if (log_index == -1) return true;
        if (log_index >= logs.size()) return false;
        if (logs[log_index].first != log_term) return false;
        return true;
    }

    bool exists(unsigned int log_index) {
        return log_index < logs.size();
    }

    void purge(unsigned int log_index) {
        while (logs.size() > log_index) logs.pop_back();
    }

    void dump() {
        for (auto &&log : logs) {
            std::cout<< log.first << " " << log.second << "\t";
        }
        std::cout << std::endl;
    }
};


#endif //RAFT_LOGSTORAGE_H
