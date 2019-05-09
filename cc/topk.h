#pragma once
#include "rpc/server.h"

struct TopK {
    vector<uint64_t> data;
    MSGPACK_DEFINE_ARRAY(count);
};
