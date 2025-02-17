#!/bin/bash
make clean
make -j

export OMP_NUM_THREADS=$2
echo "Using" $OMP_NUM_THREADS "threads"
./chromatic -input ../instances/$1
