#include "RStarTree.h"

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
    insert(root, entry, true);
}

void RStarTree::insert(Node* currentNode, const Rectangle& entry, bool allowReinsertion) {
    if (!currentNode) {
        cerr << "Error: currentNode is null!" << endl;
        return;
    }
    
    if (currentNode->isLeaf) {
        currentNode->entries.push_back(entry);
        updateRectangles(currentNode);
        
        if (currentNode->entries.size() > maxEntries){
            if (allowReinsertion) reinsert(currentNode); 
            else splitNode(currentNode); 
        }
    } else {
        Node* subtree = chooseSubtree(currentNode, entry, false);
        if (subtree) insert(subtree, entry, allowReinsertion);  
        else cerr << "Error: No valid subtree found!" << endl;
    }
    // validateTree(); // Turn on for debugging.
}

Node* RStarTree::chooseSubtree(Node* currentNode, const Rectangle& entry, bool isBatch) {
    if (!currentNode || currentNode->isLeaf) {
        cerr << "Error: chooseSubtree called on an invalid or leaf node!" << endl;
        return nullptr;
    }

    if (currentNode->children.empty()) {
        cerr << "Error: currentNode has no children!" << endl;
        return nullptr;
    }

    // If it's a batch insertion return the current node
    if (isBatch && currentNode->children[0]->isLeaf)
        return currentNode;

    // Find the child with the minimal area increase
    Node* bestChild = nullptr;
    float minAreaIncrease = numeric_limits<float>::max();
    for (size_t i = 0; i < currentNode->entries.size(); ++i) {
        float areaIncrease = currentNode->entries[i].getAreaIncrease(entry);
        if (areaIncrease < minAreaIncrease) {
            minAreaIncrease = areaIncrease;
            bestChild = currentNode->children[i];
        }
    }
    return bestChild;
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
        insertLeaf(root, newNode, true);
    }
}

void RStarTree::insertLeaf(Node* currentNode, Node* newNode, bool allowReinsertion) {
    if (!currentNode) {
        cerr << "Error: currentNode is null!" << endl;
        return;
    }

    if (currentNode->children.empty()) {
        currentNode->children.push_back(newNode);
        currentNode->entries.push_back(Rectangle::combine(newNode->entries));
        newNode->parent = currentNode;
        currentNode->isLeaf = false;
        return;
    }

    // If children are leaves
    if (!currentNode->children.empty() && currentNode->children[0]->isLeaf) {
        currentNode->children.push_back(newNode);
        currentNode->entries.push_back(Rectangle::combine(newNode->entries));
        newNode->parent = currentNode;
        updateRectangles(currentNode);

        if (currentNode->children.size() > maxEntries)
            splitNode(currentNode); 

    } else {
        Node* subtree = chooseSubtree(currentNode, Rectangle::combine(newNode->entries), true);
        if (subtree) insertLeaf(subtree, newNode, allowReinsertion);
        else cerr << "Error: No valid subtree found in batchInsert!" << endl;
    }
}

void RStarTree::recursiveSTRSort(vector<Rectangle> &rects, int dim, int maxDim) {
    if (dim >= maxDim) return;

    // Sort rectangles by the current dimension
    sort(rects.begin(), rects.end(), [dim](const Rectangle& a, const Rectangle& b) {
        return a.minCoords[dim] < b.minCoords[dim];
    });

    // Split into groups and recursively sort along the next dimension
    size_t groupSize = (rects.size() + maxEntries - 1) / maxEntries; // Number of groups
    for (size_t i = 0; i < rects.size(); i += groupSize) {
        size_t end = min(i + groupSize, rects.size());
        vector<Rectangle> group(rects.begin() + i, rects.begin() + end);
        recursiveSTRSort(group, dim + 1, maxDim);
    }
}

