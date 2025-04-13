#ifndef RSTARTREE_HPP
#define RSTARTREE_HPP

#include <vector>
#include <limits>
#include <algorithm>
#include <iostream>
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
    float getArea() const;
    float getAreaIncrease(const Rectangle& other) const;
    float getOverlapArea(const Rectangle& other) const;
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

class RStarTree {
public:
    Node* root;
    int maxEntries;
    int minEntries;
    int dimensions;

    RStarTree(int maxEntries, int dimensions);
    ~RStarTree();
    void insert(const Rectangle& entry);
    Node* insert(Node* currentNode, const Rectangle& entry, bool allowReinsertion);
    void reinsert(Node* node);
    void batchInsert(vector<Rectangle>& rectangles);
    void recursiveSTRSort(vector<Rectangle>& rects, int dim, int maxDim);
    void bulkLoad(vector<Rectangle>& rectangles);
    Node* chooseSubtree(Node* currentNode, const Rectangle& entry, bool isBatch);
    Node* splitNode(Node* node);
    void chooseBestSplit(const vector<Rectangle>& sortedEntries, vector<size_t>& sortedIndices, int& bestAxis, size_t& bestSplitIndex);
    void sortEntriesAndChildren(Node* node, const vector<Rectangle>& sortedEntries, vector<size_t>& sortedIndices, int bestAxis);
    void updateRectangles(Node* node);
    vector<Rectangle> rangeQuery(const Rectangle& query);
    void rangeQuery(Node* node, const Rectangle& query, vector<Rectangle>& results);
    Node* insertNode(Node* currentNode, Node* newNode);
    void checkHealth() const;
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

float Rectangle::getArea() const {
    float result = 1.0f;
    for (size_t i = 0; i < minCoords.size(); ++i)
        result *= (maxCoords[i] - minCoords[i]);
    return result;
}

Rectangle Rectangle::combine(const vector<Rectangle>& rectangles) {
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
    Rectangle combinedRect = combine({*this, other});
    return combinedRect.getArea() - getArea();
}

float Rectangle::getOverlapArea(const Rectangle& other) const {
    float overlapArea = 1.0;

    for (size_t i = 0; i < minCoords.size(); ++i) {
        float overlapMin = max(minCoords[i], other.minCoords[i]);
        float overlapMax = min(maxCoords[i], other.maxCoords[i]);

        if (overlapMax < overlapMin) return 0.0;
     
        overlapArea *= (overlapMax - overlapMin);
    }
    return overlapArea;
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
    root = insert(root, entry, true);
}

Node* RStarTree::insert(Node* currentNode, const Rectangle& entry, bool allowReinsertion) {
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
            Node* splitResult = splitNode(currentNode);
            if (allowReinsertion) reinsert(splitResult);
            return splitResult;
        }
    } else {
        Node* bestSubtree = chooseSubtree(currentNode, entry, false);
        Node* updatedSubtree = insert(bestSubtree, entry, allowReinsertion);
        
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
        
        if (currentNode->entries.size() > maxEntries) {
            Node* splitResult = splitNode(currentNode);
            if (allowReinsertion) reinsert(splitResult);
            return splitResult;
        }
    }
    return currentNode;
}

Node* RStarTree::chooseSubtree(Node* currentNode, const Rectangle& entry, bool isBatch) {
    Node* bestSubtree = nullptr;
    float minAreaIncrease = numeric_limits<float>::max();
    float minArea = numeric_limits<float>::max();

    for (auto* child : currentNode->children) {
        float areaIncrease = child->entries.front().getAreaIncrease(entry);
        float area = child->entries.front().getArea();

        if (areaIncrease < minAreaIncrease || (areaIncrease == minAreaIncrease && area < minArea)) {
            minAreaIncrease = areaIncrease;
            minArea = area;
            bestSubtree = child;
        }
    }
    return bestSubtree;
}

