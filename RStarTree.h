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
    int id;
    vector<float> minCoords, maxCoords;

    Rectangle() = default;
    Rectangle(int dimensions);
    Rectangle(const int id, const vector<float>& min, const vector<float>& max);

    vector<float> getCenter() const;
    static Rectangle combine(const vector<Rectangle>& rectangles);

    float area() const;
    float overlap(const Rectangle& other) const;
    
    bool overlapCheck(const Rectangle& other) const;

    void printRectangle(const string& label) const;
    
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
    Node(const vector<Rectangle>& entries);
    
    ~Node();
};


struct TreeInfo {
    size_t totalNodes = 0;
    size_t leafNodes = 0;
    size_t internalNodes = 0;
    size_t totalDataEntries = 0;
    size_t height = 0;
    size_t leafNodeVisits = 0;
    size_t internalNodeVisits = 0;
    size_t totalNodeVisits = 0;
    float sizeInMB = 0.0f;
    int dimensions = 0;
    int capacity = 0;
    int minCapacity = 0;
};


class RStarTree {
private:
    Node* root;
    int maxEntries;
    int minEntries;
    int dimensions;

    struct TreeInfo info;

public:
    RStarTree(int maxEntries, int dimensions);
    ~RStarTree();

    void insert(const Rectangle& entry);
    void insert(Node* currentNode, const Rectangle& entry, bool allowReinsertion);
    void reinsert(Node* node);
    void batchInsert(vector<Rectangle>& rectangles);
    void batchInsert(Node* currentNode, Node* newNode);
    void recursiveSTRSort(vector<Rectangle> &rects, int dim, int maxDim);
    void bulkLoad(vector<Rectangle>& rectangles);

    Node* chooseSubtree(Node* currentNode, const Rectangle& entry);
    Node* chooseSubtreeBatch(Node* currentNode, const Rectangle& entry);

    void splitNode(Node* node);

    void chooseBestSplit(const vector<Rectangle>& sortedEntries, vector<size_t>& sortedIndices, int& bestAxis, size_t& bestSplitIndex);
    void sortEntriesAndChildren(Node* node, const vector<Rectangle>& sortedEntries, vector<size_t>& sortedIndices, int bestAxis);
    void updateRectangles(Node* node);

    vector<Rectangle> rangeQuery(const Rectangle& query);
    void rangeQuery(Node* node, const Rectangle& query, vector<Rectangle>& results);
        
    void printTree() const;

    void checkHealth() const;
    void checkHealth(const Node* node) const;

    float calculateSizeInMB() const;
    TreeInfo getInfo() const;
};

#endif // RSTARTREE_H
