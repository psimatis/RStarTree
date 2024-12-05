#include "RStarTree.h"

/////////////////////
// Rectangle
/////////////////////
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

Rectangle Rectangle::combine(const vector<Rectangle>& rectangles) {
    if (rectangles.empty()) 
        return Rectangle(); // Return empty rectangle if no input

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

bool Rectangle::overlap(const Rectangle& other) const {
    for (size_t i = 0; i < minCoords.size(); ++i) {
        if (maxCoords[i] < other.minCoords[i] || minCoords[i] > other.maxCoords[i])
            return false;
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


void RStarTree::splitNode(Node* node) {
    if (!node || node->entries.empty()) {
        cerr << "Error: Invalid node in splitNode!" << endl;
        return;
    }

    Node* newNode = new Node(node->isLeaf);
    size_t splitIndex = node->entries.size() / 2;

    // Move half of the entries and children to the new node
    for (size_t i = splitIndex; i < node->entries.size(); ++i) {
        newNode->entries.push_back(node->entries[i]);
        if (!node->isLeaf) {
            newNode->children.push_back(node->children[i]);
            newNode->children.back()->parent = newNode;
        }
    }

    node->entries.resize(splitIndex);
    if (!node->isLeaf)
        node->children.resize(splitIndex);

    // Update the parent node
    if (!node->parent) {
        // Create a new root if splitting the root
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

        // If the parent overflows, split the parent
        if (parent->entries.size() > maxEntries) {
            splitNode(parent);
        }
    }
}


void RStarTree::handleOverflow(Node* node) {
    if (!node || node->entries.size() <= maxEntries) return;

    cout << "Handling overflow for node with " << node->entries.size() << " entries.\n";

    // Compute the center of the node's bounding rectangle
    Rectangle boundingRect = Rectangle::combine(node->entries);
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

void RStarTree::printTree(const Node* node, int depth) const {
    if (!node) return;

    // Print indentation for tree depth
    for (int i = 0; i < depth; ++i) cout << "  ";

    // Print node type
    if (node->isLeaf) {
        cout << "Leaf Node: ";
        for (const auto& rect : node->entries) {
            cout << "[(";
            for (float val : rect.minCoords) cout << val << ", ";
            cout << "), (";
            for (float val : rect.maxCoords) cout << val << ", ";
            cout << ")] ";
        }
    } else {
        cout << "Internal Node: ";
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


vector<Rectangle> RStarTree::rangeQuery(const Rectangle& query) const {
    vector<Rectangle> results;
    if (root) 
        rangeQuery(root, query, results);
    return results;
}

void RStarTree::rangeQuery(Node* node, const Rectangle& query, std::vector<Rectangle>& results) const {
    if (!node) return;

    for (size_t i = 0; i < node->entries.size(); ++i) {
        if (query.overlap(node->entries[i]) > 0) {
            if (node->isLeaf)
                results.push_back(node->entries[i]);
            else
                rangeQuery(node->children[i], query, results);
        }
    }
}


TreeStats RStarTree::getStats() const {
    TreeStats stats;

    std::function<void(const Node*, int)> computeStats = [&](const Node* node, int currentDepth) {
        if (!node) return;

        stats.totalNodes++;
        if (node->isLeaf) {
            stats.leafNodes++;
            stats.totalDataEntries += node->entries.size();
        } else {
            stats.internalNodes++;
            for (const auto* child : node->children) {
                computeStats(child, currentDepth + 1);
            }
        }

        stats.height = std::max(stats.height, currentDepth);
    };

    computeStats(root, 1); // Start with the root node and depth 1
    return stats;
}