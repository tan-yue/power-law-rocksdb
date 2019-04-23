#include "rocksdb/db.h"
#include <cassert>
#include <iostream>
#include <cstdlib>
#include <climits>
#include <string>
#include "random.h"

using namespace rocksdb;

// constants
#define rand_min 1
#define rand_max (1024 << 10 - 1) // 2^20
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

int main(int argc, char* argv[]){
    //open a DB
    DB * db;
    Options options;
    DestroyDB("/disk/scratch1/ty/testdb", options);
    options.create_if_missing = true;
    options.error_if_exists = true;
    options.write_buffer_size = 4 << 20; // use 4MB memtable to force compaction
    options.statistics = CreateDBStatistics();
    Status status = DB::Open(options, "/disk/scratch1/ty/testdb", &db);
    assert(status.ok());

    uint64_t key;
    Slice key_slice;
    char * buf = new char[8];
    rocksdb::WriteOptions wo;
    std::string line;

    while (std::getline(std::cin, line)) {
        key = (uint64_t)std::stol(line);
        IntToSlice(key_slice, buf, key);
        status = db->Put(wo, key_slice, *BigSlice(value_size));
    }

    //report
    std::string out;
    std::cout << "Memtable Size: " << (options.write_buffer_size >> 20) << " MB" << std::endl;
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
    //std::cout << "Total Puts: " << putting_count << std::endl;
    //std::cout << "Total failed Puts: " << putting_count_fail << std::endl;
    //std::cout << "Total successful Puts: " << putting_count - putting_count_fail << std::endl;
    //std::cout << "Total Puts in Bytes: " << (long)putting_count * value_size << std::endl;
    
    delete db;
    DestroyDB("/disk/scratch1/ty/testdb", options);

    return 0;
}
