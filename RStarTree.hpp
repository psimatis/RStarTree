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
    Node* insertNode(Node* currentNode, Node* parent, Node* newNode);
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

float Rectangle::area() const {
    float result = 1.0f;
    for (size_t i = 0; i < minCoords.size(); ++i)
        result *= (maxCoords[i] - minCoords[i]);
    return result;
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
    if (rectangles.empty()) return;

    sort(rectangles.begin(), rectangles.end(), [](const Rectangle& a, const Rectangle& b) {
        return a.minCoords[0] < b.minCoords[0];
    });

    int numBatches = (rectangles.size() + maxEntries - 1) / maxEntries;

    for (int i = 0; i < numBatches; ++i) {
        int startIdx = i * maxEntries;
        int endIdx = min(static_cast<int>(rectangles.size()), startIdx + maxEntries);
        vector<Rectangle> batch(rectangles.begin() + startIdx, rectangles.begin() + endIdx);

        Node* newNode = new Node(batch);
        root = insertNode(root, nullptr, newNode);
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
    if (rectangles.empty()) return;

    recursiveSTRSort(rectangles, 0, dimensions);

    vector<Node*> newNodes;
    size_t sliceSize = (rectangles.size() + maxEntries - 1) / maxEntries;

    for (size_t start = 0; start < rectangles.size(); start += sliceSize) {
        size_t end = min(start + sliceSize, rectangles.size());
        Node* newNode = new Node(vector<Rectangle>(rectangles.begin() + start, rectangles.begin() + end));
        // Ensure each new node has children initialized
        if (!newNode->isLeaf) {
            for (size_t i = 0; i < newNode->entries.size(); ++i) 
                newNode->children.push_back(new Node(true)); // Initialize with leaf children
        }
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
    
    // Ensure entries and children sizes match
    if (node->entries.size() != node->children.size()) {
        cerr << "Warning: Entries and children sizes don't match in updateRectangles." << endl;
        // Adjust entries to match children if needed
        while (node->entries.size() < node->children.size() && node->children.size() > 0) {
            size_t idx = node->entries.size();
            if (idx < node->children.size() && node->children[idx] && !node->children[idx]->entries.empty())
                node->entries.push_back(Rectangle::combine(node->children[idx]->entries));
            else {
                // If child has no entries, remove it
                if (idx < node->children.size()) {
                    delete node->children[idx];
                    node->children.erase(node->children.begin() + idx);
                }
            }
        }
        
        // If we have more entries than children, remove excess entries
        if (node->entries.size() > node->children.size())
            node->entries.resize(node->children.size());
    }
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

Node* RStarTree::insertNode(Node* currentNode, Node* parent, Node* newNode) {
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
    
    Node* bestNode = chooseSubtree(currentNode, Rectangle::combine(newNode->entries), true);
    
    if (bestNode->isLeaf) {
        // Best node is a leaf, adding new node as child
        currentNode->entries.push_back(Rectangle::combine(newNode->entries));
        currentNode->children.push_back(newNode);
    } else {
        // Recursively inserting into best node
        Node* result = insertNode(bestNode, currentNode, newNode);
        
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

    // Log current state of node
    cout << "Current node has " << currentNode->entries.size() << " entries and " << currentNode->children.size() << " children." << endl;
    cout << "Entries: ";
    for (const auto& entry : currentNode->entries) {
        cout << "(";
        for (size_t i = 0; i < entry.minCoords.size(); ++i) {
            cout << entry.minCoords[i];
            if (i < entry.minCoords.size() - 1) cout << ", ";
        }
        cout << "), (";
        for (size_t i = 0; i < entry.maxCoords.size(); ++i) {
            cout << entry.maxCoords[i];
            if (i < entry.maxCoords.size() - 1) cout << ", ";
        }
        cout << ") ";
    }
    cout << endl;

    // Ensure entries and children sizes are consistent
    if (currentNode->children.size() != currentNode->entries.size()) 
        cerr << "Error: Mismatch between entries and children sizes." << endl;
    
    if (currentNode->entries.size() > maxEntries) {
        // Current node exceeds max entries, splitting node
        Node* splitResult = splitNode(currentNode, parent);
        if (splitResult != currentNode) {
            if (parent) {
                for (size_t i = 0; i < parent->children.size(); i++) {
                    if (parent->children[i] == currentNode) {
                        parent->children[i] = splitResult;
                        break;
                    }
                }
            }
        }
        return splitResult;
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
            float area = leftBoundingRect.area() + rightBoundingRect.area();

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

void RStarTree::checkHealth() const {
    function<void(const Node*)> validateNode = [&](const Node* node) {
        if (!node) return;

        if (!node->isLeaf) {
            vector<Rectangle> childRects;
            for (const auto* child : node->children) 
                childRects.push_back(Rectangle::combine(child->entries));

            Rectangle combinedRect = Rectangle::combine(childRects);
            if (!node->entries[0].overlapCheck(combinedRect)) {
                cerr << "Validation Error: Parent rectangle does not encompass all children!" << endl;
                node->entries[0].printRectangle("Parent Rect");
                combinedRect.printRectangle("Combined Child Rects");
            }

            for (const auto* child : node->children) {
                validateNode(child);
            }
        }
    };
    validateNode(root);
}

#endif // RSTARTREE_HPP
