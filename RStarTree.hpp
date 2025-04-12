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
    void insertLeaf(Node* currentNode, Node* newNode, bool allowReinsertion);
    void recursiveSTRSort(vector<Rectangle>& rects, int dim, int maxDim);
    void bulkLoad(vector<Rectangle>& rectangles);

    Node* chooseSubtree(Node* currentNode, const Rectangle& entry, bool isBatch);

    void splitNode(Node* node);

    void chooseBestSplit(const vector<Rectangle>& sortedEntries, vector<size_t>& sortedIndices, int& bestAxis, size_t& bestSplitIndex);
    void sortEntriesAndChildren(Node* node, const vector<Rectangle>& sortedEntries, vector<size_t>& sortedIndices, int bestAxis);
    void updateRectangles(Node* node);

    vector<Rectangle> rangeQuery(const Rectangle& query);
    void rangeQuery(Node* node, const Rectangle& query, vector<Rectangle>& results);
        
    void printTree() const;

    void checkHealth() const;

    float calculateSizeInMB() const;
    TreeInfo getInfo() const;
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
    if (id != -1) { // Print ID only if it is valid (not the default -1)
        cout << "ID: " << id << ", ";
    }
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
Node::Node(bool leaf) : isLeaf(leaf), parent(nullptr) {}

Node::Node(const vector<Rectangle>& rects) : isLeaf(true), entries(rects), parent(nullptr) {}

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
    std::cout << "Inserting rectangle: ";
    entry.printRectangle("");
    std::cout << std::endl;
    insert(root, entry, true);
    std::cout << "Insertion completed." << std::endl;
}

