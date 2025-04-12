#ifndef RSTARTREE_HPP
#define RSTARTREE_HPP

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
    float getAreaIncrease(const Rectangle& other) const;
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

public:
    RStarTree(int maxEntries, int dimensions);
    ~RStarTree();

    void insert(const Rectangle& entry);
    Node* insert(Node* currentNode, Node* parent, const Rectangle& entry, bool allowReinsertion);
    void reinsert(Node* node, Node* parent);
    void batchInsert(vector<Rectangle>& rectangles);
    void recursiveSTRSort(vector<Rectangle>& rects, int dim, int maxDim);
    void bulkLoad(vector<Rectangle>& rectangles);
    Node* chooseSubtree(Node* currentNode, const Rectangle& entry, bool isBatch);
    Node* splitNode(Node* node, Node* parent);
    void chooseBestSplit(const vector<Rectangle>& sortedEntries, vector<size_t>& sortedIndices, int& bestAxis, size_t& bestSplitIndex);
    void sortEntriesAndChildren(Node* node, const vector<Rectangle>& sortedEntries, vector<size_t>& sortedIndices, int bestAxis);
    void updateRectangles(Node* node);
    vector<Rectangle> rangeQuery(const Rectangle& query);
    void rangeQuery(Node* node, const Rectangle& query, vector<Rectangle>& results);
};

/////////////////////
// Rectangle
/////////////////////
Rectangle::Rectangle(int dimensions)
    : id(-1), 
      minCoords(dimensions, numeric_limits<float>::max()),
      maxCoords(dimensions, numeric_limits<float>::lowest()) {}

Rectangle::Rectangle(const int id, const vector<float>& min, const vector<float>& max)
    : id(id), minCoords(min), maxCoords(max) {}

float Rectangle::area() const {
    float result = 1.0f;
    for (size_t i = 0; i < minCoords.size(); ++i)
        result *= (maxCoords[i] - minCoords[i]);
    return result;
}

float Rectangle::overlap(const Rectangle& other) const {
    float overlapArea = 1.0;

    for (size_t i = 0; i < minCoords.size(); ++i) {
        float overlapMin = max(minCoords[i], other.minCoords[i]);
        float overlapMax = min(maxCoords[i], other.maxCoords[i]);

        if (overlapMax < overlapMin) return 0.0;
     
        overlapArea *= (overlapMax - overlapMin);
    }
    return overlapArea;
}

Rectangle Rectangle::combine(const vector<Rectangle>& rectangles) {
    if (rectangles.empty()) 
        return Rectangle();

    vector<float> combinedMin = rectangles[0].minCoords;
    vector<float> combinedMax = rectangles[0].maxCoords;

    for (const auto& rect : rectangles) {
        for (size_t i = 0; i < combinedMin.size(); ++i) {
            combinedMin[i] = min(combinedMin[i], rect.minCoords[i]);
            combinedMax[i] = max(combinedMax[i], rect.maxCoords[i]);
        }
    }
    return Rectangle(-1, combinedMin, combinedMax);
}

float Rectangle::getAreaIncrease(const Rectangle& other) const {
    const size_t dim = minCoords.size();
    float combinedArea = 1.0f;
    float originalArea = 1.0f;

    for (size_t d = 0; d < dim; ++d) {
        float newMin = min(minCoords[d], other.minCoords[d]);
        float newMax = max(maxCoords[d], other.maxCoords[d]);

        float oldExtent = (maxCoords[d] - minCoords[d]);
        float newExtent = (newMax - newMin);

        originalArea *= oldExtent;
        combinedArea *= newExtent;
    }

    return combinedArea - originalArea;
}

bool Rectangle::overlapCheck(const Rectangle& other) const {
    for (size_t i = 0; i < maxCoords.size(); ++i) {
        if (other.minCoords[i] > maxCoords[i] || other.maxCoords[i] < minCoords[i])
            return false;
    }
    return true; 
}

vector<float> Rectangle::getCenter() const {
    vector<float> center(minCoords.size());
    for (size_t i = 0; i < minCoords.size(); ++i)
        center[i] = (minCoords[i] + maxCoords[i]) / 2.0f;
    return center;
}

void Rectangle::printRectangle(const string& label) const {
    cout << label << " [";
    if (id != -1) // Print ID only if it is valid (not the default -1)
        cout << "ID: " << id << ", ";
    cout << "(";
    for (size_t i = 0; i < minCoords.size(); ++i) {
        cout << minCoords[i];
        if (i < minCoords.size() - 1) cout << ", ";
    }
    cout << "), (";
    for (size_t i = 0; i < maxCoords.size(); ++i) {
        cout << maxCoords[i];
        if (i < maxCoords.size() - 1) cout << ", ";
    }
    cout << ")]";
}

/////////////////////
// Node
/////////////////////
Node::Node(bool leaf)
    : isLeaf(leaf) {}

