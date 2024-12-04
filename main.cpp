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

    return 0;
}
