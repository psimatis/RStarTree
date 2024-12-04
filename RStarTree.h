#ifndef RSTARTREE_H
#define RSTARTREE_H

#include <vector>
#include <limits>
#include <cmath>
#include <iostream>

struct Rectangle {
    std::vector<float> minCoords, maxCoords;

    Rectangle() = default;
    Rectangle(int dimensions);
    Rectangle(const std::vector<float>& min, const std::vector<float>& max);

    float area() const;
    float overlap(const Rectangle& other) const;
    Rectangle combine(const Rectangle& other) const;
    bool contains(const Rectangle& other) const;
};

class RStarTree {
private:
    struct Node {
        bool isLeaf;
        std::vector<Rectangle> entries;
        std::vector<Node*> children;
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
    void adjustTree(Node* node);

public:
    RStarTree(int maxEntries, int dimensions);
    ~RStarTree();

    void insert(const Rectangle& entry);

    std::vector<Rectangle> rangeQuery(const Rectangle& query) const;
    void rangeQueryHelper(Node* node, const Rectangle& query, std::vector<Rectangle>& results) const;

    Rectangle computeBoundingRectangle(Node* node) const;

    void printTree() const;
    void printTree(Node* node, int depth) const;
};

#endif // RSTARTREE_H
