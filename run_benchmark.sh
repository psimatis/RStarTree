#!/bin/bash

# Compile the benchmark
g++ -std=c++17 -o benchmark tests/benchmark.cpp

# Check if compilation was successful
if [ $? -ne 0 ]; then
    echo "Compilation failed!"
    exit 1
fi

# Run the benchmark with linear scan validation
# Parameters:
# -n 10000: Number of data points (10,000)
# -q 1000: Number of queries (1,000)
# -c 128: Node capacity (128)
# -d 2: Dimensions (2)
# -v: Enable validation with linear scan

echo "Running R*-Tree benchmark with linear scan comparison..."
./benchmark -n 10000 -q 1000 -c 128 -d 2 -v

# You can modify the parameters above to test different configurations
# For example, to test with more data points:
# ./benchmark -n 100000 -q 1000 -c 128 -d 2 -v
