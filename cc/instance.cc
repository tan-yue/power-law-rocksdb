#include "rocksdb/db.h"
#include <cassert>
#include <cstdlib>
#include <climits>
#include <iostream>
#include <string>
#include <vector>
#include <memory>

#include "rpc/server.h"

using namespace rocksdb;
using namespace std;

// global variables ...
DB * db;

// constants
#define value_size (1024 * 4) // 4KB

inline Slice* BigSlice(size_t size) {
  char* buf = new char[size];
  memset(buf, 1, size);
  return new Slice(buf, size);
}

inline void IntToSlice (Slice& s, char* buf, uint64_t input) {
  buf[7] =  input & 0xff;
  buf[6] = (input >> 8) & 0xff;
  buf[5] = (input >> 16) & 0xff;
  buf[4] = (input >> 24) & 0xff;
  buf[3] = (input >> 32) & 0xff;
  buf[2] = (input >> 40) & 0xff;
  buf[1] = (input >> 48) & 0xff;
  buf[0] = (input >> 56) & 0xff;
  s = Slice(buf, sizeof(input));
}

void put(uint64_t key) {
    Slice key_slice;
    char * buf = new char[8];
    IntToSlice(key_slice, buf, stol(key));
    WriteOptions wo;
    //ignore the returned status
    db->Put(wo, key_slice, *BigSlice(value_size));
}

void report_dbstats() {
    std::string out;
    db->GetProperty("rocksdb.num-immutable-mem-table", &out);
    std::cout << "# immutable memtables not yet flushed: " << out << std::endl;
    //db->GetProperty("rocksdb.num-immutable-mem-table-flushed", &out);
    //std::cout << "# immutable memtables already flushed: " << out << std::endl;
    db->GetProperty("rocksdb.mem-table-flush-pending", &out);
    if (out == "0") {
        std::cout << "No pending memtable flushing" << std::endl;
    } else {
        db->GetProperty("rocksdb.num-running-flushes", &out);
        std::cout << "There are " + out + " pending memtable flushings" << std::endl;
    }

    db->GetProperty("rocksdb.compaction-pending", &out);
    if (out == "0") {
        std::cout << "No pending compaction" << std::endl;
    } else {
        db->GetProperty("rocksdb.num-running-compactions", &out);
        std::cout << "There are " + out + " running compactions" << std::endl;
    }
    // Levels
    db->GetProperty("rocksdb.levelstats", &out);
    std::cout << out << std::endl;
    // Detailed statistics (see include/statistics.h)
    db->GetProperty("rocksdb.options-statistics", &out);
    std::cout << out << std::endl;
}

int main(int argc, char* argv[]){
    assert(argc == 2);

    string db_name = "/disk/scratch1/ty/testdb" + string(argv[1]);
    Options options;
    DestroyDB(db_name, options);
    options.create_if_missing = true;
    options.error_if_exists = true;
    options.write_buffer_size = 64 << 20;
    options.statistics = CreateDBStatistics();
    Status status = DB::Open(options, db_name, &db);
    assert(status.ok());

    int index = stoi(argv[1]);
    rpc::server db_srv((uint16_t)(8080 + i));
    db_srv.bind("put", put);
    db_srv.bind("report_dbstats", report_dbstats);
    db_srv.bind(
        "stop_db_srv",
        []() {
            rpc::this_server().stop();
            delete db;
            DestroyDB(db_name, options);
        }
    );

    db_srv.run();

    return 0;
}
