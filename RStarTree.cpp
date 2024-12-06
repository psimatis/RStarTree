#include "RStarTree.h"
#include <numeric>


/////////////////////
// Rectangle
/////////////////////
Rectangle::Rectangle(int dimensions)
    : minCoords(dimensions, numeric_limits<float>::max()),
      maxCoords(dimensions, numeric_limits<float>::lowest()) {}

Rectangle::Rectangle(const vector<float>& min, const vector<float>& max)
    : minCoords(min), maxCoords(max) {}

void Rectangle::printRectangle(const string& label) const {
    cout << label << " [(";
    for (size_t i = 0; i < minCoords.size(); ++i) {
        cout << minCoords[i];
        if (i < minCoords.size() - 1) {
            cout << ", ";
        }
    }
    cout << "), (";
    for (size_t i = 0; i < maxCoords.size(); ++i) {
        cout << maxCoords[i];
        if (i < maxCoords.size() - 1) {
            cout << ", ";
        }
    }
    cout << ")]" << endl;
}

float Rectangle::area() const {
    float result = 1.0f;
    for (size_t i = 0; i < minCoords.size(); ++i)
        result *= (maxCoords[i] - minCoords[i]);
    return result;
}

float Rectangle::overlap(const Rectangle& other) const {
    float overlapArea = 1.0;

    for (size_t i = 0; i < minCoords.size(); ++i) {
        // Calculate the overlap in the current dimension
        float overlapMin = max(minCoords[i], other.minCoords[i]);
        float overlapMax = min(maxCoords[i], other.maxCoords[i]);

        if (overlapMax < overlapMin) {
            // No overlap in this dimension
            return 0.0;
        }

        overlapArea *= (overlapMax - overlapMin);
    }

    return overlapArea;
}

Rectangle Rectangle::combine(const vector<Rectangle>& rectangles) {
    if (rectangles.empty()) 
        return Rectangle(vector<float>(2, 0), vector<float>(2, 0));

    vector<float> combinedMin = rectangles[0].minCoords;
    vector<float> combinedMax = rectangles[0].maxCoords;

    for (const auto& rect : rectangles) {
        for (size_t i = 0; i < combinedMin.size(); ++i) {
            combinedMin[i] = min(combinedMin[i], rect.minCoords[i]);
            combinedMax[i] = max(combinedMax[i], rect.maxCoords[i]);
        }
    }
    return Rectangle(combinedMin, combinedMax);
}


bool Rectangle::overlapCheck(const Rectangle& other) const {
    for (size_t i = 0; i < maxCoords.size(); ++i) {
        if (other.minCoords[i] > maxCoords[i] || other.maxCoords[i] < minCoords[i]) {
            return false;
        }
    }
    return true; 
}


/////////////////////
// Node
/////////////////////
Node::Node(bool leaf) : isLeaf(leaf), parent(nullptr) {}

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
    insert(root, entry);
}

Node* RStarTree::chooseSubtree(Node* currentNode, const Rectangle& entry) {
    if (!currentNode || currentNode->isLeaf) {
        cerr << "Error: chooseSubtree called on an invalid or leaf node!" << endl;
        return nullptr;
    }

    Node* bestChild = nullptr;
    float minAreaIncrease = numeric_limits<float>::max();

    for (size_t i = 0; i < currentNode->entries.size(); ++i) {
        Rectangle combined = Rectangle::combine({currentNode->entries[i], entry});
        float areaIncrease = combined.area() - currentNode->entries[i].area();

        if (areaIncrease < minAreaIncrease) {
            minAreaIncrease = areaIncrease;
            bestChild = currentNode->children[i];
        }
    }

    if (!bestChild)
        cerr << "Error: No valid subtree found in chooseSubtree!" << endl;

    return bestChild;
}

void RStarTree::updateBoundingRectangle(Node* node) {
    if (!node || node->entries.empty()) return;

    if (node->parent) {
        // cout << "updating parent" << endl;
        // Rectangle dummy = Rectangle::combine(node->parent->entries);
        // dummy.printRectangle("parent pre-update");
        Rectangle newBoundingRect = Rectangle::combine(node->entries);
        // Update parent's entry for this node
        for (size_t i = 0; i < node->parent->children.size(); ++i) {
            // how does this equality work? maybe its a problem
            if (node->parent->children[i] == node) {
                node->parent->entries[i] = newBoundingRect;
                // cout << "Parent found the child" << endl;
                break;
            }
        }
        // dummy = Rectangle::combine(node->parent->entries);
        // dummy.printRectangle("parent post-update");
        // Recalculate the parent's bounding rectangle to include all children
        updateBoundingRectangle(node->parent);
    }
}

