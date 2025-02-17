#!/bin/bash
make clean
make -j
./chromatic -input ../instances/queen5_5.col
