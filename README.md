* `./run-cluster.sh number-of-instances /path/to/input/file` starts `number-of-instances` rocksDb instances, one coorinator and one partitioner.
* `./kill.sh` kills all rocksDb instances, the coordinator, and the partitioner.
* A forked `rpclib` repo is contained as a git submodule, so when cloning this repo
  * either do `git clone --recursive git@github.com:tan-yue/power-law-rocksdb.git`
  * or `git clone` without `--recursive` and do `git submodule update --init` inside the cloned repo
# TODO
Implement the two functions below
1. cc/coordinator.cc:Coordinator::lookup
2. cc/partitioner.cc:Partitioner::repartition
# Build and Run
* gen-zipf.sh: a script generating Zipfian stream to the standrad output
* `make` compiles files in folder `cc` and puts executables in folder `build`
* `make debug` does the same thing as `make` but turn on debug messages
* For instructions of building rpclib, please refer to [rpclib's doc page](https://github.com/rpclib/rpclib/blob/master/doc/pages/compiling.md)
