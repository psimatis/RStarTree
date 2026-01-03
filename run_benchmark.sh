#!/bin/bash

# Compile with optimizations
echo "Compiling benchmark..."
g++ -std=c++17 -O3 -o benchmark_boost benchmark_boost.cpp

# Check if compilation was successful
if [ $? -ne 0 ]; then
    echo "Compilation failed! Make sure Boost is installed:"
    echo "  Ubuntu/Debian: sudo apt install libboost-dev"
    echo "  Fedora: sudo dnf install boost-devel"
    echo "  macOS: brew install boost"
    exit 1
fi

echo "Compilation successful. Running benchmark..."
echo ""

# Run with default parameters (100k points, 1k queries)
./benchmark_boost "$@"
