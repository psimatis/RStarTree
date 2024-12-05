#include "RStarTree.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <set>

using namespace std::chrono;

vector<Rectangle> bruteForceQuery(const vector<Rectangle>& points, const Rectangle& query) {
    vector<Rectangle> results;

    for (const auto& point : points) {
        if (query.overlapCheck(point))
            results.push_back(point);
    }
    return results;
}

int main() {
    srand(0);

    int dimension = 2;
    int capacity = 5;
    int numData = 20;
    int numQueries = 100;

    RStarTree tree(capacity, dimension);

    // Insert points
    vector<Rectangle> allPoints;
    const int minRange = 0;
    const int maxRange = 10; 
    auto start = high_resolution_clock::now();
    for (int i = 0; i < numData; ++i) {
        float x1 = static_cast<float>(minRange + rand() % (maxRange - minRange + 1));
        float y1 = static_cast<float>(minRange + rand() % (maxRange - minRange + 1));
        Rectangle rect({x1, y1}, {x1, y1});
        tree.insert(rect);
        // cout << "Inserted Rectangle: [(" << x1 << ", " << y1 << "), (" << x1 << ", " << y1 << ")]" << endl;
        allPoints.push_back(rect);
    }
    auto durationInsert = duration_cast<seconds>(high_resolution_clock::now() - start);
    cout << "Insertion of " << numData << " rectangles took: " << durationInsert.count() << "s" << endl;


    // Print tree structure
    cout << "R*-Tree Structure:\n";
    tree.printTree();

    // Query
    bool allQueriesMatch = true;
    float treeQueryTime = 0, bruteForceQueryTime = 0;

    for (int i = 0; i < numQueries; ++i) {
        float queryMinX = static_cast<float>(rand() % maxRange);
        float queryMinY = static_cast<float>(rand() % maxRange);
        float queryMaxX = queryMinX + static_cast<float>(rand() % 10 + 1);
        float queryMaxY = queryMinY + static_cast<float>(rand() % 10 + 1);
        Rectangle query({queryMinX, queryMinY}, {queryMaxX, queryMaxY});

        start = high_resolution_clock::now();
        auto rtreeResults = tree.rangeQuery(query);
        treeQueryTime += duration_cast<milliseconds>(high_resolution_clock::now() - start).count();

        start = high_resolution_clock::now();
        auto bruteResults = bruteForceQuery(allPoints, query);
        bruteForceQueryTime += duration_cast<milliseconds>(high_resolution_clock::now() - start).count();

        if (rtreeResults.size() != bruteResults.size()){
            allQueriesMatch = false;
            cout << "rtreeResults.size(): " << rtreeResults.size() << endl;
            cout << "bruteResults.size(): " << bruteResults.size() << endl;
            cout << "Query Range: [(" << queryMinX << ", " << queryMinY << "), (" << queryMaxX << ", " << queryMaxY << ")]\n";

            // Identify mismatched results
            set<pair<pair<float, float>, pair<float, float>>> rtreeSet, bruteSet;

            for (const auto& rect : rtreeResults) {
                rtreeSet.insert({{rect.minCoords[0], rect.minCoords[1]}, {rect.maxCoords[0], rect.maxCoords[1]}});
            }

            for (const auto& rect : bruteResults) {
                bruteSet.insert({{rect.minCoords[0], rect.minCoords[1]}, {rect.maxCoords[0], rect.maxCoords[1]}});
            }

            cout << "Results in R*-Tree but not in brute-force:\n";
            for (const auto& rect : rtreeSet) {
                if (bruteSet.find(rect) == bruteSet.end()) {
                    cout << "[(" << rect.first.first << ", " << rect.first.second << "), (" 
                        << rect.second.first << ", " << rect.second.second << ")]\n";
                }
            }

            cout << "Results in brute-force but not in R*-Tree:\n";
            for (const auto& rect : bruteSet) {
                if (rtreeSet.find(rect) == rtreeSet.end()) {
                    cout << "[(" << rect.first.first << ", " << rect.first.second << "), (" 
                        << rect.second.first << ", " << rect.second.second << ")]\n";
                }
            }
            break;
        }
    }

    cout << "Number of queries: " << numQueries << endl;

    if (allQueriesMatch) cout << "All queries matched!" << endl;
    else cout << "Some queries did not match!" << endl;

    cout << "R*Tree query time: " << treeQueryTime << endl;
    cout << "Brute force query time: " << bruteForceQueryTime << endl;


    TreeStats stats = tree.getStats();
    cout << "Tree Statistics" << endl;
    cout << "Height: " << stats.height << endl;
    cout << "Total Data Entries: " << stats.totalDataEntries << endl << endl;
    cout << "Total Nodes: " << stats.totalNodes << endl;
    cout << "Leaf Nodes: " << stats.leafNodes << endl;
    cout << "Internal Nodes: " << stats.internalNodes << endl << endl;
    cout << "Total Node Visits: " << stats.totalNodeVisits << endl;
    cout << "Leaf Node Visits: " << stats.leafNodeVisits << endl;
    cout << "Internal Node Visits: " << stats.internalNodeVisits << endl;

    return 0;
}
