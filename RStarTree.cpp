#include "RStarTree.h"

Rectangle::Rectangle(int dimensions)
    : minCoords(dimensions, numeric_limits<float>::max()),
      maxCoords(dimensions, numeric_limits<float>::lowest()) {}

Rectangle::Rectangle(const vector<float>& min, const vector<float>& max)
    : minCoords(min), maxCoords(max) {}

float Rectangle::area() const {
    float result = 1.0f;
    for (size_t i = 0; i < minCoords.size(); ++i)
        result *= (maxCoords[i] - minCoords[i]);
    return result;
}

Rectangle Rectangle::combine(const Rectangle& other) const {
    vector<float> newMin(minCoords.size());
    vector<float> newMax(maxCoords.size());
    for (size_t i = 0; i < minCoords.size(); ++i) {
        newMin[i] = min(minCoords[i], other.minCoords[i]);
        newMax[i] = max(maxCoords[i], other.maxCoords[i]);
    }
    return Rectangle(newMin, newMax);
}

bool Rectangle::contains(const Rectangle& other) const {
    for (size_t i = 0; i < minCoords.size(); ++i) {
        if (minCoords[i] > other.minCoords[i] || maxCoords[i] < other.maxCoords[i])
            return false;
    }
    return true;
}

// Node Implementation
RStarTree::Node::Node(bool leaf) : isLeaf(leaf), parent(nullptr) {}

RStarTree::Node::~Node() {
    for (auto* child : children)
        delete child;
}

// RStarTree Implementation
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

void RStarTree::insert(Node* currentNode, const Rectangle& entry) {
    if (!currentNode) {
        cerr << "Error: currentNode is null!" << endl;
        return;
    }

    if (currentNode->isLeaf) {
        currentNode->entries.push_back(entry);
        if (currentNode->entries.size() > maxEntries)
            splitNode(currentNode);
    } else {
        Node* subtree = chooseSubtree(currentNode, entry);
        if (subtree)
            insert(subtree, entry);
        else 
            cerr << "Error: No valid subtree found!" << endl;
    }
}

