#include "RStarTree.h"
#include <iostream>
#include <cstdlib>
#include <ctime>

int main() {
    int dimension = 2;
    int capacity = 200;

    RStarTree tree(capacity, dimension);

    // srand(time(nullptr));
    srand(0);

    // // Insert rectangles
    // for (int i = 0; i < 10; ++i) {
    //     float x1 = static_cast<float>(rand() % 100) / 10.0f;
    //     float y1 = static_cast<float>(rand() % 100) / 10.0f;
    //     float x2 = x1 + static_cast<float>(rand() % 50) / 10.0f;
    //     float y2 = y1 + static_cast<float>(rand() % 50) / 10.0f;

    //     tree.insert(Rectangle({x1, y1}, {x2, y2}));
    // }

    // Insert randomly generated points
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
    Rectangle query({0, 0}, {8, 8});
    cout << "\nPerforming Range Query: [(0, 0), (8, 8)]\n";

    auto results = tree.rangeQuery(query);
    cout << "Results size: " << results.size() << endl;

    // for (const auto& rect : results) {
    //     cout << "Found Rectangle: [(";
    //     for (float val : rect.minCoords) cout << val << " ";
    //     cout << "), (";
    //     for (float val : rect.maxCoords) cout << val << " ";
    //     cout << ")]\n";
    // }

    TreeStats stats = tree.getStats();
    cout << "\nTree Statistics:\n";
    cout << "Total Nodes: " << stats.totalNodes << "\n";
    cout << "Leaf Nodes: " << stats.leafNodes << "\n";
    cout << "Internal Nodes: " << stats.internalNodes << "\n";
    cout << "Total Data Entries: " << stats.totalDataEntries << "\n";
    cout << "Tree Height: " << stats.height << "\n";

    return 0;
}