void RStarTree::batchInsert(vector<Rectangle>& rectangles) {

    if (root->isLeaf && root->entries.empty()){
        bulkLoad(rectangles);
        return;
    }

    sort(rectangles.begin(), rectangles.end(), [](const Rectangle& a, const Rectangle& b) {
        return a.minCoords[0] < b.minCoords[0];
    });

    int numBatches = (rectangles.size() + maxEntries - 1) / maxEntries;

    for (int i = 0; i < numBatches; ++i) {
        int startIdx = i * maxEntries;
        int endIdx = min(static_cast<int>(rectangles.size()), startIdx + maxEntries);
        vector<Rectangle> batch(rectangles.begin() + startIdx, rectangles.begin() + endIdx);

        Node* newNode = new Node(batch);
        root = insertNode(root, newNode);
    }
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
        for (auto* node : newNodes) 
            newRoot->children.push_back(node);
        delete root;
        root = newRoot;
    }
    updateRectangles(root);
}

void RStarTree::updateRectangles(Node* node) {
    if (!node || node->isLeaf) return;

    // Clear existing entries before updating
    node->entries.clear();
    
    // Ensure children vector is not empty
    if (node->children.empty()) return;
    
    // Update entries based on children
    for (auto* child : node->children) {
        if (child && !child->entries.empty()) 
            node->entries.push_back(Rectangle::combine(child->entries));
    }
}

void RStarTree::reinsert(Node* node) {
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
            root = insert(root, entriesToReinsert[i], false);
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
                Node* newNode = splitNode(bestNode);
                
                // If the root was split, update it
                if (bestNode == root) 
                    root = newNode;
            }
        }
    }
}

Node* RStarTree::insertNode(Node* currentNode, Node* newNode) {
    if (!currentNode)
        // Current node is null, returning new node
        return newNode;
    
    if (currentNode->isLeaf) {
        // Current node is a leaf, creating new internal node
        Node* newInternalNode = new Node(false);
        newInternalNode->children.push_back(currentNode);
        newInternalNode->children.push_back(newNode);
        newInternalNode->entries.push_back(Rectangle::combine(currentNode->entries));
        newInternalNode->entries.push_back(Rectangle::combine(newNode->entries));
        return newInternalNode;
    }
    
    // Current node is not a leaf, finding best subtree
    Node* bestNode = chooseSubtree(currentNode, Rectangle::combine(newNode->entries), true);
    
    if (bestNode->isLeaf) {
        // Best node is a leaf, adding new node as child
        currentNode->entries.push_back(Rectangle::combine(newNode->entries));
        currentNode->children.push_back(newNode);
    } else {
        // Recursively inserting into best node
        Node* result = insertNode(bestNode, newNode);
        
        if (result != bestNode) {
            // Best node was replaced, updating reference
            for (size_t i = 0; i < currentNode->children.size(); i++) {
                if (currentNode->children[i] == bestNode) {
                    currentNode->children[i] = result;
                    break;
                }
            }
        }
    }
    
    // Ensure entries and children are synchronized
    updateRectangles(currentNode);

    // Ensure entries and children sizes are consistent
    if (currentNode->children.size() != currentNode->entries.size()) 
        cerr << "Error: Mismatch between entries and children sizes." << endl;
    
    if (currentNode->entries.size() > maxEntries) {
        // Current node exceeds max entries, splitting node
        Node* splitResult = splitNode(currentNode);
        return splitResult; // Return the result directly, it will be either the original node or a new root
    }
    return currentNode;
}