RStarTree::Node* RStarTree::chooseSubtree(Node* currentNode, const Rectangle& entry) {
    if (currentNode->isLeaf) {
        cerr << "Error: chooseSubtree called on a leaf node!" << endl;
        return nullptr;
    }

    Node* bestChild = nullptr;
    float minAreaIncrease = numeric_limits<float>::max();

    for (size_t i = 0; i < currentNode->entries.size(); ++i) {
        Rectangle combined = currentNode->entries[i].combine(entry);
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

Rectangle RStarTree::computeBoundingRectangle(Node* node) const {
    vector<float> minCoords(dimensions, numeric_limits<float>::max());
    vector<float> maxCoords(dimensions, numeric_limits<float>::lowest());

    for (const auto& entry : node->entries) {
        for (size_t i = 0; i < dimensions; ++i) {
            minCoords[i] = min(minCoords[i], entry.minCoords[i]);
            maxCoords[i] = max(maxCoords[i], entry.maxCoords[i]);
        }
    }

    return Rectangle(minCoords, maxCoords);
}

void RStarTree::splitNode(Node* node) {
    if (!node || node->entries.empty()) {
        cerr << "Error: Invalid node in splitNode!" << endl;
        return;
    }

    Node* newNode = new Node(node->isLeaf);

    // Optimal Seed Picking: Find the pair of entries farthest apart
    size_t bestSeed1 = 0, bestSeed2 = 1;
    float maxSeparation = -1.0f;

    for (size_t i = 0; i < node->entries.size(); ++i) {
        for (size_t j = i + 1; j < node->entries.size(); ++j) {
            Rectangle combined = node->entries[i].combine(node->entries[j]);
            float separation = combined.area() - node->entries[i].area() - node->entries[j].area();
            if (separation > maxSeparation) {
                maxSeparation = separation;
                bestSeed1 = i;
                bestSeed2 = j;
            }
        }
    }

    // Move the seeds to separate nodes
    newNode->entries.push_back(node->entries[bestSeed2]);
    node->entries.erase(node->entries.begin() + bestSeed2);

    newNode->entries.push_back(node->entries[bestSeed1]);
    node->entries.erase(node->entries.begin() + bestSeed1);

    if (!node->isLeaf) {
        newNode->children.push_back(node->children[bestSeed2]);
        newNode->children.back()->parent = newNode;
        node->children.erase(node->children.begin() + bestSeed2);

        newNode->children.push_back(node->children[bestSeed1]);
        newNode->children.back()->parent = newNode;
        node->children.erase(node->children.begin() + bestSeed1);
    }

    // Distribute remaining entries
    for (const auto& entry : node->entries) {
        Rectangle combined1 = computeBoundingRectangle(node).combine(entry);
        Rectangle combined2 = computeBoundingRectangle(newNode).combine(entry);

        if (combined1.area() < combined2.area())
            node->entries.push_back(entry);
        else
            newNode->entries.push_back(entry);
    }

    if (!node->isLeaf) {
        for (size_t i = 0; i < node->children.size(); ++i) {
            newNode->children.push_back(node->children[i]);
            newNode->children.back()->parent = newNode;
        }
        node->children.clear();
    }

    // Update parent node
    if (!node->parent) {
        root = new Node(false);
        root->children.push_back(node);
        root->children.push_back(newNode);
        root->entries.push_back(computeBoundingRectangle(node));
        root->entries.push_back(computeBoundingRectangle(newNode));
        node->parent = root;
        newNode->parent = root;
    } else {
        Node* parent = node->parent;
        parent->children.push_back(newNode);
        parent->entries.push_back(computeBoundingRectangle(newNode));
        newNode->parent = parent;

        // If the parent overflows, split the parent
        if (parent->entries.size() > maxEntries)
            splitNode(parent);
    }

    // Clear node entries after splitting
    node->entries.clear();
}

void RStarTree::handleOverflow(Node* node) {
    if (!node || node->entries.size() <= maxEntries) return;

    cout << "Handling overflow for node with " << node->entries.size() << " entries.\n";

    // Compute the center of the node's bounding rectangle
    Rectangle boundingRect = computeBoundingRectangle(node);
    vector<float> center(dimensions);
    for (size_t i = 0; i < dimensions; ++i) 
        center[i] = (boundingRect.minCoords[i] + boundingRect.maxCoords[i]) / 2.0f;

    // Sort entries by distance from the center
    vector<pair<float, size_t>> distances;
    for (size_t i = 0; i < node->entries.size(); ++i) {
        float distance = 0.0f;
        for (size_t j = 0; j < dimensions; ++j) {
            float diff = (node->entries[i].minCoords[j] + node->entries[i].maxCoords[j]) / 2.0f - center[j];
            distance += diff * diff;
        }
        distances.push_back(make_pair(sqrt(distance), i));
    }

    sort(distances.begin(), distances.end(),
              [](const auto& a, const auto& b) { return a.first > b.first; });

    size_t numReinsert = static_cast<size_t>(0.3 * node->entries.size());
    vector<Rectangle> reinsertEntries;

    for (size_t i = 0; i < numReinsert; ++i) {
        size_t index = distances[i].second;
        reinsertEntries.push_back(node->entries[index]);
    }

    for (const auto& entry : reinsertEntries) {
        node->entries.erase(
            remove(node->entries.begin(), node->entries.end(), entry),
            node->entries.end()
        );
        insert(entry); // Reinsertion
    }

    if (node->entries.size() > maxEntries)
        splitNode(node);
}



void RStarTree::printTree() const {
    printTree(root, 0);
}

void RStarTree::printTree(Node* node, int depth) const {
    if (!node) return;

    for (int i = 0; i < depth; ++i) cout << "  ";
    cout << (node->isLeaf ? "Leaf" : "Internal") << " Node: ";
    for (const auto& rect : node->entries) {
        cout << "[(";
        for (float val : rect.minCoords) cout << val << " ";
        cout << "), (";
        for (float val : rect.maxCoords) cout << val << " ";
        cout << ")] ";
    }
    cout << "\n";

    for (auto* child : node->children)
        printTree(child, depth + 1);
}

vector<Rectangle> RStarTree::rangeQuery(const Rectangle& query) const {
    vector<Rectangle> results;
    if (root) 
        rangeQueryHelper(root, query, results);
    return results;
}

void RStarTree::rangeQueryHelper(Node* node, const Rectangle& query, vector<Rectangle>& results) const {
    if (!node) return;

    for (const auto& entry : node->entries) {
        if (query.overlap(entry) > 0) { // Check for overlap
            if (node->isLeaf)
                results.push_back(entry);
            else {
                for (Node* child : node->children)
                    rangeQueryHelper(child, query, results);
            }
        }
    }
}

float Rectangle::overlap(const Rectangle& other) const {
    for (size_t i = 0; i < minCoords.size(); ++i) {
        if (maxCoords[i] < other.minCoords[i] || minCoords[i] > other.maxCoords[i]) {
            // No overlap in this dimension
            return 0.0f;
        }
    }
    return 1.0f; // Overlap exists
}
