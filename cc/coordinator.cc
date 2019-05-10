#include <string>
#include <iostream>
#include <vector>
#include <thread>

#include "rpc/client.h"
#include "rpc/server.h"
#include "topk.h"

using namespace std;

class Coordinator {
    vector<rpc::client *> clients;
    uint64_t max_key;
public:
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

    int lookup (uint64_t key);
    
    void stop() {
        for (auto it = clients.begin(); it != clients.end(); ++it) {
            (*it)->call("stop_db_srv");
        }
    }

    void report_stats() {
        for (auto it = clients.begin(); it != clients.end(); ++it) {
            (*it)->call("report_dbstats");
        }
    } 
};

int Coordinator::lookup (uint64_t key) {
    return (int)(key % (uint64_t)clients.size());
}

void handle_repartition(ExceptionList exceptions) {
    //TODO
    //test only
    cout << "Hello from handle_repartition" << endl;
}

int main(int argc, char * argv[]) {
    assert(argc == 2);

    Coordinator coord(stoi(argv[1]), 4<<20); // key space is 4 million
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
    });

    rpc::server srv((uint16_t)7080);
    srv.bind("handle_repartition", handle_repartition);
    srv.run();

    coord.stop();
    cout << "Coordinator stopped." << endl;

    return 0;
}
