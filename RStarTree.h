#ifndef RSTARTREE_H
#define RSTARTREE_H

#include <vector>
#include <limits>
#include <algorithm>
#include <iostream>
#include <cmath>

using namespace std;

class Rectangle {
public:
    vector<float> minCoords, maxCoords;

    Rectangle() = default;
    Rectangle(int dimensions);
    Rectangle(const vector<float>& min, const vector<float>& max);

    float area() const;
    float overlap(const Rectangle& other) const;
    Rectangle combine(const Rectangle& other) const;
    
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

class RStarTree {
private:
    Node* root;
    int maxEntries;
    int minEntries;
    int dimensions;

public:
    RStarTree(int maxEntries, int dimensions);
    ~RStarTree();

    void insert(const Rectangle& entry);
    void insert(Node* currentNode, const Rectangle& entry);
    void splitNode(Node* node);
    Node* chooseSubtree(Node* currentNode, const Rectangle& entry);
    Rectangle computeBoundingRectangle(Node* node) const;
    void handleOverflow(Node* node);

    vector<Rectangle> rangeQuery(const Rectangle& query) const;
    void rangeQueryHelper(Node* node, const Rectangle& query, vector<Rectangle>& results) const;
        
    void printTree() const;
    void printTree(Node* node, int depth) const;
};

#endif // RSTARTREE_H
