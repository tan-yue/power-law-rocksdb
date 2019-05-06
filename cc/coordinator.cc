#include <string>
#include <iostream>
#include <vector>

#include "rpc/client.h"

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

int main(int argc, char * argv[]) {
    assert(argc == 2);

    Coordinator coord(stoi(argv[1]));

    string key;
    while (getline(cin, key)) {
        coord.put((uint64_t)stol(key));
    }
    coord.stop();
    
    return 0;
}
