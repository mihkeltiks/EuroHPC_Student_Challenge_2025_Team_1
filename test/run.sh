#!/bin/bash

if [ $# -ne 1 ]; then
    echo "Usage: $0 <instance_file>"
    echo "Example: $0 square.clq"
    exit 1
fi

# Compile the test file
g++ -std=c++11 TestMaxClique.cpp -I src/ -o test_max_clique

# Check if compilation was successful
if [ $? -ne 0 ]; then
    echo "Compilation failed!"
    exit 1
fi

# Run the test with the specified instance
./test_max_clique "../maxclique_instances/$1"