void RStarTree::bulkLoad(vector<Rectangle>& rectangles) {
    if (rectangles.empty()) return;

    // Sort rectangles recursively across all dimensions
    recursiveSTRSort(rectangles, 0, dimensions);

    // Create leaf nodes
    vector<Node*> leafNodes;
    for (size_t i = 0; i < rectangles.size(); i += maxEntries) {
        size_t end = min(i + maxEntries, rectangles.size());
        Node* leafNode = new Node(true);
        leafNode->entries.insert(leafNode->entries.end(), rectangles.begin() + i, rectangles.begin() + end);
        leafNodes.push_back(leafNode);
    }

    // Build the tree bottom-up
    vector<Node*> currentLevel = leafNodes;
    vector<Node*> nextLevel;

    while (currentLevel.size() > 1) {
        nextLevel.clear();

        for (size_t i = 0; i < currentLevel.size(); i += maxEntries) {
            size_t end = min(i + maxEntries, currentLevel.size());
            Node* parentNode = new Node(false);
            for (size_t j = i; j < end; ++j) {
                parentNode->children.push_back(currentLevel[j]);
                parentNode->entries.push_back(Rectangle::combine(currentLevel[j]->entries));
                currentLevel[j]->parent = parentNode;
            }
            nextLevel.push_back(parentNode);
        }

        swap(currentLevel, nextLevel); // Swap buffers for the next iteration
    }

    root = currentLevel.front(); // Set the final root node
}

void RStarTree::updateRectangles(Node* node) {
    if (!node || node->entries.empty()) return;

    if (node->parent) {
        Rectangle newBoundingRect = Rectangle::combine(node->entries);
        for (size_t i = 0; i < node->parent->children.size(); ++i) {
            if (node->parent->children[i] == node) {
                node->parent->entries[i] = newBoundingRect;
                break;
            }
        }
        updateRectangles(node->parent);
    }
}

void RStarTree::reinsert(Node* node) {

    // Reinsert the farthest 30% of entries
    size_t reinsertCount = static_cast<size_t>(node->entries.size() * 0.3); 
    if (reinsertCount == 0) return;

    // Sort entries by distance from the center
    vector<float> center = Rectangle::combine(node->entries).getCenter();
    vector<pair<float, size_t>> distances; // {distance, index}
    for (size_t i = 0; i < node->entries.size(); ++i) {
        float distance = 0.0f;
        vector<float> entryCenter = node->entries[i].getCenter();
        for (size_t d = 0; d < center.size(); ++d) 
            distance += (entryCenter[d] - center[d]) * (entryCenter[d] - center[d]);
        distances.emplace_back(distance, i);
    }
    sort(distances.rbegin(), distances.rend());

    vector<Rectangle> reinsertEntries;
    for (size_t i = 0; i < reinsertCount; ++i) 
        reinsertEntries.push_back(node->entries[distances[i].second]);

    // Remove the selected entries from the node
    vector<Rectangle> remainingEntries;
    for (size_t i = reinsertCount; i < distances.size(); ++i) 
        remainingEntries.push_back(node->entries[distances[i].second]);
    node->entries = remainingEntries;

    // Reinsert
    for (const auto& entry : reinsertEntries)
        insert(root, entry, false);

    // If the node still overflows, split it
    if (node->entries.size() > maxEntries)
        splitNode(node);
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

            float overlap = leftBoundingRect.overlap(rightBoundingRect);
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
    sort(sortedIndices.begin(), sortedIndices.end(),
         [&sortedEntries, bestAxis](size_t i, size_t j) {
             return sortedEntries[i].minCoords[bestAxis] < sortedEntries[j].minCoords[bestAxis];
         });

    vector<Rectangle> sortedEntriesByBestAxis(sortedEntries.size());
    for (size_t i = 0; i < sortedIndices.size(); ++i)
        sortedEntriesByBestAxis[i] = sortedEntries[sortedIndices[i]];

    if (!node->isLeaf) {
        vector<Node*> sortedChildren(sortedEntries.size());
        for (size_t i = 0; i < sortedIndices.size(); ++i)
            sortedChildren[i] = node->children[sortedIndices[i]];
        node->children = sortedChildren;
    }

    node->entries = sortedEntriesByBestAxis;
}

void RStarTree::splitNode(Node* node) {
    if (!node || node->entries.empty()) {
        cerr << "Error: Invalid node in splitNode!" << endl;
        return;
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
    }

    node->entries = leftEntries;
    newNode->entries = rightEntries;

    // Handle Parent
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

        updateRectangles(parent);

        if (parent->entries.size() > maxEntries)
            splitNode(parent);
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
                if (child->parent != node) {
                    cerr << "Validation Error: Child's parent pointer is incorrect!" << endl;
                    child->entries[0].printRectangle("Child Rect");
                    node->entries[0].printRectangle("Parent Rect");
                }
                validateNode(child);
            }
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
    treeInfo.totalNodeVisits = info.totalNodeVisits;
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