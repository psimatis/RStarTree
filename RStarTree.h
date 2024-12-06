#ifndef RSTARTREE_H
#define RSTARTREE_H

#include <vector>
#include <limits>
#include <algorithm>
#include <iostream>
#include <cmath>
#include <functional>
#include <numeric>

using namespace std;

class Rectangle {
public:
    vector<float> minCoords, maxCoords;

    Rectangle() = default;
    Rectangle(int dimensions);
    Rectangle(const vector<float>& min, const vector<float>& max);

    void printRectangle(const string& label) const;
    float area() const;
    bool overlapCheck(const Rectangle& other) const;
    float overlap(const Rectangle& other) const;

    static Rectangle combine(const vector<Rectangle>& rectangles);
    
    bool operator==(const Rectangle& other) const {
        return minCoords == other.minCoords && maxCoords == other.maxCoords;
    }
};

class Node {
public:
    bool isLeaf;
    vector<Rectangle> entries;
    vector<Node*> children;
    Node* parent;

    Node(bool leaf);
    ~Node();
};


struct TreeStats {
    size_t totalNodes = 0;
    size_t leafNodes = 0;
    size_t internalNodes = 0;
    size_t totalDataEntries = 0;
    size_t height = 0;
    size_t leafNodeVisits = 0;
    size_t internalNodeVisits = 0;
    size_t totalNodeVisits = 0;
};


class RStarTree {
private:
    Node* root;
    int maxEntries;
    int minEntries;
    int dimensions;

    struct TreeStats stats;

public:
    RStarTree(int maxEntries, int dimensions);
    ~RStarTree();

    void insert(const Rectangle& entry);
    void insert(Node* currentNode, const Rectangle& entry, bool allowReinsertion);
    void reinsert(Node* node);
    void splitNode(Node* node);
    Node* chooseSubtree(Node* currentNode, const Rectangle& entry);
    void updateRectangles(Node* node);

    vector<Rectangle> rangeQuery(const Rectangle& query);
    void rangeQuery(Node* node, const Rectangle& query, vector<Rectangle>& results);
        
    void printTree() const;
    void printTree(const Node* node, int depth) const;

    void checkHealth() const;
    void checkHealth(const Node* node) const;

    TreeStats getStats();
};

#endif // RSTARTREE_H
