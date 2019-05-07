#!/bin/bash

maxval=1000000
numpoints=1000000
for alpha in 0.3
do
	echo "generating Zipfian distributed input file with alpha=$alpha..."
	cargo run --bin power-law --release -- --alpha $alpha --maxval $maxval --numpoints $numpoints > alpha_in/alpha"$alpha".in
done
