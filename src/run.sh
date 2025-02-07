#!/bin/bash
make clean
make -j
./chromatic -input ../instances/myciel3.col
