#!/bin/bash

for alpha in 0.3 0.6 1.0 1.2 1.4 1.6
do
	echo "running alpha $alpha..."
	cargo run --release --bin power-law $alpha > alpha_in/alpha"$alpha".in
	./power-law < alpha"$alpha".in > alpha_out/alpha"$alpha".out
	if [[ $? -ne 0 ]]; then
		echo "Error on alpha $alpha"
		exit 1
	fi
done
