echo "========================================="
echo "Running test: bulkLoadAndBatch.cpp"
echo "========================================="
g++ bulkLoadAndBatch.cpp ../RStarTree.cpp -o bulkLoadAndBatch.exe -I../
./bulkLoadAndBatch.exe

echo "========================================="
echo "Running test: mixStrategy.cpp"
echo "========================================="
g++ mixStrategy.cpp ../RStarTree.cpp -o mixStrategy.exe -I../
./mixStrategy.exe

echo "========================================="
echo "Running test: benchmark.cpp"
echo "========================================="
g++ benchmark.cpp ../RStarTree.cpp -o benchmark.exe -I../
./benchmark.exe

rm -f *.exe