void RStarTree::insert(Node* currentNode, const Rectangle& entry) {
    if (!currentNode) {
        cerr << "Error: currentNode is null!" << endl;
        return;
    }
    
    if (currentNode->isLeaf) {
        // Rectangle::combine(currentNode->entries).printRectangle("pre-update");
        currentNode->entries.push_back(entry);
        // Rectangle::combine(currentNode->entries).printRectangle("post-update");
        updateBoundingRectangle(currentNode);
        
        if (currentNode->entries.size() > maxEntries)
            splitNode(currentNode);
    } else {
        Node* subtree = chooseSubtree(currentNode, entry);
        if (subtree) {
            insert(subtree, entry);
        } else {
            cerr << "Error: No valid subtree found!" << endl;
        }
    }
    // validateTree();
}

void RStarTree::splitNode(Node* node) {
    if (!node || node->entries.empty()) {
        cerr << "Error: Invalid node in splitNode!" << endl;
        return;
    }

    // cout << "Splitting node with " << node->entries.size() << " entries." << endl;

    // Step 1: Choose Split Axis and Sort Entries
    int bestAxis = -1;
    size_t bestSplitIndex;
    float minOverlap = numeric_limits<float>::max();
    float minArea = numeric_limits<float>::max();

    int totalEntries = node->entries.size();
    vector<Rectangle> sortedEntries = node->entries;
    vector<size_t> sortedIndices(totalEntries);
    iota(sortedIndices.begin(), sortedIndices.end(), 0); // Fill with 0, 1, ..., totalEntries-1

    for (int axis = 0; axis < dimensions; ++axis) {
        // Sort entries by minCoords along the current axis and track indices
        sort(sortedIndices.begin(), sortedIndices.end(),
             [&sortedEntries, axis](size_t i, size_t j) {
                 return sortedEntries[i].minCoords[axis] < sortedEntries[j].minCoords[axis];
             });

        vector<Rectangle> sortedByAxis(totalEntries);
        for (size_t i = 0; i < totalEntries; ++i) {
            sortedByAxis[i] = sortedEntries[sortedIndices[i]];
        }

        // Step 2: Evaluate split points for the current axis
        for (size_t splitIndex = minEntries; splitIndex <= totalEntries - minEntries; ++splitIndex) {
            vector<Rectangle> leftEntries(sortedByAxis.begin(), sortedByAxis.begin() + splitIndex);
            vector<Rectangle> rightEntries(sortedByAxis.begin() + splitIndex, sortedByAxis.end());

            Rectangle leftBoundingRect = Rectangle::combine(leftEntries);
            Rectangle rightBoundingRect = Rectangle::combine(rightEntries);

            float overlap = leftBoundingRect.overlap(rightBoundingRect);
            float area = leftBoundingRect.area() + rightBoundingRect.area();

            if (overlap < minOverlap || (overlap == minOverlap && area < minArea)) {
                bestAxis = axis;
                minOverlap = overlap;
                minArea = area;
                bestSplitIndex = splitIndex;
            }
        }
    }

    if (bestAxis == -1) {
        cerr << "Error: No valid split found!" << endl;
        return;
    }

    // Step 3: Sort Entries and Children by the Best Axis
    sort(sortedIndices.begin(), sortedIndices.end(),
         [&sortedEntries, bestAxis](size_t i, size_t j) {
             return sortedEntries[i].minCoords[bestAxis] < sortedEntries[j].minCoords[bestAxis];
         });

    vector<Rectangle> sortedEntriesByBestAxis(totalEntries);
    for (size_t i = 0; i < totalEntries; ++i) {
        sortedEntriesByBestAxis[i] = sortedEntries[sortedIndices[i]];
    }

    // If the node is not a leaf, sort the children to match the sorted entries
    vector<Node*> sortedChildren;
    if (!node->isLeaf) {
        sortedChildren.resize(totalEntries);
        for (size_t i = 0; i < totalEntries; ++i) {
            sortedChildren[i] = node->children[sortedIndices[i]];
        }
        node->children = sortedChildren;
    }

    // Step 4: Perform the Split
    vector<Rectangle> leftEntries(sortedEntriesByBestAxis.begin(), sortedEntriesByBestAxis.begin() + bestSplitIndex);
    vector<Rectangle> rightEntries(sortedEntriesByBestAxis.begin() + bestSplitIndex, sortedEntriesByBestAxis.end());

    Node* newNode = new Node(node->isLeaf);

    if (!node->isLeaf) {
        vector<Node*> leftChildren, rightChildren;
        for (size_t i = 0; i < totalEntries; ++i) {
            if (i < bestSplitIndex) {
                leftChildren.push_back(node->children[i]);
                leftChildren.back()->parent = node;
            } else {
                rightChildren.push_back(node->children[i]);
                rightChildren.back()->parent = newNode;
            }
        }

        node->children = leftChildren;
        newNode->children = rightChildren;

        node->entries = leftEntries;
        newNode->entries = rightEntries;
    } else {
        node->entries = leftEntries;
        newNode->entries = rightEntries;
    }

    // cout << "Performing split: Axis " << bestAxis 
    //      << ", Best Split Index " << bestSplitIndex 
    //      << ", Left Group Size " << leftEntries.size() 
    //      << ", Right Group Size " << rightEntries.size() << endl;

    // Step 5: Update Parent Relationships
    if (!node->parent) {
        root = new Node(false);
        root->children.push_back(node);
        root->children.push_back(newNode);
        root->entries.push_back(Rectangle::combine(node->entries));
        root->entries.push_back(Rectangle::combine(newNode->entries));
        node->parent = root;
        newNode->parent = root;
    } else {
        Node* parent = node->parent;
        parent->children.push_back(newNode);
        parent->entries.push_back(Rectangle::combine(newNode->entries));
        newNode->parent = parent;

        // Update bounding rectangles recursively
        updateBoundingRectangle(parent);

        // Check for parent overflow
        if (parent->entries.size() > maxEntries) {
            splitNode(parent);
        }
    }
}



