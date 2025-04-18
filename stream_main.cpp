/*
=====================================================================
R*-Tree Demo: Stream Data Processing
=====================================================================

What does it do?
    - Reads data from a stream file (WILDFIRES.stream)
    - Only processes rows that start with 'E'
    - Uses the 2nd column as ID and the 3rd and 4th columns as coordinates
    - Performs the same tests as the original main.cpp:
      1. Single Insertions
      2. Batch Insertions
      3. Bulk Loading
    - Validates range queries results against a linear scan
    - Calculates performance metrics and tree statistics

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
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <set>
#include <limits>

using namespace chrono;
using namespace std;

void parseArguments(int argc, char* argv[], int& numData, int& numQueries, int& dimension, int& capacity, bool& validateResults, string& streamFile) {
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
        } else if (arg == "-s" || arg == "--stream") {
            if (i + 1 < argc) streamFile = argv[++i];
        } else {
            cout << "Usage: " << argv[0] << " [options]\n";
            cout << "Options:\n";
            cout << "  -n, --numData <num>       Number of data points to insert (default: 10000)\n";
            cout << "  -q, --numQueries <num>    Number of range queries to perform (default: 1000)\n";
            cout << "  -d, --dimension <num>     Dimensionality of the data (default: 2)\n";
            cout << "  -c, --capacity <num>      Node capacity of the R*-Tree (default: 128)\n";
            cout << "  -v, --validate            Enable brute-force validation (default: off)\n";
            cout << "  -s, --stream <file>       Stream file to read (default: streams/WILDFIRES.stream)\n";
            exit(0);
        } 
    }
}

vector<Rectangle> readStreamFile(const string& filename) {
    vector<Rectangle> dataPoints;
    ifstream file(filename);
    
    if (!file.is_open()) {
        cerr << "Error: Could not open file " << filename << endl;
        return dataPoints;
    }
    
    string line;
    char type;
    long id, x, y, dummy;
    
    while (getline(file, line)) {
        istringstream iss(line);
        iss >> type >> id >> x >> y >> dummy;
        
        // Only process rows that start with 'E'
        if (type == 'E') {
            // Create a point rectangle with the coordinates
            Rectangle rect(id, {static_cast<float>(x), static_cast<float>(y)}, {static_cast<float>(x), static_cast<float>(y)});
            dataPoints.push_back(rect);
        }
    }
    
    file.close();
    cout << "Read " << dataPoints.size() << " points from " << filename << endl;
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
        if (query.overlapCheck(point)) {
            results.push_back(point);
        }
    }
    return results;
}

void performQueries(RStarTree& tree, const vector<Rectangle>& dataPoints, int numQueries, int maxRange, bool validateResults) {
    long long totalTreeQueryTime = 0;
    long long linearScanQueryTime = 0;
    bool allQueriesMatch = true;
    
    // Find the actual range of the data to create appropriate queries
    float minX = std::numeric_limits<float>::max();
    float minY = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::min();
    float maxY = std::numeric_limits<float>::min();
    
    for (const auto& point : dataPoints) {
        minX = min(minX, point.minCoords[0]);
        minY = min(minY, point.minCoords[1]);
        maxX = max(maxX, point.maxCoords[0]);
        maxY = max(maxY, point.maxCoords[1]);
    }
    
    cout << "Data range: [" << minX << ", " << minY << "] to [" << maxX << ", " << maxY << "]" << endl;
    
    // Adjust the range to the actual data
    float rangeX = maxX - minX;
    float rangeY = maxY - minY;
    
    // Create a density map to identify areas with data points
    const int GRID_SIZE = 10;
    vector<vector<int>> densityMap(GRID_SIZE, vector<int>(GRID_SIZE, 0));
    
    for (const auto& point : dataPoints) {
        int gridX = min(GRID_SIZE-1, max(0, static_cast<int>((point.minCoords[0] - minX) / rangeX * GRID_SIZE)));
        int gridY = min(GRID_SIZE-1, max(0, static_cast<int>((point.minCoords[1] - minY) / rangeY * GRID_SIZE)));
        densityMap[gridX][gridY]++;
    }
    
    // Find cells with data
    vector<pair<int, int>> cellsWithData;
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            if (densityMap[i][j] > 0) {
                cellsWithData.push_back({i, j});
            }
        }
    }
    
    cout << "Found " << cellsWithData.size() << " grid cells with data points" << endl;
    
    for (int i = 0; i < numQueries; ++i) {
        float queryMinX, queryMinY, queryMaxX, queryMaxY;
        
        if (!cellsWithData.empty() && (rand() % 100) < 80) {
            // 80% of the time, pick a cell with data
            int cellIdx = rand() % cellsWithData.size();
            int gridX = cellsWithData[cellIdx].first;
            int gridY = cellsWithData[cellIdx].second;
            
            // Convert grid cell to coordinate range
            queryMinX = minX + (gridX * rangeX / GRID_SIZE);
            queryMinY = minY + (gridY * rangeY / GRID_SIZE);
            
            // Make the query window cover the entire cell plus some neighboring area
            queryMaxX = queryMinX + (rangeX / GRID_SIZE) * (1 + (rand() % 3));
            queryMaxY = queryMinY + (rangeY / GRID_SIZE) * (1 + (rand() % 3));
        } else {
            // 20% of the time, use a random query
            queryMinX = minX + static_cast<float>(rand() % static_cast<int>(rangeX));
            queryMinY = minY + static_cast<float>(rand() % static_cast<int>(rangeY));
            // Make the query window larger (about 10-20% of the data range)
            queryMaxX = queryMinX + static_cast<float>(rand() % static_cast<int>(rangeX * 0.2 + 1));
            queryMaxY = queryMinY + static_cast<float>(rand() % static_cast<int>(rangeY * 0.2 + 1));
        }
        
        Rectangle query(-1, {queryMinX, queryMinY}, {queryMaxX, queryMaxY});

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

        // cout << linearScanResults.size() << " | " << rtreeResults.size() << endl;
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
    int spaceMax = 100000;
    string streamFile = "streams/WILDFIRES.stream";

    parseArguments(argc, argv, numData, numQueries, dimension, capacity, validateResults, streamFile);

    // Read data from the specified stream file
    vector<Rectangle> dataPoints = readStreamFile(streamFile);
    
    // Update numData based on actual data points read
    numData = dataPoints.size();
    
    if (numData == 0) {
        cerr << "No data points were read from the file. Exiting." << endl;
        return 1;
    }

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
