#include <cstddef>
#include <iostream>

namespace rocksdb {

int ss_read_stream (char* filename, long long context_size, long long internal_counters);

int ss_get_topk (long long output_counters);

}
