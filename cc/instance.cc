#include <cassert>
#include <cstdlib>
#include <climits>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <thread>
#include <sstream>
#include <iomanip>

#include "rocksdb/db.h"
#include "rpc/server.h"
#include "rpc/this_server.h"
#include "topk.h"

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

void put(uint64_t key) {
#ifdef DEBUG
    cout << "Receive a Put to key " << to_string((long)key) << endl;
#endif
    ostringstream ss;
    ss << setw(8) << setfill('0') << key;
    WriteOptions wo;
    //ignore the returned status
    db->Put(wo, ss.str(), *BigSlice(value_size));
}

TopK report_topk() {
    //TODO: nothing yet
    return TopK{0};
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
    thread db_th([index] () {
        cout << "Starting db_srv" << endl;
        rpc::server db_srv((uint16_t)(8080 + index));
        db_srv.bind("put", put);
        db_srv.bind("report_dbstats", report_dbstats);
        //db_srv.bind("report_topk", report_topk);
        db_srv.bind(
            "stop_db_srv",
            [index]() {
                rpc::this_server().stop();
                delete db;
                cout << "db_srv: Instance " << to_string(index) << " stopped." << endl;
            }
        );
        db_srv.run();
    });

    cout << "Starting pa_srv" << endl;
    rpc::server pa_srv((uint16_t)(9080 + index));
    pa_srv.bind("report_topk", report_topk);
    pa_srv.bind(
        "stop_pa_srv",
        [db_name, index]() {
            rpc::this_server().stop();
            cout << "db_srv: Instance " << to_string(index) << " stopped." << endl;
        }
    );
    pa_srv.run();

    return 0;
}
