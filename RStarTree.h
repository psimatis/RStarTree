#ifndef RSTARTREE_H
#define RSTARTREE_H

#include <vector>
#include <limits>
#include <algorithm>
#include <iostream>
#include <cmath>


using namespace std;

struct Rectangle {
    vector<float> minCoords, maxCoords;

    Rectangle() = default;
    Rectangle(int dimensions);
    Rectangle(const vector<float>& min, const vector<float>& max);

    float area() const;
    float overlap(const Rectangle& other) const;
    Rectangle combine(const Rectangle& other) const;
    bool contains(const Rectangle& other) const;

    bool operator==(const Rectangle& other) const {
        return minCoords == other.minCoords && maxCoords == other.maxCoords;
    }
};

class RStarTree {
public:
    struct Node {
        bool isLeaf;
        vector<Rectangle> entries;
        vector<Node*> children;
        Node* parent;

        Node(bool leaf);
        ~Node();
    };

    Node* root;
    int maxEntries;
    int minEntries;
    int dimensions;

    void insert(Node* currentNode, const Rectangle& entry);
    void splitNode(Node* node);
    Node* chooseSubtree(Node* currentNode, const Rectangle& entry);
    void rangeQueryHelper(Node* node, const Rectangle& query, vector<Rectangle>& results) const;
    Rectangle computeBoundingRectangle(Node* node) const;
    void handleOverflow(Node* node);

    RStarTree(int maxEntries, int dimensions);
    ~RStarTree();

    void insert(const Rectangle& entry);
    vector<Rectangle> rangeQuery(const Rectangle& query) const;
    void printTree() const;
    void printTree(Node* node, int depth) const;
};

#endif // RSTARTREE_H
