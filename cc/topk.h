#pragma once
#include "rpc/server.h"

struct TopK {
    int count;
    MSGPACK_DEFINE_ARRAY(count);
};
