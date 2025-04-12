#include "RStarTree.hpp"
#include <random>
#include <iostream>

int main() {
    // Create a new R*-Tree with node capacity = 10 and 2 dimensions
    RStarTree tree(10, 2);

    // Random number generation setup
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 10.0);

    // Generate random rectangles for bulk loading
    vector<Rectangle> rectangles;
    for (int i = 0; i < 100; ++i) {
        float minX = dis(gen);
        float minY = dis(gen);
        float maxX = minX + dis(gen);
        float maxY = minY + dis(gen);
        rectangles.emplace_back(i, vector<float>{minX, minY}, vector<float>{maxX, maxY});
    }
    std::cout << "Bulk loading rectangles..." << std::endl;
    tree.bulkLoad(rectangles);

    // Insert a single rectangle
    std::cout << "Inserting a single rectangle..." << std::endl;
    Rectangle singleRect(101, {3.0f, 3.0f}, {8.0f, 8.0f});
    tree.insert(singleRect);

    // Generate random rectangles for batch insertion
    vector<Rectangle> moreRectangles;
    for (int i = 112; i < 122; ++i) {
        float minX = dis(gen);
        float minY = dis(gen);
        float maxX = minX + dis(gen);
        float maxY = minY + dis(gen);
        moreRectangles.emplace_back(i, vector<float>{minX, minY}, vector<float>{maxX, maxY});
    }
    std::cout << "Batch inserting rectangles..." << std::endl;
    tree.batchInsert(moreRectangles);

    // Perform a range query
    std::cout << "Performing a range query..." << std::endl;
    Rectangle query(23, {0.0f, 0.0f}, {20.0f, 20.0f});
    auto results = tree.rangeQuery(query);

    // Print the results
    for (const auto& res : results) 
        res.printRectangle("Result");
        
    return 0;
}
