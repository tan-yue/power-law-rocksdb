#include <chrono>
#include <iostream>
#include <fstream>
#include <tuple>
#include <string>
#include <vector>
#include <cassert>
#include <unistd.h>
#include <ctime>

#include "rpc/client.h"
#include "topk.h"

using namespace std;
typedef RPCLIB_MSGPACK::object_handle handle;
typedef vector<pair<unsigned long long, unsigned long long>> TopkData;

class Partitioner {
    vector<rpc::client *> clients;
    rpc::client * coord_client;
    int k;
public:
    Partitioner(int num) : k(10) {
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
        int seconds_to_sleep = 10;
#else
        int seconds_to_sleep = 60;
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
                futures.push_back(std::move((*it)->async_call("report_topk", k)));
            }
            repartition(futures);
            // sync call
            //vector<TopkData> topks;
            //for (size_t i = 0; i < clients.size(); ++i) {
            //    cout << "client " << i << endl;
            //    TopkData topk = clients[i]->call("report_topk", k).as<TopK>().data;
            //    assert(typeid(topk) == typeid(TopkData));
            //    topks.push_back(topk);
            //    //topks.back().size();
            //}
            //repartition(topks);
            seconds_to_sleep -=
                chrono::duration_cast<chrono::seconds>(chrono::steady_clock::now() - start)
                .count();
        }
    }

private:
    void repartition(vector<future<handle> > & futures);
    void repartition(const vector<TopkData> & topks);
};

void Partitioner::repartition(vector<future<handle> > & futures) {
    vector<TopkData> topks;
    for (auto it = futures.begin(); it != futures.end(); ++it) {
        topks.push_back(std::move(it->get().as<TopK>().data));
    }
    // merge sort
    vector<tuple<uint64_t, uint64_t, int> > merged;
    vector<size_t> curs(clients.size(), 0);
    
    cout << "key_id\tfrequency\tinstance#" << endl;
    while(merged.size() < k * clients.size()) {
        size_t max_i = 0;
        unsigned long long max_cnt = 0;
        for (size_t i = 0; i < clients.size(); ++i) {
            if (curs[i] < k && topks[i][curs[i]].second > max_cnt){
                max_i = i;
            }
        }
        merged.emplace_back((uint64_t)topks[max_i][curs[max_i]].first, (uint64_t)topks[max_i][curs[max_i]].second, (int)max_i);
        cout << topks[max_i][curs[max_i]].first << "," << topks[max_i][curs[max_i]].second << "," << max_i << endl;
        ++curs[max_i];
    }

    // TODO:make repartition decisions
    ExceptionList exceptions{0};

    // let's do not wait for response
    coord_client->async_call("handle_repartition", exceptions);
}

void Partitioner::repartition(const vector<TopkData> & topks) {
    // merge sort
    vector<tuple<uint64_t, uint64_t, int> > merged;
    vector<size_t> curs(clients.size(), 0);
    
    while(merged.size() < k * clients.size()) {
        size_t max_i = 0;
        unsigned long long max_cnt = 0;
        cout << "round " << merged.size() << endl;
        for (size_t i = 0; i < clients.size(); ++i) {
            if (curs[i] < k && topks[i][curs[i]].second > max_cnt){
                max_i = i;
            }
        }
        uint64_t key = (uint64_t)topks[max_i][curs[max_i]].first;
        uint64_t freq = (uint64_t)topks[max_i][curs[max_i]].second;
        cout << key << "," << freq << "," << max_i << endl;
        merged.emplace_back(key, freq, (int)max_i);
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
