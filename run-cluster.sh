#!/bin/bash

if [[ $# -ne 2 ]]; then
	echo 'usage: ./run-cluster.sh number-of-instances /path/to/input/file'
	exit 1	
fi
mkdir -p logs
num=$1
end=$((num - 1))
# first start all `num` instances
for ((i=0;i<=end;i++));
do
	build/instance $i &> logs/instance$i.log &
done

# wait 10s
sleep 10
# next start the coordinator, it reads in a input file
build/coordinator $num < $2 &> logs/coord.log &

# wait 10s
sleep 10
# start the partitioner
build/partitioner $num &> logs/pa.log &
