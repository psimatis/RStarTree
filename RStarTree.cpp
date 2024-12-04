#include "RStarTree.h"
#include <algorithm>
#include <iostream>

// Rectangle Implementation
Rectangle::Rectangle(int dimensions)
    : minCoords(dimensions, std::numeric_limits<float>::max()),
      maxCoords(dimensions, std::numeric_limits<float>::lowest()) {}

Rectangle::Rectangle(const std::vector<float>& min, const std::vector<float>& max)
    : minCoords(min), maxCoords(max) {}

float Rectangle::area() const {
    float result = 1.0f;
    for (size_t i = 0; i < minCoords.size(); ++i) {
        result *= (maxCoords[i] - minCoords[i]);
    }
    return result;
}

float Rectangle::overlap(const Rectangle& other) const {
    float result = 1.0f;
    for (size_t i = 0; i < minCoords.size(); ++i) {
        float overlapMin = std::max(minCoords[i], other.minCoords[i]);
        float overlapMax = std::min(maxCoords[i], other.maxCoords[i]);
        if (overlapMax >= overlapMin) {  // Include touching boundaries
            result *= (overlapMax - overlapMin);
        } else {
            return 0.0f;  // No overlap
        }
    }
    return result;
}

Rectangle Rectangle::combine(const Rectangle& other) const {
    std::vector<float> newMin(minCoords.size());
    std::vector<float> newMax(maxCoords.size());
    for (size_t i = 0; i < minCoords.size(); ++i) {
        newMin[i] = std::min(minCoords[i], other.minCoords[i]);
        newMax[i] = std::max(maxCoords[i], other.maxCoords[i]);
    }
    return Rectangle(newMin, newMax);
}

bool Rectangle::contains(const Rectangle& other) const {
    for (size_t i = 0; i < minCoords.size(); ++i) {
        if (minCoords[i] > other.minCoords[i] || maxCoords[i] < other.maxCoords[i]) {
            return false;
        }
    }
    return true;
}

// Node Implementation
RStarTree::Node::Node(bool leaf) : isLeaf(leaf), parent(nullptr) {}

RStarTree::Node::~Node() {
    for (auto* child : children) {
        delete child;
    }
}

// RStarTree Implementation
RStarTree::RStarTree(int maxEntries, int dimensions)
    : maxEntries(maxEntries), minEntries(maxEntries / 2), dimensions(dimensions) {
    root = new Node(true);
    std::cout << "Initialized R*-Tree with root node.\n";
}

RStarTree::~RStarTree() {
    delete root;
}

void RStarTree::insert(const Rectangle& entry) {
    insert(root, entry);
}

void RStarTree::insert(Node* currentNode, const Rectangle& entry) {
    if (currentNode->isLeaf) {
        currentNode->entries.push_back(entry);
        if (currentNode->entries.size() > maxEntries) {
            splitNode(currentNode);
        }
    } else {
        Node* subtree = chooseSubtree(currentNode, entry);
        insert(subtree, entry);
    }
}

RStarTree::Node* RStarTree::chooseSubtree(Node* currentNode, const Rectangle& entry) {
    Node* bestNode = nullptr;
    float minIncrease = std::numeric_limits<float>::max();

    for (size_t i = 0; i < currentNode->entries.size(); ++i) {
        Rectangle combined = currentNode->entries[i].combine(entry);
        float increase = combined.area() - currentNode->entries[i].area();
        if (increase < minIncrease) {
            minIncrease = increase;
            bestNode = currentNode->children[i];
        }
    }

    return bestNode;
}

void RStarTree::splitNode(Node* node) {
    Node* newNode = new Node(node->isLeaf);
    size_t splitIndex = node->entries.size() / 2;

    // Move half of the entries to the new node
    for (size_t i = splitIndex; i < node->entries.size(); ++i) {
        newNode->entries.push_back(node->entries[i]);
    }
    node->entries.resize(splitIndex);

    if (node->parent == nullptr) {
        // Create a new root if the node is the root
        root = new Node(false);
        root->children.push_back(node);
        root->children.push_back(newNode);

        node->parent = root;
        newNode->parent = root;

        root->entries.push_back(computeBoundingRectangle(node));
        root->entries.push_back(computeBoundingRectangle(newNode));
    } else {
        // Add the new node to the parent
        node->parent->children.push_back(newNode);
        newNode->parent = node->parent;

        // Update parent's bounding rectangles
        size_t childIndex = std::distance(
            node->parent->children.begin(),
            std::find(node->parent->children.begin(), node->parent->children.end(), static_cast<RStarTree::Node*>(node))
        );

        node->parent->entries[childIndex] = computeBoundingRectangle(node);
        node->parent->entries.push_back(computeBoundingRectangle(newNode));

        if (node->parent->entries.size() > maxEntries) {
            splitNode(node->parent);
        }
    }
}

void RStarTree::adjustTree(Node* node) {
    while (node != nullptr) {
        if (node->parent != nullptr) {
            size_t childIndex = std::distance(
                node->parent->children.begin(),
                std::find(node->parent->children.begin(), node->parent->children.end(), static_cast<RStarTree::Node*>(node))
            );
            node->parent->entries[childIndex] = computeBoundingRectangle(node);
        }

        if (node->entries.size() > maxEntries) {
            splitNode(node);
        }

        node = node->parent;
    }
}

Rectangle RStarTree::computeBoundingRectangle(Node* node) const {
    std::vector<float> minCoords(dimensions, std::numeric_limits<float>::max());
    std::vector<float> maxCoords(dimensions, std::numeric_limits<float>::lowest());

    for (const auto& entry : node->entries) {
        for (size_t i = 0; i < dimensions; ++i) {
            minCoords[i] = std::min(minCoords[i], entry.minCoords[i]);
            maxCoords[i] = std::max(maxCoords[i], entry.maxCoords[i]);
        }
    }

    return Rectangle(minCoords, maxCoords);
}

std::vector<Rectangle> RStarTree::rangeQuery(const Rectangle& query) const {
    std::vector<Rectangle> results;
    if (root) {
        rangeQueryHelper(root, query, results);
    }
    return results;
}

void RStarTree::rangeQueryHelper(Node* node, const Rectangle& query, std::vector<Rectangle>& results) const {
    for (size_t i = 0; i < node->entries.size(); ++i) {
        if (query.overlap(node->entries[i]) > 0) {  // Check for overlap
            if (node->isLeaf) {
                results.push_back(node->entries[i]);
            } else {
                rangeQueryHelper(node->children[i], query, results);
            }
        }
    }
}

void RStarTree::printTree() const {
    printTree(root, 0);
}

void RStarTree::printTree(Node* node, int depth) const {
    for (int i = 0; i < depth; ++i) {
        std::cout << "  ";
    }
    std::cout << (node->isLeaf ? "Leaf" : "Internal") << " Node: ";
    for (const auto& rect : node->entries) {
        std::cout << "[(";
        for (float val : rect.minCoords) std::cout << val << " ";
        std::cout << "), (";
        for (float val : rect.maxCoords) std::cout << val << " ";
        std::cout << ")] ";
    }
    std::cout << "\n";

    for (auto* child : node->children) {
        printTree(child, depth + 1);
    }
}
