/*
========================================================
R*-Tree Demo: Mix all insertion methods
========================================================

What does it test?
Validate the tree created with a mixed strategy of:
    1. Bulk Loading.
    2. Single Insertions.
    3. Batch Insertions.

What does it do?
    1. Bulk loads the tree with random points.
    2. Performs R* insertions.
    3. Performs batch insertions.
    4. Cross-checks query results with linear scan.
========================================================
*/

#include "RStarTree.h"
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>

using namespace std;

vector<Rectangle> generateRandomRectangles(int num, int dimensions, int rangeMin, int rangeMax) {
    vector<Rectangle> rectangles;
    for (int i = 0; i < num; ++i) {
        vector<float> minCoords(dimensions), maxCoords(dimensions);
        for (int d = 0; d < dimensions; ++d) {
            float minVal = rangeMin + static_cast<float>(rand()) / RAND_MAX * (rangeMax - rangeMin);
            float maxVal = minVal + static_cast<float>(rand()) / RAND_MAX * 10; // Ensure max > min
            minCoords[d] = minVal;
            maxCoords[d] = maxVal;
        }
        rectangles.emplace_back(i, minCoords, maxCoords);
    }
    return rectangles;
}

vector<Rectangle> linearScanQuery(const vector<Rectangle>& data, const Rectangle& query) {
    vector<Rectangle> results;
    for (const auto& rect : data) {
        if (query.overlapCheck(rect))
            results.push_back(rect);
    }
    return results;
}

int main() {
    srand(0);

    const int dimensions = 3;
    const int capacity = 128;
    const int numBulkLoadPoints = 10000;
    const int numSingleInsertions = 100;
    const int numBatchInsertions = 10;
    const int numQueries = 10000;

    vector<Rectangle> bulkLoadData = generateRandomRectangles(numBulkLoadPoints, dimensions, 0, 100);
    vector<Rectangle> singleInsertionData = generateRandomRectangles(numSingleInsertions, dimensions, 0, 100);

    cout << "Bulk loading..." << endl;
    RStarTree tree(capacity, dimensions);
    tree.bulkLoad(bulkLoadData);

    cout << "Performing insertions..." << endl;
    for (const auto& rect : singleInsertionData)
        tree.insert(rect);

    cout << "Performing batch insertions..." << endl;
    vector<vector<Rectangle>> allBatchData;
    for (int i = 0; i < numBatchInsertions; ++i) {
        vector<Rectangle> batchData = generateRandomRectangles(capacity, dimensions, 0, 100);
        tree.batchInsert(batchData);
        allBatchData.push_back(batchData);
    }

    // Merge all data points and validate
    vector<Rectangle> allData = bulkLoadData;
    allData.insert(allData.end(), singleInsertionData.begin(), singleInsertionData.end());
    for (const auto& batch : allBatchData) 
        allData.insert(allData.end(), batch.begin(), batch.end());

    cout << "Validating range queries..." << endl;
    for (int i = 0; i < numQueries; ++i) {
        Rectangle query = generateRandomRectangles(1, dimensions, 0, 100).front();

        vector<Rectangle> rtreeResults = tree.rangeQuery(query);
        vector<Rectangle> linearResults = linearScanQuery(allData, query);

        if (rtreeResults.size() != linearResults.size()) {
            cout << "Query " << (i + 1) << " does not match." << endl;
            return 1;
        }
    }

    TreeInfo stats = tree.getInfo();
    cout << endl << "Tree info" << endl;
    cout << "   Dimension: " << stats.dimensions << endl;
    cout << "   Capacity: " << stats.capacity << endl;
    cout << "   Min capacity: " << stats.minCapacity << endl << endl;

    cout << "   Height: " << stats.height << endl;
    cout << "   Total nodes: " << stats.totalNodes << endl;
    cout << "   Data Entries: " << stats.totalDataEntries << endl;

    cout << endl << "All " << numQueries << " queries matched!" << endl << endl;
    return 0;
}
