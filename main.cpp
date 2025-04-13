/*
=====================================================================
R*-Tree Demo: Benchmark insertion methods
=====================================================================

What does it test?
 Benchmark and validate the insertion methods:
    1. Bulk Loading.
    2. Single Insertions.
    3. Batch Insertions.

What does it do?
    - Validates range queries results against a linear scan.
    - Calculates performance metrics (e.g., insertion time).
    - Calculates tree statistics (e.g.,size in MB).

Command-line arguments:
    - `-n` / `--numData`: Number of data points (default: 10000).
    - `-q` / `--numQueries`: Number of queries (default: 1000).
    - `-d` / `--dimension`: Data dimensionality (default: 2).
    - `-c` / `--capacity`: Node capacity (default: 128).
    - `-v` / `--validate`: Validate query results (default: off).
=====================================================================
 */

#include "RStarTree.hpp"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <set>

using namespace chrono;

void parseArguments(int argc, char* argv[], int& numData, int& numQueries, int& dimension, int& capacity, bool& validateResults) {
    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        if (arg == "-n" || arg == "--numData") {
            if (i + 1 < argc) numData = atoi(argv[++i]);
        } else if (arg == "-q" || arg == "--numQueries") {
            if (i + 1 < argc) numQueries = atoi(argv[++i]);
        } else if (arg == "-d" || arg == "--dimension") {
            if (i + 1 < argc) dimension = atoi(argv[++i]);
        } else if (arg == "-c" || arg == "--capacity") {
            if (i + 1 < argc) capacity = atoi(argv[++i]);
        } else if (arg == "-v" || arg == "--validate") {
            validateResults = true;
        } else {
            cout << "Usage: " << argv[0] << " [options]\n";
            cout << "Options:\n";
            cout << "  -n, --numData <num>       Number of data points to insert (default: 10000)\n";
            cout << "  -q, --numQueries <num>    Number of range queries to perform (default: 1000)\n";
            cout << "  -d, --dimension <num>     Dimensionality of the data (default: 2)\n";
            cout << "  -c, --capacity <num>      Node capacity of the R*-Tree (default: 128)\n";
            cout << "  -v, --validate            Enable brute-force validation (default: off)\n";
            exit(0);
        } 
    }
}

vector<Rectangle> generateRandomData(int numData, int minRange, int maxRange) {
    vector<Rectangle> dataPoints;
    for (int i = 0; i < numData; ++i) {
        float x1 = static_cast<float>(minRange + rand() % (maxRange - minRange + 1));
        float y1 = static_cast<float>(minRange + rand() % (maxRange - minRange + 1));
        Rectangle rect(i, {x1, y1}, {x1, y1});
        dataPoints.push_back(rect);
    }
    return dataPoints;
}

void insert(RStarTree& tree, const vector<Rectangle>& dataPoints) {
    auto start = high_resolution_clock::now();
    for (const auto& rect : dataPoints)
        tree.insert(rect);
    auto duration = duration_cast<milliseconds>(high_resolution_clock::now() - start);
    cout << "Insertion time: " << duration.count() / 1000.0 << " s" << endl;
}

void insertBatches(RStarTree& tree, vector<Rectangle>& dataPoints, int capacity) {
    auto start = high_resolution_clock::now();
    tree.batchInsert(dataPoints);
    auto duration = duration_cast<milliseconds>(high_resolution_clock::now() - start);
    cout << "Insertion time: " << duration.count() / 1000.0 << " s" << endl;
}

void insertBulkLoad(RStarTree& tree, vector<Rectangle>& dataPoints) {
    auto start = high_resolution_clock::now();
    tree.bulkLoad(dataPoints); 
    auto duration = duration_cast<milliseconds>(high_resolution_clock::now() - start);
    cout << "Insertion time: " << duration.count() / 1000.0 << " s" << endl;
}

vector<Rectangle> linearScanQuery(const vector<Rectangle>& points, const Rectangle& query) {
    vector<Rectangle> results;

    for (const auto& point : points) {
        if (query.overlapCheck(point))
            results.push_back(point);
    }
    return results;
}

