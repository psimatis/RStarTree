#include "RStarTree.h"
#include <iostream>
#include <cstdlib>
#include <ctime>


int main() {
    int dimension = 2;
    int capacity = 256;

    RStarTree tree(capacity, dimension);

    srand(0);

    // Insert points
    for (int i = 0; i < 100000; ++i) {
        float x1 = static_cast<float>(rand() % 10);
        float y1 = static_cast<float>(rand() % 10);
        tree.insert(Rectangle({x1, y1}, {x1, y1}));
        // cout << "Inserted Rectangle: [(" << x1 << ", " << y1 << "), (" << x1 << ", " << y1 << ")]" << endl;
    }

    // Print tree structure
    // cout << "R*-Tree Structure:\n";
    // tree.printTree();

    // Perform range query
    Rectangle query({0, 0}, {1, 1});
    cout << "\nPerforming Range Query: [(0, 0), (1, 1)]\n";

    auto results = tree.rangeQuery(query);
    cout << "Results size: " << results.size() << endl << endl;

    // for (const auto& rect : results) {
    //     cout << "Found Rectangle: [(";
    //     for (float val : rect.minCoords) cout << val << " ";
    //     cout << "), (";
    //     for (float val : rect.maxCoords) cout << val << " ";
    //     cout << ")]\n";
    // }

    TreeStats stats = tree.getStats();
    cout << "Tree Statistics" << endl;
    cout << "Tree Height: " << stats.height << endl;
    cout << "Total Data Entries: " << stats.totalDataEntries << endl;
    cout << "Total Nodes: " << stats.totalNodes << endl;
    cout << "Leaf Nodes: " << stats.leafNodes << endl;
    cout << "Internal Nodes: " << stats.internalNodes << endl;
    cout << "Total Node Visits: " << stats.totalNodeVisits << endl;
    cout << "Leaf Node Visits: " << stats.leafNodeVisits << endl;
    cout << "Internal Node Visits: " << stats.internalNodeVisits << endl;

    return 0;
}
