#ifndef SPLAYTREE_HPP
#define SPLAYTREE_HPP

#include <iostream>
#include <sstream>

void splayTreeTest1();

void splayTreeTest2();

void splayTreeTest3_4();

template<typename K, typename V = std::less<K>>
class Node {
public:
    K key;
    V value;
    Node<K, V>* left {nullptr};
    Node<K, V>* right {nullptr};
    Node<K, V>* parent {nullptr};

    explicit Node(K key, V data = V()) : key(key), value(data) {};

    void setLeftChild(Node<K, V>* child);

    void setRightChild(Node<K, V>* child);

    Node<K, V>* rightmost();

    Node<K, V>* leftmost();
};

template<typename K, typename V>
Node<K, V>* Node<K, V>::rightmost() {
    Node<K, V>* node = this;
    while (node->right) node = node->right;
    return node;
}

template<typename K, typename V>
Node<K, V>* Node<K, V>::leftmost() {
    Node<K, V>* node = this;
    while (node->left) node = node->left;
    return node;
}


template<typename K, typename V, typename Comparator = std::less<K>>
class SplayTree {
public:
    Node<K, V>* root;
    Comparator compare;

    SplayTree() : root(nullptr), compare(Comparator()) {}

    explicit SplayTree(Comparator comparator) : root(nullptr), compare(comparator) {}

    void add(K key, V value);

    Node<K, V>* get(K key);

    // Like get(), but without side effects (no splaying)
    Node<K, V>* search(K key) const;

    void remove(K key);

    void removeNode(Node<K, V>* node);

    [[nodiscard]] std::string toString() const;

    void replace(Node<K, V>* x, Node<K, V>* y);

private:
    void rotateLeft(Node<K, V>* node, Node<K, V>* rootParent = nullptr);

    void rotateRight(Node<K, V>* node, Node<K, V>* rootParent = nullptr);

    void splay(Node<K, V>* node, Node<K, V>* rootParent = nullptr);

    static void toStringRecursive(Node<K, V>* node, int depth, std::stringstream &oss);