Node::Node(const vector<Rectangle>& rects)
    : isLeaf(true), entries(rects) {}

Node::~Node() {
    for (auto* child : children)
        delete child;
}


/////////////////////
// RStarTree
////////////////////
RStarTree::RStarTree(int maxEntries, int dimensions)
    : maxEntries(maxEntries), minEntries(maxEntries / 2), dimensions(dimensions) {
    root = new Node(true);
}

RStarTree::~RStarTree() {
    delete root;
}

void RStarTree::insert(const Rectangle& entry) {
    root = insert(root, nullptr, entry, true);
}

Node* RStarTree::insert(Node* currentNode, Node* parent, const Rectangle& entry, bool allowReinsertion) {
    if (!currentNode) {
        // Create a new leaf node if the tree is empty
        Node* newNode = new Node(true);
        newNode->entries.push_back(entry);
        return newNode;
    }
    
    if (currentNode->isLeaf) {
        currentNode->entries.push_back(entry);
        updateRectangles(currentNode);
        
        if (currentNode->entries.size() > maxEntries) {
            currentNode = splitNode(currentNode, parent);
            if (allowReinsertion) reinsert(currentNode, parent);
        }
    } else {
        Node* bestSubtree = chooseSubtree(currentNode, entry, false);
        Node* updatedSubtree = insert(bestSubtree, currentNode, entry, allowReinsertion);
        
        // If the subtree was replaced (e.g., due to a split), update the reference in the current node
        if (updatedSubtree != bestSubtree) {
            for (size_t i = 0; i < currentNode->children.size(); i++) {
                if (currentNode->children[i] == bestSubtree) {
                    currentNode->children[i] = updatedSubtree;
                    break;
                }
            }
        }
        
        // Update the current node's entries
        updateRectangles(currentNode);
    }
    
    return currentNode;
}

Node* RStarTree::chooseSubtree(Node* currentNode, const Rectangle& entry, bool isBatch) {
    if (!currentNode) return nullptr;

    Node* bestSubtree = nullptr;
    float minAreaIncrease = numeric_limits<float>::max();
    float minArea = numeric_limits<float>::max();

    for (auto* child : currentNode->children) {
        float areaIncrease = child->entries.front().getAreaIncrease(entry);
        float area = child->entries.front().area();

        if (areaIncrease < minAreaIncrease || (areaIncrease == minAreaIncrease && area < minArea)) {
            minAreaIncrease = areaIncrease;
            minArea = area;
            bestSubtree = child;
        }
    }

    return bestSubtree;
}

void RStarTree::batchInsert(vector<Rectangle>& rectangles) {
    bulkLoad(rectangles);
}

void RStarTree::recursiveSTRSort(vector<Rectangle>& rects, int dim, int maxDim) {
    if (dim >= maxDim) return;

    sort(rects.begin(), rects.end(), [dim](const Rectangle& a, const Rectangle& b) {
        return a.getCenter()[dim] < b.getCenter()[dim];
    });

    size_t sliceSize = (rects.size() + maxEntries - 1) / maxEntries;

    for (size_t start = 0; start < rects.size(); start += sliceSize) {
        size_t end = min(start + sliceSize, rects.size());
        vector<Rectangle> subRects(rects.begin() + start, rects.begin() + end);
        recursiveSTRSort(subRects, dim + 1, maxDim);
    }
}

void RStarTree::bulkLoad(vector<Rectangle>& rectangles) {
    if (rectangles.empty()) return;

    recursiveSTRSort(rectangles, 0, dimensions);

    vector<Node*> newNodes;
    size_t sliceSize = (rectangles.size() + maxEntries - 1) / maxEntries;

    for (size_t start = 0; start < rectangles.size(); start += sliceSize) {
        size_t end = min(start + sliceSize, rectangles.size());
        Node* newNode = new Node(vector<Rectangle>(rectangles.begin() + start, rectangles.begin() + end));
        newNodes.push_back(newNode);
    }

    if (newNodes.size() == 1) {
        delete root;
        root = newNodes.front();
    } else {
        Node* newRoot = new Node(false);
        for (auto* node : newNodes) {
            newRoot->children.push_back(node);
        }
        delete root;
        root = newRoot;
    }
    updateRectangles(root);
}

void RStarTree::updateRectangles(Node* node) {
    if (!node || node->isLeaf) return;

    for (auto* child : node->children) 
        node->entries.push_back(Rectangle::combine(child->entries));
}

