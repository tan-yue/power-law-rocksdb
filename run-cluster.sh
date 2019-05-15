#!/bin/bash

if [[ $# -ne 5 ]]; then
	echo 'usage: ./run-cluster.sh /directory/to/logs number-of-instances k percent /path/to/input/file'
	exit 1	
fi
logDir=$1
num=$2
k=$3
percent=$4
inputFile=$5

rm -rf ../testdb* ../wal*
mkdir -p $logDir
end=$((num - 1))
# first start all `num` instances
for ((i=0;i<=end;i++));
do
	nohup time build/instance $i &> $logDir/instance$i.log &
done

# wait 10s
sleep 10
# next start the coordinator, it reads in a input file
nohup time build/coordinator $num < $inputFile &> $logDir/coord.log &

# wait 10s
sleep 10
# start the partitioner
nohup time build/partitioner $num $k $percent &> $logDir/pa.log &