    Node<K, V>* join(Node<K, V>* left, Node<K, V>* right);
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

template<typename K, typename V, typename Comparator>
std::string SplayTree<K, V, Comparator>::toString() const {
    std::stringstream oss;
    toStringRecursive(root, 0, oss);
    return oss.str();
}

template<typename K, typename V, typename Comparator>
void SplayTree<K, V, Comparator>::toStringRecursive(Node<K, V>* node, int depth, std::stringstream &oss) {
    for (int i = 0; i < depth; i++) oss << "|\t";
    if (node == nullptr) {
        oss << "--" << std::endl;
        return;
    }
    oss << node->key << "->" << node->value << std::endl;
    if (node->left == nullptr && node->right == nullptr) return;
    toStringRecursive(node->left, depth + 1, oss);
    toStringRecursive(node->right, depth + 1, oss);
}


template<typename K, typename V, typename Comparator>
void SplayTree<K, V, Comparator>::rotateLeft(Node<K, V>* node, Node<K, V>* rootParent) {
    Node<K, V>* y = node->right;
    if (y) {
        node->right = y->left;
        if (y->left) y->left->parent = node;
        y->parent = node->parent;
    }
    if (node->parent == rootParent && rootParent == nullptr) root = y;
    else if (node == node->parent->left) node->parent->left = y;
    else node->parent->right = y;
    if (y) y->left = node;
    node->parent = y;
}


template<typename K, typename V, typename Comparator>
void SplayTree<K, V, Comparator>::rotateRight(Node<K, V>* node, Node<K, V>* rootParent) {
    Node<K, V>* leftChild = node->left;

    if (leftChild) {
        node->left = leftChild->right;
        if (leftChild->right) leftChild->right->parent = node;
        leftChild->parent = node->parent;
    }

    if (node->parent == rootParent && rootParent == nullptr) root = leftChild;
    else if (node == node->parent->left) node->parent->left = leftChild;
    else node->parent->right = leftChild;
    if (leftChild) leftChild->right = node;
    node->parent = leftChild;
}


template<typename K, typename V, typename Comparator>
void SplayTree<K, V, Comparator>::splay(Node<K, V>* node, Node<K, V>* rootParent) {
    if (node == nullptr) return;
    while (node->parent != rootParent) {
        bool parentLeft = node->parent->left == node;

        // If parent is root, directly rotate to the root
        if (node->parent->parent == rootParent) {
            if (parentLeft) rotateRight(node->parent, rootParent);
            else rotateLeft(node->parent, rootParent);
            continue;
        }

        bool grandparentLeft = node->parent->parent->left == node->parent;

        // Check for each zig/zag
        if (parentLeft && grandparentLeft) {
            // Left-sided zig-zig
            rotateRight(node->parent->parent, rootParent);
            rotateRight(node->parent, rootParent);
        } else if (parentLeft /* && grandparent rightSite */) {
            // ">" Zig-zag
            rotateRight(node->parent, rootParent);
            rotateLeft(node->parent, rootParent);
        } else if (/* parent rightSite && */ !grandparentLeft) {
            // Right-sided zig-zig
            rotateLeft(node->parent->parent, rootParent);
            rotateLeft(node->parent, rootParent);
        } else /* if (parent rightSite && grandparent rightSite) */ {
            // "<" Zig-zag
            rotateLeft(node->parent, rootParent);
            rotateRight(node->parent, rootParent);
        }
    }
}


template<typename K, typename V, typename Comparator>
void SplayTree<K, V, Comparator>::add(K key, V value) {
    Node<K, V>* node = root;
    Node<K, V>* parent = nullptr;

    while (node) {
        parent = node;
        node = compare(node->key, key) ? node->right : node->left;
    }

    auto newNode = new Node<K, V>(key, value);

    if (!parent) root = newNode;
    else if (compare(key, parent->key)) parent->setLeftChild(newNode);
    else parent->setRightChild(newNode);

    splay(newNode);
}


template<typename K, typename V, typename Comparator>
Node<K, V>* SplayTree<K, V, Comparator>::get(K key) {
    Node<K, V>* node = this->search(key);
    if (node) splay(node);
    return node;
}


template<typename K, typename V, typename Comparator>
Node<K, V>* SplayTree<K, V, Comparator>::search(K key) const {
    Node<K, V>* node = root;
    while (node) {
        if (node->key == key) return node;
        node = compare(node->key, key) ? node->right : node->left;
    }
    return nullptr;
}


template<typename K, typename V, typename Comparator>
void SplayTree<K, V, Comparator>::replace(Node<K, V>* x, Node<K, V>* y) {
    if (x == root) root = y;
    else if (x == x->parent->left) x->parent->setLeftChild(y);
    else x->parent->setRightChild(y);
}


template<typename K, typename V, typename Comparator>
Node<K, V>* SplayTree<K, V, Comparator>::join(Node<K, V>* left, Node<K, V>* right) {
    Node<K, V>* leftParent = left->parent;
    Node<K, V>* newRoot = left->rightmost();
    splay(newRoot, leftParent);
    newRoot->setRightChild(right);
    return newRoot;
}


template<typename K, typename V, typename Comparator>
void SplayTree<K, V, Comparator>::remove(K key) {
    Node<K, V>* x = search(key);
    if (!x) return;

    Node<K, V>* newRoot;
    if (!x->left) newRoot = x->right;
    else if (!x->right) newRoot = x->left;
    else newRoot = join(x->left, x->right);

    replace(x, newRoot);
    splay(x->parent);

    x->left = nullptr;
    x->right = nullptr;
    delete x;
}


template<typename K, typename V, typename Comparator>
void SplayTree<K, V, Comparator>::removeNode(Node<K, V>* node) {
    if (node == nullptr) return;

    Node<K, V>* newRoot;
    if (!node->left) newRoot = node->right;
    else if (!node->right) newRoot = node->left;
    else newRoot = join(node->left, node->right);

    replace(node, newRoot);
    splay(node->parent);

    node->left = nullptr;
    node->right = nullptr;
    delete node;
}

#endif
