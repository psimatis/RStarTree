#include "RStarTree.h"
#include <iostream>

int main() {
    RStarTree tree(4, 2);

    // Insert rectangles
    tree.insert(Rectangle({0, 0}, {1, 1}));
    tree.insert(Rectangle({2, 2}, {3, 3}));
    tree.insert(Rectangle({4, 4}, {5, 5}));
    tree.insert(Rectangle({6, 6}, {7, 7}));
    tree.insert(Rectangle({8, 8}, {9, 9}));

    // Print tree structure
    std::cout << "R*-Tree Structure:\n";
    tree.printTree();

    // Perform range query
    Rectangle query({1, 1}, {5, 5});
    std::cout << "\nPerforming Range Query with Rectangle: [(1, 1), (5, 5)]\n";

    auto results = tree.rangeQuery(query);
    std::cout << "results size: " << results.size() << std::endl;
    for (const auto& rect : results) {
        std::cout << "Found Rectangle: [(";
        for (float val : rect.minCoords) std::cout << val << " ";
        std::cout << "), (";
        for (float val : rect.maxCoords) std::cout << val << " ";
        std::cout << ")]\n";
    }

    return 0;
}
