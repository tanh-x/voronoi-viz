#ifndef SPLAYTREE_HPP
#define SPLAYTREE_HPP

#include <iostream>
#include <sstream>

template<typename K, typename V>
class Node {
public:
    K key;
    V value;
    Node<K, V>* left {nullptr};
    Node<K, V>* right {nullptr};
    Node<K, V>* parent {nullptr};

    void setLeftChild(Node<K, V>* child);

    void setRightChild(Node<K, V>* child);

    explicit Node(K key, V data = V()) : key(key), value(data) {};
};

template<typename K, typename V>
class SplayTree {
public:
    Node<K, V>* root;

    SplayTree() : root(nullptr) {}

    void add(K key, V value);

    Node<K, V>* find(K key);

    void remove(K key);

    std::string toString();

private:
    void rotateLeft(Node<K, V>* x);

    void rotateRight(Node<K, V>* x);

    void splay(Node<K, V>* node);

    static void printTree(Node<K, V>* node, int depth, std::stringstream &oss);
};


template<typename K, typename V>
void Node<K, V>::setLeftChild(Node<K, V>* child) {
    this->left = child;
    if (child) child->parent = this;
}

template<typename K, typename V>
void Node<K, V>::setRightChild(Node<K, V>* child) {
    this->right = child;
    if (child) child->parent = this;
}

template<typename K, typename V>
std::string SplayTree<K, V>::toString() {
    std::stringstream oss;
    printTree(root, 0, oss);
    return oss.str();
}

template<typename K, typename V>
void SplayTree<K, V>::printTree(Node<K, V>* node, int depth, std::stringstream &oss) {
    for (int i = 0; i < depth; i++) oss << "|\t";
    if (node == nullptr) {
        oss << "--" << std::endl;
        return;
    }
    oss << node->key << "->" << node->value << std::endl;
    if (node->left == nullptr && node->right == nullptr) return;
    printTree(node->left, depth + 1, oss);
    printTree(node->right, depth + 1, oss);
}


template<typename K, typename V>
void SplayTree<K, V>::rotateLeft(Node<K, V>* x) {
    Node<K, V>* y = x->right;
    if (y) {
        x->right = y->left;
        if (y->left) y->left->parent = x;
        y->parent = x->parent;
    }
    if (!x->parent) root = y;
    else if (x == x->parent->left) x->parent->left = y;
    else x->parent->right = y;
    if (y) y->left = x;
    x->parent = y;
}


template<typename K, typename V>
void SplayTree<K, V>::rotateRight(Node<K, V>* x) {
    Node<K, V>* leftChild = x->left;

    if (leftChild) {
        x->left = leftChild->right;
        if (leftChild->right) leftChild->right->parent = x;
        leftChild->parent = x->parent;
    }

    if (!x->parent) root = leftChild;
    else if (x == x->parent->left) x->parent->left = leftChild;
    else x->parent->right = leftChild;
    if (leftChild) leftChild->right = x;
    x->parent = leftChild;
}


template<typename K, typename V>
void SplayTree<K, V>::splay(Node<K, V>* node) {
    while (node->parent) {
        bool parentLeft = node->parent->left == node;

        // If parent is root, directly rotate to the root
        if (!node->parent->parent) {
            if (parentLeft) rotateRight(node->parent);
            else rotateLeft(node->parent);
            continue;
        }

        bool grandparentLeft = node->parent->parent->left == node->parent;

        // Check for each zig/zag
        if (parentLeft && grandparentLeft) {
            // Left-sided zig-zig
            rotateRight(node->parent->parent);
            rotateRight(node->parent);
        } else if (parentLeft /* && grandparent right */) {
            // ">" Zig-zag
            rotateRight(node->parent);
            rotateLeft(node->parent);
        } else if (/* parent right && */ !grandparentLeft) {
            // Right-sided zig-zig
            rotateLeft(node->parent->parent);
            rotateLeft(node->parent);
        } else /* if (parent right && grandparent right) */ {
            // "<" Zig-zag
            rotateLeft(node->parent);
            rotateRight(node->parent);
        }
    }
}


template<typename K, typename V>
void SplayTree<K, V>::add(K key, V value) {
    Node<K, V>* node = root;
    Node<K, V>* parent = nullptr;

    while (node) {
        parent = node;
        node = (node->key < key) ? node->right : node->left;
    }

    node = new Node<K, V>(key, value);
    node->parent = parent;

    if (!parent) root = node;
    else if (parent->key < node->key) parent->right = node;
    else parent->left = node;

    splay(node);
}


template<typename K, typename V>
Node<K, V>* SplayTree<K, V>::find(K key) {
    Node<K, V>* node = root;
    while (node) {
        if (node->key == key) {
            splay(node);
            return node;
        }
        node = (node->key < key) ? node->right : node->left;
    }
    return nullptr;
}


template<typename K, typename V>
void SplayTree<K, V>::remove(K key) {
    Node<K, V>* node = find(key);
    if (!node) return;

    splay(node);

    if (!node->left) replace(node, node->right);
    else if (!node->right) replace(node, node->left);
    else {
        Node<K, V>* minNode = minimum(node->right);
        if (minNode->parent != node) {
            replace(minNode, minNode->right);
            minNode->right = node->right;
            minNode->right->parent = minNode;
        }

        replace(node, minNode);
        minNode->left = node->left;
        minNode->left->parent = minNode;
    }

    delete node;
}

#endif