void RStarTree::chooseBestSplit(const vector<Rectangle>& sortedEntries, vector<size_t>& sortedIndices, int& bestAxis, size_t& bestSplitIndex) {

    float minOverlap = numeric_limits<float>::max();
    float minArea = numeric_limits<float>::max();

    for (int axis = 0; axis < dimensions; ++axis) {
        // Sort entries by minCoords along the current axis and track indices
        sort(sortedIndices.begin(), sortedIndices.end(),
             [&sortedEntries, axis](size_t i, size_t j) {
                 return sortedEntries[i].minCoords[axis] < sortedEntries[j].minCoords[axis];
             });

        vector<Rectangle> sortedByAxis(sortedEntries.size());
        for (size_t i = 0; i < sortedIndices.size(); ++i) 
            sortedByAxis[i] = sortedEntries[sortedIndices[i]];

        // Evaluate split points for the current axis
        for (size_t splitIndex = minEntries; splitIndex <= sortedEntries.size() - minEntries; ++splitIndex) {
            vector<Rectangle> leftEntries(sortedByAxis.begin(), sortedByAxis.begin() + splitIndex);
            vector<Rectangle> rightEntries(sortedByAxis.begin() + splitIndex, sortedByAxis.end());

            Rectangle leftBoundingRect = Rectangle::combine(leftEntries);
            Rectangle rightBoundingRect = Rectangle::combine(rightEntries);

            float overlap = leftBoundingRect.getOverlapArea(rightBoundingRect);
            float area = leftBoundingRect.getArea() + rightBoundingRect.getArea();

            if (overlap < minOverlap || (overlap == minOverlap && area < minArea)) {
                bestAxis = axis;
                bestSplitIndex = splitIndex;
                minOverlap = overlap;
                minArea = area;
            }
        }
    }
}

void RStarTree::sortEntriesAndChildren(Node* node, const vector<Rectangle>& sortedEntries, vector<size_t>& sortedIndices, int bestAxis) {
    if (!node || node->children.empty()) return;

    // Sort indices based on the best axis
    sort(sortedIndices.begin(), sortedIndices.end(), [&](size_t a, size_t b) {
        return sortedEntries[a].getCenter()[bestAxis] < sortedEntries[b].getCenter()[bestAxis];
    });

    // Create a copy of the children before modifying them
    vector<Node*> oldChildren = node->children;
    vector<Node*> sortedChildren;
    
    // Only process indices that are within the bounds of oldChildren
    for (size_t i = 0; i < sortedIndices.size() && i < oldChildren.size(); ++i) {
        size_t index = sortedIndices[i];
        if (index < oldChildren.size()) 
            sortedChildren.push_back(oldChildren[index]);
    }
    
    // Only update if we have a valid result
    if (!sortedChildren.empty())
        node->children = sortedChildren;
}

Node* RStarTree::splitNode(Node* node) {
    if (!node || node->entries.empty()) {
        cerr << "Error: Invalid node in splitNode!" << endl;
        return node;
    }

    // Choose split axis and index
    int bestAxis = -1;
    size_t bestSplitIndex;
    vector<size_t> sortedIndices(node->entries.size());
    iota(sortedIndices.begin(), sortedIndices.end(), 0); // Fill with 0, 1, ..., totalEntries-1

    chooseBestSplit(node->entries, sortedIndices, bestAxis, bestSplitIndex);

    sortEntriesAndChildren(node, node->entries, sortedIndices, bestAxis);

    // Split
    vector<Rectangle> leftEntries(node->entries.begin(), node->entries.begin() + bestSplitIndex);
    vector<Rectangle> rightEntries(node->entries.begin() + bestSplitIndex, node->entries.end());

    Node* newNode = new Node(node->isLeaf);

    if (!node->isLeaf) {
        vector<Node*> leftChildren, rightChildren;
        for (size_t i = 0; i < node->entries.size(); ++i) {
            if (i < bestSplitIndex) 
                leftChildren.push_back(node->children[i]);
            else 
                rightChildren.push_back(node->children[i]);
        }

        node->children = leftChildren;
        newNode->children = rightChildren;
    }

    node->entries = leftEntries;
    newNode->entries = rightEntries;

    // If this is the root node being split, create a new root
    if (node == root) {
        Node* newRoot = new Node(false);
        newRoot->children.push_back(node);
        newRoot->children.push_back(newNode);
        newRoot->entries.push_back(Rectangle::combine(node->entries));
        newRoot->entries.push_back(Rectangle::combine(newNode->entries));
        return newRoot; // Return the new root to update the tree's root
    }
    
    // For non-root nodes, return the original node
    // The parent management will be handled by the caller
    return node;
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
            if (node->isLeaf)
                results.push_back(currentEntry);
            else {
                if (i < node->children.size())
                    rangeQuery(node->children[i], query, results);
                else {
                    cerr << "Error: Child index out of bounds." << endl;
                    return;
                }
            }
        }
    }
}

#endif // RSTARTREE_HPP
