class Partitioner {
    map<uint64_t, int> key_instance;
public:
    Partitioner(size_t num_instances) {
        // hardcoded partitioning for now
        // read in first file
        // read in second file
    }

    ~Partitioner() {}

    int receiveTopK(uint64_t key) {
        return 0; 
    }
};
