#include "RStarTree.h"
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

using namespace std;

void parseArguments(int argc, char* argv[], int& numData, int& dimensions, int& capacity) {
    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        if (arg == "-n" && i + 1 < argc) numData = atoi(argv[++i]);
        else if (arg == "-d" && i + 1 < argc) dimensions = atoi(argv[++i]);
        else if (arg == "-c" && i + 1 < argc) capacity = atoi(argv[++i]);
        else {
            cerr << "Usage: " << argv[0] << " [options]\n";
            cerr << "Options:\n";
            cerr << "  -n <num>   Number of data points (default: 1000)\n";
            cerr << "  -d <dim>   Dimensionality (default: 2)\n";
            cerr << "  -c <cap>   Node capacity (default: 128)\n";
            cerr << "Example: " << endl;
            cerr << "./rstartree.exe -n 1000 -d 2 -c 128" << endl << endl;
            exit(1);
        }
    }
}

vector<Rectangle> generateRandomRectangles(int num, int dimensions, int rangeMin, int rangeMax) {
    vector<Rectangle> rectangles;
    for (int i = 0; i < num; ++i) {
        vector<float> minCoords(dimensions), maxCoords(dimensions);
        for (int d = 0; d < dimensions; ++d) {
            float minVal = rangeMin + static_cast<float>(rand()) / RAND_MAX * (rangeMax - rangeMin);
            float maxVal = minVal + static_cast<float>(rand()) / RAND_MAX * 10;
            minCoords[d] = minVal;
            maxCoords[d] = maxVal;
        }
        rectangles.emplace_back(minCoords, maxCoords);
    }
    return rectangles;
}

int main(int argc, char* argv[]) {
    srand(0);

    // Default parameters
    int numData = 1000;
    int dimensions = 2;
    int capacity = 128;

    // Parse command-line arguments
    parseArguments(argc, argv, numData, dimensions, capacity);

    // Generate random data
    vector<Rectangle> dataPoints = generateRandomRectangles(numData, dimensions, 0, 100);

    // Create the R*-Tree
    cout << "Creating R*-Tree with capacity " << capacity << " and " << dimensions << " dimensions " << endl;
    RStarTree tree(capacity, dimensions);

    // Insert data
    cout << "Inserting " << numData << " data points" << endl;
    for (const auto& rect : dataPoints)
        tree.insert(rect);

    // Perform a range query
    cout << "Performing a range query" << endl;
    vector<float> queryMin(dimensions, 10); 
    vector<float> queryMax(dimensions, 50);
    Rectangle query(queryMin, queryMax);    
    vector<Rectangle> results = tree.rangeQuery(query);

    // Display tree statistics
    TreeInfo stats = tree.getStats();
    cout << "Tree info:" << endl;
    cout << "  Height: " << stats.height << "\n";
    cout << "  Total nodes: " << stats.totalNodes << "\n";
    cout << "  Total objects: " << stats.totalDataEntries << "\n";
    cout << "  Tree size: " << stats.sizeInMB << " MB\n";

    return 0;
}