void RStarTree::reinsert(Node* node, Node* parent) {
    if (!node || node->isLeaf) return;

    vector<Rectangle> entriesToReinsert;
    vector<Node*> childrenToReinsert;

    // Identify entries to reinsert (typically 30% of entries)
    size_t reinsertCount = node->entries.size() / 3;
    if (reinsertCount == 0) return;

    // Take the last reinsertCount entries for reinsertion
    for (size_t i = 0; i < reinsertCount; i++) {
        size_t idx = node->entries.size() - 1;
        entriesToReinsert.push_back(node->entries[idx]);
        if (!node->isLeaf) 
            childrenToReinsert.push_back(node->children[idx]);
        node->entries.pop_back();
        if (!node->isLeaf) 
            node->children.pop_back();
    }

    // Update MBR after removing entries
    updateRectangles(node);

    // Reinsert entries
    for (size_t i = 0; i < entriesToReinsert.size(); i++) {
        if (node->isLeaf) 
            // For leaf nodes, just reinsert the entry
            root = insert(root, nullptr, entriesToReinsert[i], false);
        else {
            // For internal nodes, find the best subtree for each child
            Node* child = childrenToReinsert[i];
            Rectangle mbr = entriesToReinsert[i];
            
            // Find the best node to insert this child into
            Node* bestNode = chooseSubtree(root, mbr, true);
            
            // Add the child to the best node
            bestNode->entries.push_back(mbr);
            bestNode->children.push_back(child);
            
            // Update MBR of the best node
            updateRectangles(bestNode);
            
            // Check if the best node needs to be split
            if (bestNode->entries.size() > maxEntries) {
                // We know the parent of bestNode is root in this context
                Node* newNode = splitNode(bestNode, root);
                
                // If the root was split, update it
                if (bestNode == root) 
                    root = newNode;
            }
        }
    }
}



void RStarTree::chooseBestSplit(const vector<Rectangle>& sortedEntries, vector<size_t>& sortedIndices, int& bestAxis, size_t& bestSplitIndex) {
    float minMarginSum = numeric_limits<float>::max();
    size_t bestIndex = 0;

    for (int axis = 0; axis < dimensions; ++axis) {
        float marginSum = 0.0f;

        for (size_t i = 0; i < sortedEntries.size(); ++i) 
            marginSum += sortedEntries[i].getAreaIncrease(Rectangle::combine(vector<Rectangle>(sortedEntries.begin(), sortedEntries.begin() + i)));

        if (marginSum < minMarginSum) {
            minMarginSum = marginSum;
            bestIndex = axis;
        }
    }

    bestAxis = bestIndex;
    bestSplitIndex = sortedIndices[bestIndex];
}

void RStarTree::sortEntriesAndChildren(Node* node, const vector<Rectangle>& sortedEntries, vector<size_t>& sortedIndices, int bestAxis) {
    if (!node) return;

    sort(sortedIndices.begin(), sortedIndices.end(), [&](size_t a, size_t b) {
        return sortedEntries[a].getCenter()[bestAxis] < sortedEntries[b].getCenter()[bestAxis];
    });

    vector<Node*> sortedChildren;
    for (size_t index : sortedIndices) 
        sortedChildren.push_back(node->children[index]);

    node->children = sortedChildren;
}

Node* RStarTree::splitNode(Node* node, Node* parent) {
    if (!node) return node;

    // Create a new node
    Node* newNode = new Node(node->isLeaf);

    // Calculate split point (middle)
    size_t splitIndex = node->entries.size() / 2;

    // Create copies of entries and children to be moved
    vector<Rectangle> entriesToMove;
    vector<Node*> childrenToMove;

    // Move half of the entries to the new node
    for (size_t i = splitIndex; i < node->entries.size(); ++i)
        entriesToMove.push_back(node->entries[i]);

    // Move corresponding children if not a leaf
    if (!node->isLeaf) {
        for (size_t i = splitIndex; i < node->children.size(); ++i) 
            childrenToMove.push_back(node->children[i]);
    }

    // Update new node with entries and children
    newNode->entries = entriesToMove;
    if (!node->isLeaf) 
        newNode->children = childrenToMove;

    // Remove moved entries and children from original node
    node->entries.resize(splitIndex);
    if (!node->isLeaf) 
        node->children.resize(splitIndex);

    // Update MBRs
    updateRectangles(node);
    updateRectangles(newNode);

    // Handle parent relationships
    if (!parent) {
        // Node is the root, create new root
        Node* newRoot = new Node(false);
        newRoot->children.push_back(node);
        newRoot->children.push_back(newNode);
        updateRectangles(newRoot);
        return newRoot;
    } else {
        // Add new node to existing parent
        parent->children.push_back(newNode);
        updateRectangles(parent);
        return node;
    }
}

vector<Rectangle> RStarTree::rangeQuery(const Rectangle& query){
    vector<Rectangle> results;
    if (root) rangeQuery(root, query, results);
    return results;
}

void RStarTree::rangeQuery(Node* node, const Rectangle& query, vector<Rectangle>& results) {
    if (!node) return;

    for (size_t i = 0; i < node->entries.size(); ++i) {
        const Rectangle& currentEntry = node->entries[i];
        if (query.overlapCheck(currentEntry)) {
            if (node->isLeaf) results.push_back(currentEntry);
            else rangeQuery(node->children[i], query, results);
        }
    }
}

#endif // RSTARTREE_HPP
