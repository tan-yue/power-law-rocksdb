#include <string>
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <unistd.h>

#include "rpc/client.h"
#include "rpc/server.h"
#include "topk.h"

using namespace std;

class Coordinator;
Coordinator * coord_ptr = nullptr;
std::mutex except_lock;

class Coordinator {
    vector<rpc::client *> clients;
    uint64_t max_key;
public:
    unordered_map<uint64_t, int> exceptions;

    Coordinator (int num, int max_key_) : max_key(max_key_) {
        string localhost = "127.0.0.1";
        for (int i = 0; i < num; ++i) {
            clients.push_back(new rpc::client(localhost, (uint16_t)(8080 + i)));
        }
    }

    ~Coordinator () {
        for (auto it = clients.begin(); it != clients.end(); ++it) {
            delete *it;
        }
    }

    void put (uint64_t key) {
        clients[this->lookup(key)]->call("put", key);
    }

    void del (int inst, uint64_t key) {
#ifdef DEBUG
        cout << "Calling Delete to key " << key << " on instance " << inst << endl;
#endif
        clients[inst]->call("delete", key);
    }

    int lookup (uint64_t key);
    
    void stop() {
        for (auto it = clients.begin(); it != clients.end(); ++it) {
            (*it)->async_call("stop_db_srv");
        }
        sleep(5);
        cout << "Coordinator stopped." << endl;
    }

    void report_stats() {
        for (auto it = clients.begin(); it != clients.end(); ++it) {
            (*it)->call("report_dbstats");
        }
    } 
};

int Coordinator::lookup (uint64_t key) {
    std::lock_guard<std::mutex> guard(except_lock);
    auto it = exceptions.find(key);
    if (it != exceptions.end()) {
        return it->second;
    } else {
        return (int)(key % (uint64_t)clients.size());
    }
}

void handle_repartition(ExceptionList exceptions) {
    std::lock_guard<std::mutex> guard(except_lock);
    cout << "Hello from handle_repartition" << endl;
    coord_ptr->exceptions = exceptions.key_to_instance;
    for(auto &item: coord_ptr->exceptions) {
        int origin = (int)(item.first % 3);
        coord_ptr->del(origin, item.first);
    }
}

int main(int argc, char * argv[]) {
    assert(argc == 2);

    Coordinator coord(stoi(argv[1]), 4<<20); // key space is 4 million
    coord_ptr = &coord;
    cout << "Coordinator started." << endl;

    thread puts([&coord]() {
            string key;
            while (getline(cin, key)) {
#ifdef DEBUG
                cout << "Receive a Put to key " << key << endl;
#endif
                coord.put((uint64_t)stol(key));
            }
            coord.report_stats();
            coord.stop();
            exit(0);
    });

    rpc::server srv((uint16_t)7080);
    srv.bind("handle_repartition", handle_repartition);
    srv.run();

    return 0;
}
