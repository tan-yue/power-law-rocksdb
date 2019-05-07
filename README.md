* `./run-cluster.sh number-of-instances /path/to/input/file` starts `number-of-instances` rocksDb instances, one coorinator and one partitioner.
* `./kill.sh` kills all rocksDb instances, the coordinator, and the partitioner.
* A forked `rpclib` repo is contained as a git submodule, so when cloning this repo
  * either do `git clone --recursive git@github.com:tan-yue/power-law-rocksdb.git`
  * or `git clone` without `--recursive` and do `git submodule update --init` inside the cloned repo
