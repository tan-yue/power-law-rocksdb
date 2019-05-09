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
    int placeholder;
    MSGPACK_DEFINE_ARRAY(placeholder);
};
