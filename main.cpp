#include "RStarTree.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <set>

using namespace std::chrono;

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

vector<Rectangle> bruteForceQuery(const vector<Rectangle>& points, const Rectangle& query) {
    vector<Rectangle> results;

    for (const auto& point : points) {
        if (query.overlapCheck(point))
            results.push_back(point);
    }
    return results;
}

vector<Rectangle> generateRandomData(int numData, int minRange, int maxRange) {
    vector<Rectangle> dataPoints;
    for (int i = 0; i < numData; ++i) {
        float x1 = static_cast<float>(minRange + rand() % (maxRange - minRange + 1));
        float y1 = static_cast<float>(minRange + rand() % (maxRange - minRange + 1));
        Rectangle rect({x1, y1}, {x1, y1});
        dataPoints.push_back(rect);
    }
    return dataPoints;
}

void insertPointsIndividually(RStarTree& tree, const vector<Rectangle>& dataPoints) {
    auto start = high_resolution_clock::now();
    for (const auto& rect : dataPoints) {
        tree.insert(rect);
    }
    auto duration = duration_cast<milliseconds>(high_resolution_clock::now() - start);
    cout << "Insertion time (1 by 1): " << duration.count() / 1000.0 << " s" << endl;
}

void insertPointsInBatches(RStarTree& tree, vector<Rectangle>& dataPoints, int capacity) {
    // Sort the data points by the first dimension (minCoords[0])
    sort(dataPoints.begin(), dataPoints.end(), [](const Rectangle& a, const Rectangle& b) {
        return a.minCoords[0] < b.minCoords[0];
    });

    int numBatches = (dataPoints.size() + capacity - 1) / capacity; // Calculate the number of batches
    auto start = high_resolution_clock::now();

    for (int i = 0; i < numBatches; ++i) {
        int startIdx = i * capacity;
        int endIdx = min(static_cast<int>(dataPoints.size()), startIdx + capacity);
        vector<Rectangle> batch(dataPoints.begin() + startIdx, dataPoints.begin() + endIdx);

        Node* batchNode = new Node(batch);
        tree.batchInsert(batchNode);
    }

    auto duration = duration_cast<milliseconds>(high_resolution_clock::now() - start);
    cout << "Insertion time (batches): " << duration.count() / 1000.0 << " s" << endl;
}


void performQueries(RStarTree& tree, const vector<Rectangle>& dataPoints, int numQueries, int maxRange, bool validateResults) {
    bool allQueriesMatch = true;
    auto totalTreeQueryTime = 0.0, totalBruteForceQueryTime = 0.0;

    for (int i = 0; i < numQueries; ++i) {
        float queryMinX = static_cast<float>(rand() % maxRange);
        float queryMinY = static_cast<float>(rand() % maxRange);
        float queryMaxX = queryMinX + static_cast<float>(rand() % 100 + 1);
        float queryMaxY = queryMinY + static_cast<float>(rand() % 100 + 1);
        Rectangle query({queryMinX, queryMinY}, {queryMaxX, queryMaxY});

        auto start = high_resolution_clock::now();
        auto rtreeResults = tree.rangeQuery(query);
        auto treeQueryDuration = duration_cast<microseconds>(high_resolution_clock::now() - start).count();
        totalTreeQueryTime += treeQueryDuration;

        if (!validateResults) continue;

        start = high_resolution_clock::now();
        auto bruteResults = bruteForceQuery(dataPoints, query);
        auto bruteForceDuration = duration_cast<microseconds>(high_resolution_clock::now() - start).count();
        totalBruteForceQueryTime += bruteForceDuration;

        if (rtreeResults.size() != bruteResults.size()) {
            allQueriesMatch = false;
            cout << "Query Range: [(" << queryMinX << ", " << queryMinY << "), (" << queryMaxX << ", " << queryMaxY << ")]\n";
            cout << "rtreeResults.size(): " << rtreeResults.size() << " | bruteResults.size(): " << bruteResults.size() << endl;

            set<pair<pair<float, float>, pair<float, float>>> rtreeSet, bruteSet;
            for (const auto& rect : rtreeResults) rtreeSet.insert({{rect.minCoords[0], rect.minCoords[1]}, {rect.maxCoords[0], rect.maxCoords[1]}});
            for (const auto& rect : bruteResults) bruteSet.insert({{rect.minCoords[0], rect.minCoords[1]}, {rect.maxCoords[0], rect.maxCoords[1]}});

            cout << "Results in R*-Tree but not in brute-force:\n";
            for (const auto& rect : rtreeSet) {
                if (bruteSet.find(rect) == bruteSet.end()) 
                    cout << "[(" << rect.first.first << ", " << rect.first.second << "), (" << rect.second.first << ", " << rect.second.second << ")]\n";
            }

            cout << "Results in brute-force but not in R*-Tree:\n";
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
        cout << "Total Brute force query time: " << totalBruteForceQueryTime / 1000000 << " s" << endl;
    }
    cout << "Total R*Tree query time: " << totalTreeQueryTime / 1000000 << "s" << endl;
}

void report(TreeStats stats){
    cout << "Tree Statistics" << endl;
    cout << "Height: " << stats.height << endl;
    cout << "Total Data Entries: " << stats.totalDataEntries << endl << endl;
    cout << "Total Nodes: " << stats.totalNodes << endl;
    cout << "Leaf Nodes: " << stats.leafNodes << endl;
    cout << "Internal Nodes: " << stats.internalNodes << endl << endl;
    cout << "Total Node Visits: " << stats.totalNodeVisits << endl;
    cout << "Leaf Node Visits: " << stats.leafNodeVisits << endl;
    cout << "Internal Node Visits: " << stats.internalNodeVisits << endl;
}

int main(int argc, char* argv[]) {
    srand(0);

    int dimension = 2;
    int capacity = 128;
    int numData = 10000;
    int numQueries = 1000;
    bool validateResults = false;
    const int spaceMin = 0;
    const int spaceMax = 100000;

    parseArguments(argc, argv, numData, numQueries, dimension, capacity, validateResults);

    vector<Rectangle> dataPoints = generateRandomData(numData, spaceMin, spaceMax);
    // for (auto& rect : dataPoints) {
    //     rect.printRectangle("");
    //     cout << endl;
    // }

    // Insert points one by one into R*-Tree
    cout << "*Testing insertion one by one*" << endl;
    RStarTree treeOneByOne(capacity, dimension);
    insertPointsIndividually(treeOneByOne, dataPoints);
    performQueries(treeOneByOne, dataPoints, numQueries, spaceMax, validateResults);
    // treeOneByOne.printTree();
    report(treeOneByOne.getStats());


    // Insert points in batch into another R*-Tree
    cout << endl << "*Testing batch insertion*" << endl;
    RStarTree treeBatch(capacity, dimension);
    insertPointsInBatches(treeBatch, dataPoints, capacity);
    performQueries(treeBatch, dataPoints, numQueries, spaceMax, validateResults);
    // treeBatch.printTree();  
    report(treeBatch.getStats());

    return 0;
}
