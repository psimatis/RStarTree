#include "RStarTree.h"

int main() {
    // Create a new R*-Tree with node capacity = 10 and 2 dimensions
    RStarTree tree(10, 2);

    // Insert a rectangle
    Rectangle rect({0.0f, 0.0f}, {5.0f, 5.0f});
    tree.insert(rect);

    // Perform a range query
    Rectangle query({1.0f, 1.0f}, {6.0f, 6.0f});
    auto results = tree.rangeQuery(query);

    // Print the results
    for (const auto& res : results) 
        res.printRectangle("Result");
        
    return 0;
}
