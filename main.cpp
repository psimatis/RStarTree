#include "RStarTree.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <chrono>

using namespace std::chrono;


int main() {
    int dimension = 2;
    int capacity = 100;
    int numData = 100;

    RStarTree tree(capacity, dimension);

    srand(0);

    // Insert points
    const int minRange = 0;
    const int maxRange = 100; 
    auto start = high_resolution_clock::now();
    for (int i = 0; i < numData; ++i) {
        float x1 = static_cast<float>(minRange + rand() % (maxRange - minRange + 1));
        float y1 = static_cast<float>(minRange + rand() % (maxRange - minRange + 1));
        tree.insert(Rectangle({x1, y1}, {x1, y1}));
        // cout << "Inserted Rectangle: [(" << x1 << ", " << y1 << "), (" << x1 << ", " << y1 << ")]" << endl;
    }
    auto durationInsert = duration_cast<seconds>(high_resolution_clock::now() - start);
    cout << "Insertion of " << numData << " rectangles took: " << durationInsert.count() << "s" << endl;


    // Print tree structure
    cout << "R*-Tree Structure:\n";
    tree.printTree();

    // Perform range query
    start = high_resolution_clock::now();
    Rectangle query({static_cast<float>(minRange), static_cast<float>(minRange)}, {static_cast<float>(minRange + 10), static_cast<float>(minRange + 10)});
    auto durationQuery = duration_cast<milliseconds>(high_resolution_clock::now() - start);
    cout << "Range query took: " << durationQuery.count() << " ms" << endl;
    cout << "\nPerforming Range Query: [(" << minRange << ", " << minRange << "), (" << minRange + 10 << ", " << minRange + 10 << ")]\n";

    auto results = tree.rangeQuery(query);
    cout << "Results size: " << results.size() << endl << endl;

    for (const auto& rect : results) {
        cout << "Found Rectangle: [(";
        for (float val : rect.minCoords) cout << val << " ";
        cout << "), (";
        for (float val : rect.maxCoords) cout << val << " ";
        cout << ")]\n";
    }

    TreeStats stats = tree.getStats();
    cout << "Tree Statistics" << endl;
    cout << "Tree Height: " << stats.height << endl;
    cout << "Total Data Entries: " << stats.totalDataEntries << endl << endl;
    cout << "Total Nodes: " << stats.totalNodes << endl;
    cout << "Leaf Nodes: " << stats.leafNodes << endl;
    cout << "Internal Nodes: " << stats.internalNodes << endl << endl;
    cout << "Total Node Visits: " << stats.totalNodeVisits << endl;
    cout << "Leaf Node Visits: " << stats.leafNodeVisits << endl;
    cout << "Internal Node Visits: " << stats.internalNodeVisits << endl;

    return 0;
}
