# Compile
g++ -std=c++17 -o main.exe main.cpp

# Check if compilation was successful
if [ $? -ne 0 ]; then
    echo "Compilation failed!"
    exit 1
fi

# Run linear scan validation
# Parameters:
# -n 10000: Number of data points (10,000)
# -q 1000: Number of queries (1,000)
# -c 128: Node capacity (128)
# -d 2: Dimensions (2)
# -v: Enable validation with linear scan

echo "Running R*-Tree with linear scan comparison..."
./main.exe -n 10000 -q 1000 -c 128 -d 2 -v