void performQueries(RStarTree& tree, const vector<Rectangle>& dataPoints, int numQueries, int maxRange, bool validateResults) {
    bool allQueriesMatch = true;
    auto totalTreeQueryTime = 0.0, linearScanQueryTime = 0.0;

    for (int i = 0; i < numQueries; ++i) {
        float queryMinX = static_cast<float>(rand() % maxRange);
        float queryMinY = static_cast<float>(rand() % maxRange);
        float queryMaxX = queryMinX + static_cast<float>(rand() % 100 + 1);
        float queryMaxY = queryMinY + static_cast<float>(rand() % 100 + 1);
        Rectangle query(i, {queryMinX, queryMinY}, {queryMaxX, queryMaxY});

        auto start = high_resolution_clock::now();
        auto rtreeResults = tree.rangeQuery(query);
        auto treeQueryDuration = duration_cast<microseconds>(high_resolution_clock::now() - start).count();
        totalTreeQueryTime += treeQueryDuration;

        if (!validateResults) continue;

        start = high_resolution_clock::now();
        auto linearScanResults = linearScanQuery(dataPoints, query);
        auto linearScanDuration = duration_cast<microseconds>(high_resolution_clock::now() - start).count();
        linearScanQueryTime += linearScanDuration;

        if (rtreeResults.size() != linearScanResults.size()) {
            allQueriesMatch = false;
            cout << "Query Range: [(" << queryMinX << ", " << queryMinY << "), (" << queryMaxX << ", " << queryMaxY << ")]\n";
            cout << "R*tree results count: " << rtreeResults.size() << " | Linear scan results count: " << linearScanResults.size() << endl;

            set<pair<pair<float, float>, pair<float, float>>> rtreeSet, bruteSet;
            for (const auto& rect : rtreeResults) rtreeSet.insert({{rect.minCoords[0], rect.minCoords[1]}, {rect.maxCoords[0], rect.maxCoords[1]}});
            for (const auto& rect : linearScanResults) bruteSet.insert({{rect.minCoords[0], rect.minCoords[1]}, {rect.maxCoords[0], rect.maxCoords[1]}});

            cout << "Results in R*-Tree but not in linear scan:\n";
            for (const auto& rect : rtreeSet) {
                if (bruteSet.find(rect) == bruteSet.end()) 
                    cout << "[(" << rect.first.first << ", " << rect.first.second << "), (" << rect.second.first << ", " << rect.second.second << ")]\n";
            }

            cout << "Results in linear scan but not in R*-Tree:\n";
            for (const auto& rect : bruteSet) {
                if (rtreeSet.find(rect) == rtreeSet.end()) 
                    cout << "[(" << rect.first.first << ", " << rect.first.second << "), (" << rect.second.first << ", " << rect.second.second << ")]\n";
            }
            break;
        }
    }

    cout << "Number of queries: " << numQueries << endl;

    if (validateResults) {
        cout << (allQueriesMatch ? "All queries matched!" : "Some queries did not match!") << endl;
        cout << "Total linear scan query time: " << linearScanQueryTime / 1000000 << " s" << endl;
    }
    cout << "Total R*Tree query time: " << totalTreeQueryTime / 1000000 << "s" << endl;
}

void report(RStarTree& tree){
    cout << "Tree info" << endl;
    cout << "   Dimension: " << tree.dimensions << endl;
    cout << "   Capacity: " << tree.maxEntries << endl;
    cout << "   Min capacity: " << tree.minEntries << endl << endl;
    cout << "   Size in MB: " << tree.calculateSizeInMB() << endl << endl;
    cout << "-------------------------" << endl << endl;
}

int main(int argc, char* argv[]) {
    srand(0);

    int dimension = 2;
    int capacity = 128;
    int numData = 10000;
    int numQueries = 1000;
    bool validateResults = false;
    int spaceMin = 0;
    int spaceMax = 100000;

    parseArguments(argc, argv, numData, numQueries, dimension, capacity, validateResults);

    vector<Rectangle> dataPoints = generateRandomData(numData, spaceMin, spaceMax);

    cout << "*Test: Insertion*" << endl;
    RStarTree treeOneByOne(capacity, dimension);
    insert(treeOneByOne, dataPoints);
    performQueries(treeOneByOne, dataPoints, numQueries, spaceMax, validateResults);
    report(treeOneByOne);

    cout << "*Test: Batch insertion*" << endl;
    RStarTree treeBatch(capacity, dimension);
    insertBatches(treeBatch, dataPoints, capacity);
    performQueries(treeBatch, dataPoints, numQueries, spaceMax, validateResults);
    report(treeBatch);

    cout << "*Test: Bulk loading*" << endl;
    RStarTree treeBulk(capacity, dimension);
    insertBulkLoad(treeBulk, dataPoints);
    performQueries(treeBulk, dataPoints, numQueries, spaceMax, validateResults);
    report(treeBulk);

    cout << endl << "Benchmark completed." << endl << endl;
    return 0;
}
