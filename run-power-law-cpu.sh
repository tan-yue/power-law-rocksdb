#!/bin/bash

for alpha in 1.6
do
	echo "running alpha $alpha..."
	cargo run --release --bin power-law $alpha > alpha_in/alpha"$alpha".in
	date
	./power-law < alpha"$alpha".in > alpha_out/alpha"$alpha".out &
	../cpustat/cpustat 1 7200 -T -p $! > alpha_out/alpha"$alpha"-cpu.out &
	cpustatPid=$!
	while [[ $(ps ux | grep ./power-law | wc -l) -ne 1 ]]; do
		sleep 1
	done
	kill $cpustatPid
	date
done
