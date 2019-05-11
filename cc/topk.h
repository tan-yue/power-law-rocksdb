#pragma once

#include <vector>
#include <utility>

#include "rpc/server.h"

using namespace std;

struct TopK {
    vector<pair<unsigned long long, unsigned long long> > data;
    MSGPACK_DEFINE_ARRAY(data);
};

struct ExceptionList {
    unordered_map<uint64_t, int> key_to_instance;
    MSGPACK_DEFINE_ARRAY(key_to_instance);
};