void RStarTree::checkHealth() const {
    checkHealth(root);

}

void RStarTree::checkHealth(const Node* node) const {
    if (!node) return;

    if (!node->isLeaf) {
        // Extract rectangles from children
        vector<Rectangle> childRects;
        for (const auto* child : node->children) {
            childRects.push_back(Rectangle::combine(child->entries));
        }

        // Combine child rectangles to validate parent bounding rectangle
        Rectangle combinedRect = Rectangle::combine(childRects);
        if (!node->entries[0].overlapCheck(combinedRect)) {
            cerr << "Validation Error: Parent rectangle does not encompass all children!" << endl;
            node->entries[0].printRectangle("Parent Rect");
            combinedRect.printRectangle("Combined Child Rects");
        }

        // Validate each child node
        for (const auto* child : node->children) {
            if (child->parent != node) {
                cerr << "Validation Error: Child's parent pointer is incorrect!" << endl;
                child->entries[0].printRectangle("Child Rect");
                node->entries[0].printRectangle("Parent Rect");
            }
            checkHealth(child); // Recursive call for child nodes
        }
    }
}



void RStarTree::printTree() const {
    printTree(root, 0);
    cout << "-------------" << endl;
}

void RStarTree::printTree(const Node* node, int depth) const {
    if (!node) return;

    // Print indentation for tree depth
    for (int i = 0; i < depth; ++i) cout << "  ";

    // Print node type
    if (node->isLeaf) {
        cout << "Leaf Node ";
        Rectangle::combine(node->entries).printRectangle("");
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
        for (const auto& rect : node->entries) {
            cout << "[(";
            for (float val : rect.minCoords) cout << val << ", ";
            cout << "), (";
            for (float val : rect.maxCoords) cout << val << ", ";
            cout << ")] ";
        }
    }
    cout << endl;

    // Recursively print children
    if (!node->isLeaf) {
        for (const auto* child : node->children)
            printTree(child, depth + 1);
    }
}


vector<Rectangle> RStarTree::rangeQuery(const Rectangle& query){
    vector<Rectangle> results;
    // query.printRectangle("query");
    if (root) 
        rangeQuery(root, query, results);
    return results;
}

void RStarTree::rangeQuery(Node* node, const Rectangle& query, vector<Rectangle>& results) {
    if (!node)
        return;

    for (size_t i = 0; i < node->entries.size(); ++i) {
        const Rectangle& currentEntry = node->entries[i];
        if (query.overlapCheck(currentEntry)) {
            if (node->isLeaf) {
                results.push_back(currentEntry);
            } else {
                rangeQuery(node->children[i], query, results);
            }
        }
    }
}



TreeStats RStarTree::getStats() {
    TreeStats stats;
    cout << "Capacity: " << maxEntries << endl;
    cout << "Minimum capacity: " << minEntries << endl;
    cout << "Dimensions: " << dimensions << endl;

    function<void(const Node*, int)> computeStats = [&](const Node* node, size_t currentDepth) {
        if (!node) return;

        stats.totalNodes++;
        if (node->isLeaf) {
            stats.leafNodes++;
            stats.totalDataEntries += node->entries.size();
        } else {
            stats.internalNodes++;
            for (const auto* child : node->children)
                computeStats(child, currentDepth + 1);
        }

        stats.height = max(stats.height, currentDepth);
    };

    computeStats(root, 1);
    return stats;
}