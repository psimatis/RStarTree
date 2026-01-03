/*
=====================================================================
R*-Tree Benchmark: Custom R*-Tree vs Boost.Geometry R-tree
=====================================================================

Compares performance of:
    1. Custom R*-Tree implementation
    2. Boost.Geometry R-tree (with R*-tree algorithm)

Operations benchmarked:
    - Single insertions
    - Bulk loading
    - Range queries

Requirements:
    - Boost library (libboost-dev on Ubuntu/Debian)

Compile:
    g++ -std=c++17 -O3 -o benchmark_boost benchmark_boost.cpp

=====================================================================
*/

#include "RStarTree.hpp"
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <random>

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

using BoostPoint = bg::model::point<float, 2, bg::cs::cartesian>;
using BoostBox = bg::model::box<BoostPoint>;
using BoostValue = std::pair<BoostBox, int>;  // box + id
using BoostRTree = bgi::rtree<BoostValue, bgi::rstar<128>>;  // R*-tree with capacity 128

using namespace std::chrono;

struct BenchmarkResult {
    double insertTime;
    double bulkLoadTime;
    double queryTime;
    size_t queryResults;
};

// Generate random 2D points as rectangles (point = rectangle with same min/max)
std::vector<Rectangle> generateRandomData(int numData, float minRange, float maxRange, unsigned seed) {
    std::mt19937 gen(seed);
    std::uniform_real_distribution<float> dist(minRange, maxRange);
    
    std::vector<Rectangle> dataPoints;
    dataPoints.reserve(numData);
    
    for (int i = 0; i < numData; ++i) {
        float x = dist(gen);
        float y = dist(gen);
        dataPoints.emplace_back(i, std::vector<float>{x, y}, std::vector<float>{x, y});
    }
    return dataPoints;
}

// Generate random range queries
std::vector<Rectangle> generateQueries(int numQueries, float minRange, float maxRange, float querySize, unsigned seed) {
    std::mt19937 gen(seed);
    std::uniform_real_distribution<float> dist(minRange, maxRange - querySize);
    
    std::vector<Rectangle> queries;
    queries.reserve(numQueries);
    
    for (int i = 0; i < numQueries; ++i) {
        float x = dist(gen);
        float y = dist(gen);
        queries.emplace_back(i, 
            std::vector<float>{x, y}, 
            std::vector<float>{x + querySize, y + querySize});
    }
    return queries;
}

// ==================== Custom R*-Tree Benchmarks ====================

double benchmarkCustomInsert(RStarTree& tree, const std::vector<Rectangle>& data) {
    auto start = high_resolution_clock::now();
    for (const auto& rect : data) {
        tree.insert(rect);
    }
    auto end = high_resolution_clock::now();
    return duration_cast<microseconds>(end - start).count() / 1000.0;
}

double benchmarkCustomBulkLoad(RStarTree& tree, std::vector<Rectangle> data) {
    auto start = high_resolution_clock::now();
    tree.bulkLoad(data);
    auto end = high_resolution_clock::now();
    return duration_cast<microseconds>(end - start).count() / 1000.0;
}

std::pair<double, size_t> benchmarkCustomQuery(RStarTree& tree, const std::vector<Rectangle>& queries) {
    size_t totalResults = 0;
    auto start = high_resolution_clock::now();
    for (const auto& query : queries) {
        auto results = tree.rangeQuery(query);
        totalResults += results.size();
    }
    auto end = high_resolution_clock::now();
    return {duration_cast<microseconds>(end - start).count() / 1000.0, totalResults};
}

// ==================== Boost R-tree Benchmarks ====================

double benchmarkBoostInsert(BoostRTree& tree, const std::vector<Rectangle>& data) {
    auto start = high_resolution_clock::now();
    for (const auto& rect : data) {
        BoostBox box(
            BoostPoint(rect.minCoords[0], rect.minCoords[1]),
            BoostPoint(rect.maxCoords[0], rect.maxCoords[1])
        );
        tree.insert(std::make_pair(box, rect.id));
    }
    auto end = high_resolution_clock::now();
    return duration_cast<microseconds>(end - start).count() / 1000.0;
}

double benchmarkBoostBulkLoad(const std::vector<Rectangle>& data) {
    std::vector<BoostValue> values;
    values.reserve(data.size());
    
    for (const auto& rect : data) {
        BoostBox box(
            BoostPoint(rect.minCoords[0], rect.minCoords[1]),
            BoostPoint(rect.maxCoords[0], rect.maxCoords[1])
        );
        values.emplace_back(box, rect.id);
    }
    
    auto start = high_resolution_clock::now();
    BoostRTree tree(values.begin(), values.end());
    auto end = high_resolution_clock::now();
    return duration_cast<microseconds>(end - start).count() / 1000.0;
}

std::pair<double, size_t> benchmarkBoostQuery(BoostRTree& tree, const std::vector<Rectangle>& queries) {
    size_t totalResults = 0;
    std::vector<BoostValue> results;
    
    auto start = high_resolution_clock::now();
    for (const auto& query : queries) {
        results.clear();
        BoostBox queryBox(
            BoostPoint(query.minCoords[0], query.minCoords[1]),
            BoostPoint(query.maxCoords[0], query.maxCoords[1])
        );
        tree.query(bgi::intersects(queryBox), std::back_inserter(results));
        totalResults += results.size();
    }
    auto end = high_resolution_clock::now();
    return {duration_cast<microseconds>(end - start).count() / 1000.0, totalResults};
}

// ==================== Main Benchmark ====================

