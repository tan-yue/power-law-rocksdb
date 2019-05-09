#include <chrono>
#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include <unistd.h>

#include "rpc/client.h"
#include "topk.h"

using namespace std;
typedef RPCLIB_MSGPACK::object_handle handle;

class Partitioner {
    vector<rpc::client *> clients;
    rpc::client * coord_client;
    int k;
public:
    Partitioner(int num) : k(500) {
        string localhost = "127.0.0.1";
        for (int i = 0; i < num; ++i) {
            clients.push_back(new rpc::client(localhost, (uint16_t)(9080 + i)));
        }
        coord_client = new rpc::client(localhost, (uint16_t)7080);
    }

    ~Partitioner() {
        delete coord_client;
    }

    void run() {
#ifdef DEBUG
        int seconds_to_sleep = 6;
#else
        int seconds_to_sleep = 600;
#endif
        while (true) {
#ifdef DEBUG
            cout << "Hello from run()" << endl;
#endif
            sleep(seconds_to_sleep);
            auto start = chrono::steady_clock::now();
            vector<future<handle> > futures;
            futures.reserve(clients.size());
            for (auto it = clients.begin(); it != clients.end(); ++it) {
                futures.push_back((*it)->async_call("report_topk", k));
            }
            repartition(futures);
            seconds_to_sleep -=
                chrono::duration_cast<chrono::seconds>(chrono::steady_clock::now() - start)
                .count();
        }
    }

private:
    void repartition(vector<future<handle> > & futures);
};

void Partitioner::repartition(vector<future<handle> > & futures) {
    for (auto it = futures.begin(); it != futures.end(); ++it) {
#ifdef DEBUG
        cout << "waiting..." << endl;
#endif
        it->wait();
    }

    // merge sort
    vector<pair<uint64_t, int> > merged;
    vector<size_t> curs;
    while(merged.size() < k * clients.size()) {
        size_t max_i = 0;
        unsigned long long max_cnt = 0;
        for (size_t i = 0; i < clients.size(); ++i) {
            if (curs[i] < k && futures[i].get().as<TopK>().data[curs[i]].second > max_cnt)
                max_i = i;
        }
        merged.push_back({(uint64_t)futures[max_i].get().as<TopK>().data[curs[max_i]].first, (int)max_i});
        ++curs[max_i];
    }

    // TODO:make repartition decisions
    ExceptionList exceptions{0};

    // let's do not wait for response
    coord_client->async_call("handle_repartition", exceptions);
}

int main(int argc, char * argv[]) {
    assert(argc == 2);

    Partitioner p(stoi(argv[1]));

    // this is a blocking call
    p.run();

    return 0;
}
