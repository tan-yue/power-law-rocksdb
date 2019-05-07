#include <string>
#include <iostream>
#include <vector>
#include <thread>

#include "rpc/client.h"
#include "rpc/server.h"

using namespace std;

class Coordinator {
    vector<rpc::client *> clients;
public:
    Coordinator (int num) {
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
};

int Coordinator::lookup (uint64_t key) {
    return 0;
}

void handle_repartition() {
    //TODO
    //test only
    cout << "Hello from handle_repartition" << endl;
}

int main(int argc, char * argv[]) {
    assert(argc == 2);

    Coordinator coord(stoi(argv[1]));
    cout << "Coordinator started." << endl;

    thread puts([&coord]() {
            string key;
            while (getline(cin, key)) {
#ifdef DEBUG
                cout << "Receive a Put to key " << key << endl;
#endif
                coord.put((uint64_t)stol(key));
            }
    });

    rpc::server srv((uint16_t)7080);
    srv.bind("handle_repartition", handle_repartition);
    srv.run();

    coord.stop();
    cout << "Coordinator stopped." << endl;

    return 0;
}
