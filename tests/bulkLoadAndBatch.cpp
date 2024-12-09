/*
===============================================================
R*-Tree Demo: Bulk Load and Batch Insert
===============================================================

What does it test?
    That the tree entries equal the number of inserted points.

What does it do?
    1. Bulk loading an R*-Tree with random points.
    2. Inserting additional points in batches.

Parameters:
    - `dimensions`: Data dimensionality.
    - `capacity`: Max entries per node.
    - `numBulkLoadPoints`: Points for bulk loading.
    - `numBatchPoints`: Points for batch insertions.
    - `rangeMax`: Max range for coordinates.
===============================================================
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
        vector<float> coords(dimensions);

        for (int d = 0; d < dimensions; ++d) {
            int val = rangeMin + rand() % (rangeMax - rangeMin + 1); 
            coords[d] = static_cast<float>(val);
        }

        Rectangle rect(i, coords, coords);
        rectangles.emplace_back(rect);
    }
    return rectangles;
}

int main() {
    srand(0);

    const int dimensions = 4;
    const int capacity = 128;
    const int numBulkLoadPoints = 10000;
    const int numBatchPoints = 1000;
    const int rangeMax = 100;

    cout << "Bulk loading..." << endl;
    vector<Rectangle> bulkLoadData = generateRandomRectangles(numBulkLoadPoints, dimensions, 0, rangeMax);
    RStarTree tree(capacity, dimensions);
    tree.bulkLoad(bulkLoadData);

    cout << "Batch inserting..." << endl;
    vector<Rectangle> batchData = generateRandomRectangles(numBatchPoints, dimensions, 0, rangeMax);
        tree.batchInsert(batchData);

    if (tree.getInfo().totalDataEntries != numBulkLoadPoints + numBatchPoints) {
        cout << "Error: totalDataEntries does not match expected value" << endl;
        return 1;
    }
    cout << endl << "The tree was built successfully." << endl << endl;

    return 0;
}
