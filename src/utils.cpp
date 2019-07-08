//
// Created by Alex Chi on 2019-07-08.
//

#include "utils.h"

TICK get_tick() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}
