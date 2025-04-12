#include "RStarTree.hpp"
#include <random>
#include <iostream>

int main() {
    // Create a new R*-Tree with node capacity = 10 and 2 dimensions
    RStarTree tree(10, 2);

    // Random number generation setup
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> dis(0.0, 10.0);

    // Generate random rectangles for bulk loading
    vector<Rectangle> rectangles;
    for (int i = 0; i < 100; ++i) {
        float minX = dis(gen);
        float minY = dis(gen);
        float maxX = minX + dis(gen);
        float maxY = minY + dis(gen);
        rectangles.emplace_back(i, vector<float>{minX, minY}, vector<float>{maxX, maxY});
    }
    cout << "Bulk loading rectangles..." << endl;
    tree.bulkLoad(rectangles);
    
    // Check tree health after bulk loading
    cout << "Checking tree health after bulk loading..." << endl;
    tree.checkHealth();

    // Insert a single rectangle
    cout << "Inserting a single rectangle..." << endl;
    Rectangle singleRect(101, {3.0f, 3.0f}, {8.0f, 8.0f});
    tree.insert(singleRect);
    
    // Check tree health after single insertion
    cout << "Checking tree health after single insertion..." << endl;
    tree.checkHealth();

    // Generate random rectangles for batch insertion
    vector<Rectangle> moreRectangles;
    for (int i = 112; i < 122; ++i) {
        float minX = dis(gen);
        float minY = dis(gen);
        float maxX = minX + dis(gen);
        float maxY = minY + dis(gen);
        moreRectangles.emplace_back(i, vector<float>{minX, minY}, vector<float>{maxX, maxY});
    }
    cout << "Batch inserting rectangles..." << endl;
    tree.batchInsert(moreRectangles);
    
    // Check tree health after batch insertion
    cout << "Checking tree health after batch insertion..." << endl;
    tree.checkHealth();

    // Perform a range query
    cout << "Performing a range query..." << endl;
    Rectangle query(23, {0.0f, 0.0f}, {5.0f, 5.0f});
    auto results = tree.rangeQuery(query);

    // Print the results
    cout << "Range query results: " << results.size() << endl;
    for (const auto& res : results) 
        res.printRectangle("Result");
    
    // Final tree health check
    cout << "Final tree health check..." << endl;
    tree.checkHealth();
        
    return 0;
}
