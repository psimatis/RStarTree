# Default stream file
STREAM_FILE="streams/WILDFIRES.stream"

# Parse command line arguments to check for a stream file
for arg in "$@"; do
    if [[ "$prev_arg" == "-s" || "$prev_arg" == "--stream" ]]; then
        STREAM_FILE="$arg"
        break
    fi
    prev_arg="$arg"
done

# Display which stream file will be used
echo "Using stream file: $STREAM_FILE"

# Compile the stream_main.cpp file
g++ -o stream_main stream_main.cpp -std=c++11

# Check if compilation was successful
if [ $? -eq 0 ]; then
    echo "Compilation successful. Running stream_main..."
    # Run the executable with all arguments
    ./stream_main "$@"
else
    echo "Compilation failed."
fi