void printHeader() {
    std::cout << "\n";
    std::cout << "╔══════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║         R*-Tree Benchmark: Custom vs Boost.Geometry              ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════════╝\n\n";
}

void printResults(const std::string& operation, double customTime, double boostTime) {
    double speedup = boostTime / customTime;
    std::string winner = customTime < boostTime ? "Custom" : "Boost";
    
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "  " << std::left << std::setw(20) << operation 
              << "│ Custom: " << std::setw(10) << customTime << " ms"
              << " │ Boost: " << std::setw(10) << boostTime << " ms"
              << " │ Winner: " << winner;
    if (speedup > 1.0) {
        std::cout << " (" << speedup << "x faster)";
    } else {
        std::cout << " (" << (1.0/speedup) << "x faster)";
    }
    std::cout << "\n";
}

void runBenchmark(int numData, int numQueries, int capacity) {
    const float minRange = 0.0f;
    const float maxRange = 100000.0f;
    const float querySize = 1000.0f;  // 1% of space
    const unsigned seed = 42;
    
    std::cout << "Configuration:\n";
    std::cout << "  Data points:    " << numData << "\n";
    std::cout << "  Queries:        " << numQueries << "\n";
    std::cout << "  Node capacity:  " << capacity << "\n";
    std::cout << "  Space range:    [" << minRange << ", " << maxRange << "]\n";
    std::cout << "  Query size:     " << querySize << " x " << querySize << "\n\n";
    
    // Generate data
    std::cout << "Generating data...\n";
    auto data = generateRandomData(numData, minRange, maxRange, seed);
    auto queries = generateQueries(numQueries, minRange, maxRange, querySize, seed + 1);
    
    std::cout << "\n─────────────────────────────────────────────────────────────────────\n";
    std::cout << "                         SINGLE INSERTION\n";
    std::cout << "─────────────────────────────────────────────────────────────────────\n";
    
    // Single insertion benchmark
    RStarTree customTree1(capacity, 2);
    double customInsertTime = benchmarkCustomInsert(customTree1, data);
    
    BoostRTree boostTree1;
    double boostInsertTime = benchmarkBoostInsert(boostTree1, data);
    
    printResults("Insert", customInsertTime, boostInsertTime);
    
    // Query after single insertion
    auto [customQueryTime1, customResults1] = benchmarkCustomQuery(customTree1, queries);
    auto [boostQueryTime1, boostResults1] = benchmarkBoostQuery(boostTree1, queries);
    
    printResults("Range Query", customQueryTime1, boostQueryTime1);
    std::cout << "  Query results:      Custom: " << customResults1 << " │ Boost: " << boostResults1 << "\n";
    
    std::cout << "\n─────────────────────────────────────────────────────────────────────\n";
    std::cout << "                           BULK LOADING\n";
    std::cout << "─────────────────────────────────────────────────────────────────────\n";
    
    // Bulk load benchmark
    RStarTree customTree2(capacity, 2);
    double customBulkTime = benchmarkCustomBulkLoad(customTree2, data);
    double boostBulkTime = benchmarkBoostBulkLoad(data);
    
    printResults("Bulk Load", customBulkTime, boostBulkTime);
    
    // Query after bulk load
    auto [customQueryTime2, customResults2] = benchmarkCustomQuery(customTree2, queries);
    
    // Need to rebuild boost tree for querying
    std::vector<BoostValue> boostValues;
    for (const auto& rect : data) {
        BoostBox box(
            BoostPoint(rect.minCoords[0], rect.minCoords[1]),
            BoostPoint(rect.maxCoords[0], rect.maxCoords[1])
        );
        boostValues.emplace_back(box, rect.id);
    }
    BoostRTree boostTree2(boostValues.begin(), boostValues.end());
    auto [boostQueryTime2, boostResults2] = benchmarkBoostQuery(boostTree2, queries);
    
    printResults("Range Query", customQueryTime2, boostQueryTime2);
    std::cout << "  Query results:      Custom: " << customResults2 << " │ Boost: " << boostResults2 << "\n";
    
    std::cout << "\n─────────────────────────────────────────────────────────────────────\n";
    std::cout << "                            SUMMARY\n";
    std::cout << "─────────────────────────────────────────────────────────────────────\n";
    
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "  Custom R*-Tree memory: " << customTree2.calculateSizeInMB() << " MB\n";
    std::cout << "  Boost R-tree entries:  " << boostTree2.size() << "\n";
}

int main(int argc, char* argv[]) {
    int numData = 100000;
    int numQueries = 1000;
    int capacity = 128;
    
    // Parse arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if ((arg == "-n" || arg == "--numData") && i + 1 < argc) {
            numData = std::atoi(argv[++i]);
        } else if ((arg == "-q" || arg == "--numQueries") && i + 1 < argc) {
            numQueries = std::atoi(argv[++i]);
        } else if ((arg == "-c" || arg == "--capacity") && i + 1 < argc) {
            capacity = std::atoi(argv[++i]);
        } else if (arg == "-h" || arg == "--help") {
            std::cout << "Usage: " << argv[0] << " [options]\n";
            std::cout << "Options:\n";
            std::cout << "  -n, --numData <num>     Number of data points (default: 100000)\n";
            std::cout << "  -q, --numQueries <num>  Number of queries (default: 1000)\n";
            std::cout << "  -c, --capacity <num>    Node capacity (default: 128)\n";
            return 0;
        }
    }
    
    printHeader();
    runBenchmark(numData, numQueries, capacity);
    
    std::cout << "\nBenchmark completed.\n\n";
    return 0;
}
