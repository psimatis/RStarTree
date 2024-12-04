#include "RStarTree.h"
#include <iostream>
#include <cstdlib>
#include <ctime>

int main() {
    RStarTree tree(100, 20);

    // Seed for random number generation
    srand(time(nullptr));

    // // Insert randomly generated rectangles
    // for (int i = 0; i < 10; ++i) {
    //     float x1 = static_cast<float>(rand() % 100) / 10.0f;
    //     float y1 = static_cast<float>(rand() % 100) / 10.0f;
    //     float x2 = x1 + static_cast<float>(rand() % 50) / 10.0f;
    //     float y2 = y1 + static_cast<float>(rand() % 50) / 10.0f;

    //     tree.insert(Rectangle({x1, y1}, {x2, y2}));
    // }

    // Insert randomly generated points
    for (int i = 0; i < 10; ++i) {
        float x1 = static_cast<float>(rand() % 10);
        float y1 = static_cast<float>(rand() % 10);
        tree.insert(Rectangle({x1, y1}, {x1, y1}));
    }

    // Print tree structure
    cout << "R*-Tree Structure:\n";
    tree.printTree();

    // Perform range query
    Rectangle query({0, 0}, {8, 8});
    cout << "\nPerforming Range Query: [(0, 0), (8, 8)]\n";

    auto results = tree.rangeQuery(query);
    cout << "Results size: " << results.size() << endl;

    for (const auto& rect : results) {
        cout << "Found Rectangle: [(";
        for (float val : rect.minCoords) cout << val << " ";
        cout << "), (";
        for (float val : rect.maxCoords) cout << val << " ";
        cout << ")]\n";
    }

    return 0;
}