void RStarTree::insert(Node* currentNode, const Rectangle& entry, bool allowReinsertion) {
    if (!currentNode) {
        std::cerr << "Error: currentNode is null!" << std::endl;
        return;
    }
    
    if (currentNode->isLeaf) {
        currentNode->entries.push_back(entry);
        updateRectangles(currentNode);
        
        std::cout << "Inserted into leaf node. Current entries: " << currentNode->entries.size() << std::endl;
        if (currentNode->entries.size() > maxEntries) {
            std::cout << "Splitting node due to overflow..." << std::endl;
            splitNode(currentNode);
            if (allowReinsertion) reinsert(currentNode);
        }
    } else {
        Node* bestSubtree = chooseSubtree(currentNode, entry, false);
        insert(bestSubtree, entry, allowReinsertion);
    }
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

void RStarTree::insertLeaf(Node* currentNode, Node* newNode, bool allowReinsertion) {
    if (!currentNode) return;

    currentNode->children.push_back(newNode);
    updateRectangles(currentNode);

    if (currentNode->children.size() > maxEntries) {
        splitNode(currentNode);
        if (allowReinsertion) reinsert(currentNode);
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
    std::cout << "Bulk loading " << rectangles.size() << " rectangles..." << std::endl;
    if (rectangles.empty()) return;

    recursiveSTRSort(rectangles, 0, dimensions);

    vector<Node*> newNodes;
    size_t sliceSize = (rectangles.size() + maxEntries - 1) / maxEntries;

    for (size_t start = 0; start < rectangles.size(); start += sliceSize) {
        size_t end = min(start + sliceSize, rectangles.size());
        Node* newNode = new Node(vector<Rectangle>(rectangles.begin() + start, rectangles.begin() + end));
        newNodes.push_back(newNode);
        std::cout << "Created new node with " << newNode->entries.size() << " entries." << std::endl;
    }

    if (newNodes.size() == 1) {
        delete root;
        root = newNodes.front();
        std::cout << "Root replaced with new node." << std::endl;
    } else {
        Node* newRoot = new Node(false);
        for (auto* node : newNodes) {
            newRoot->children.push_back(node);
            node->parent = newRoot;
        }
        delete root;
        root = newRoot;
        std::cout << "New root created with " << newRoot->children.size() << " children." << std::endl;
    }
    updateRectangles(root);
    std::cout << "Bulk loading completed." << std::endl;
}

void RStarTree::updateRectangles(Node* node) {
    if (!node || node->isLeaf) return;

    for (auto* child : node->children) {
        node->entries.push_back(Rectangle::combine(child->entries));
    }
}

void RStarTree::reinsert(Node* node) {
    if (!node || node->isLeaf) return;

    vector<Rectangle> entriesToReinsert;

    for (auto it = node->entries.begin(); it != node->entries.end();) {
        if (it->overlapCheck(Rectangle::combine(node->entries))) {
            entriesToReinsert.push_back(*it);
            it = node->entries.erase(it);
        } else {
            ++it;
        }
    }

    for (const auto& entry : entriesToReinsert) {
        insert(root, entry, false);
    }
}

void RStarTree::chooseBestSplit(const vector<Rectangle>& sortedEntries, vector<size_t>& sortedIndices, int& bestAxis, size_t& bestSplitIndex) {
    float minMarginSum = numeric_limits<float>::max();
    size_t bestIndex = 0;

    for (int axis = 0; axis < dimensions; ++axis) {
        float marginSum = 0.0f;

        for (size_t i = 0; i < sortedEntries.size(); ++i) {
            marginSum += sortedEntries[i].getAreaIncrease(Rectangle::combine(vector<Rectangle>(sortedEntries.begin(), sortedEntries.begin() + i)));
        }

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
    for (size_t index : sortedIndices) {
        sortedChildren.push_back(node->children[index]);
    }

    node->children = sortedChildren;
}

void RStarTree::splitNode(Node* node) {
    if (!node) return;

    // Create a new node
    Node* newNode = new Node(node->isLeaf);

    // Calculate split point (middle)
    size_t splitIndex = node->entries.size() / 2;

    // Create copies of entries and children to be moved
    vector<Rectangle> entriesToMove;
    vector<Node*> childrenToMove;

    // Move half of the entries to the new node
    for (size_t i = splitIndex; i < node->entries.size(); ++i) {
        entriesToMove.push_back(node->entries[i]);
    }

    // Move corresponding children if not a leaf
    if (!node->isLeaf) {
        for (size_t i = splitIndex; i < node->children.size(); ++i) {
            childrenToMove.push_back(node->children[i]);
        }
    }

    // Update new node with entries and children
    newNode->entries = entriesToMove;
    if (!node->isLeaf) {
        newNode->children = childrenToMove;
        // Update parent pointers
        for (Node* child : newNode->children) {
            if (child) child->parent = newNode;
        }
    }

    // Remove moved entries and children from original node
    node->entries.resize(splitIndex);
    if (!node->isLeaf) {
        node->children.resize(splitIndex);
    }

    // Handle parent relationships
    if (!node->parent) {
        // Create new root
        Node* newRoot = new Node(false);
        newRoot->children.push_back(node);
        newRoot->children.push_back(newNode);
        node->parent = newRoot;
        newNode->parent = newRoot;
        root = newRoot;
        updateRectangles(root);
    } else {
        // Add new node to existing parent
        newNode->parent = node->parent;
        node->parent->children.push_back(newNode);
        updateRectangles(node->parent);
    }

    // Update MBRs for both nodes
    updateRectangles(node);
    updateRectangles(newNode);
}

void RStarTree::checkHealth() const {
    if (!root) {
        std::cerr << "Error: Tree is empty!" << std::endl;
        return;
    }

    function<void(const Node*)> validateNode = [&](const Node* node) {
        if (!node) return;

        if (node->isLeaf) {
            if (node->entries.size() > maxEntries) {
                cerr << "Error: Leaf node has too many entries" << endl;
            }
        } else {
            if (node->children.size() > maxEntries) {
                cerr << "Error: Internal node has too many children" << endl;
            }

            for (const auto* child : node->children)
                validateNode(child);
        }
    };
    validateNode(root);
}

vector<Rectangle> RStarTree::rangeQuery(const Rectangle& query){
    vector<Rectangle> results;
    if (root) rangeQuery(root, query, results);
    return results;
}

void RStarTree::rangeQuery(Node* node, const Rectangle& query, vector<Rectangle>& results) {
    if (!node) return;

    info.totalNodeVisits++;
    if (node->isLeaf) info.leafNodeVisits++;
    else info.internalNodeVisits++;

    for (size_t i = 0; i < node->entries.size(); ++i) {
        const Rectangle& currentEntry = node->entries[i];
        if (query.overlapCheck(currentEntry)) {
            if (node->isLeaf) results.push_back(currentEntry);
            else rangeQuery(node->children[i], query, results);
        }
    }
}

void RStarTree::printTree() const {
    cout << "R*-Tree Structure:" << endl;

    function<void(const Node*, int)> printNode = [&](const Node* node, int depth) {
        if (!node) return;

        // Indent for tree depth
        for (int i = 0; i < depth; ++i) cout << "  ";

        if (node->isLeaf) {
            cout << "Leaf Node ";
            Rectangle::combine(node->entries).printRectangle("");
            cout << " -> ";
            for (const auto& rect : node->entries) {
                cout << "[(";
                for (float val : rect.minCoords) cout << val << ", ";
                cout << "), (";
                for (float val : rect.maxCoords) cout << val << ", ";
                cout << ")] ";
            }
        } else {
            cout << "Internal Node ";
            Rectangle::combine(node->entries).printRectangle("");
            cout << " -> ";
            for (const auto& rect : node->entries) {
                cout << "[(";
                for (float val : rect.minCoords) cout << val << ", ";
                cout << "), (";
                for (float val : rect.maxCoords) cout << val << ", ";
                cout << ")] ";
            }
        }
        cout << endl;

        for (const auto* child : node->children) {
            printNode(child, depth + 1);
        }
    };

    printNode(root, 0);
    cout << "-------------" << endl;
}

float RStarTree::calculateSizeInMB() const {
    size_t totalSize = 0;

    function<void(const Node*)> calculateNodeSize = [&](const Node* node) {
        if (!node) return;

        totalSize += sizeof(bool);
        totalSize += sizeof(Node*); 
        totalSize += sizeof(vector<Node*>);
        totalSize += sizeof(vector<Rectangle>); 

        // Ignore data points
        if (!node->isLeaf) {
            totalSize += node->entries.size() * sizeof(Rectangle);

            for (const auto& rectangle : node->entries)
                totalSize += 2 * (dimensions * sizeof(float));
        }

        totalSize += node->children.size() * sizeof(Node*);

        for (const auto* child : node->children)
            calculateNodeSize(child);
    };

    calculateNodeSize(root);

    return static_cast<float>(totalSize) / (1024.0f * 1024.0f); 
}


TreeInfo RStarTree::getInfo() const {
    TreeInfo treeInfo;

    treeInfo.capacity = maxEntries;
    treeInfo.minCapacity = minEntries;
    treeInfo.dimensions = dimensions;
    treeInfo.leafNodeVisits = info.leafNodeVisits;
    treeInfo.internalNodeVisits = info.internalNodeVisits;

    // Lambda function to compute stats recursively
    function<void(const Node*, size_t)> computeStats = [&](const Node* node, size_t currentDepth) {
        if (!node) return;

        treeInfo.totalNodes++;
        if (node->isLeaf) {
            treeInfo.leafNodes++;
            treeInfo.totalDataEntries += node->entries.size();
        } else {
            treeInfo.internalNodes++;
            for (const auto* child : node->children)
                computeStats(child, currentDepth + 1);
        }

        treeInfo.height = max(treeInfo.height, currentDepth);
    };

    computeStats(root, 1); 
    treeInfo.sizeInMB = calculateSizeInMB(); 
    return treeInfo;
}

#endif // RSTARTREE_HPP
