#ifndef RSTARTREE_H
#define RSTARTREE_H

#include <vector>
#include <limits>
#include <algorithm>
#include <iostream>
#include <cmath>
#include <functional> 

using namespace std;

class Rectangle {
public:
    vector<float> minCoords, maxCoords;

    Rectangle() = default;
    Rectangle(int dimensions);
    Rectangle(const vector<float>& min, const vector<float>& max);

    float area() const;
    bool overlap(const Rectangle& other) const;

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
    int totalNodes = 0;
    int leafNodes = 0;
    int internalNodes = 0;
    int totalDataEntries = 0;
    int height = 0;
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
    void insert(Node* currentNode, const Rectangle& entry);
    void splitNode(Node* node);
    Node* chooseSubtree(Node* currentNode, const Rectangle& entry);
    void handleOverflow(Node* node);

    vector<Rectangle> rangeQuery(const Rectangle& query) const;
    void rangeQuery(Node* node, const Rectangle& query, vector<Rectangle>& results) const;
        
    void printTree() const;
    void printTree(const Node* node, int depth) const;

    TreeStats getStats() const;
};

#endif // RSTARTREE_H
