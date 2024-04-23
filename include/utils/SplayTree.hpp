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
};

template<typename K, typename V>
Node<K, V>* Node<K, V>::rightmost() {
    Node<K, V>* node = this;
    while (node->right) node = node->right;
    return node;
}

template<typename K, typename V, typename Comparator = std::less<K>>
class SplayTree {
public:
    Node<K, V>* root;
    Comparator comp;

    SplayTree() : root(nullptr), comp(Comparator()) {}

    explicit SplayTree(Comparator comparator) : root(nullptr), comp(comparator) {}

    void add(K key, V value);

    Node<K, V>* find(K key, bool doSplay = true);

    void remove(K key);

    std::string toString();

private:
    void rotateLeft(Node<K, V>* node, Node<K, V>* rootParent = nullptr);

    void rotateRight(Node<K, V>* node, Node<K, V>* rootParent = nullptr);

    void splay(Node<K, V>* node, Node<K, V>* rootParent = nullptr);

    void replace(Node<K, V>* x, Node<K, V>* y);

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
std::string SplayTree<K, V, Comparator>::toString() {
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
        } else if (parentLeft /* && grandparent right */) {
            // ">" Zig-zag
            rotateRight(node->parent, rootParent);
            rotateLeft(node->parent, rootParent);
        } else if (/* parent right && */ !grandparentLeft) {
            // Right-sided zig-zig
            rotateLeft(node->parent->parent, rootParent);
            rotateLeft(node->parent, rootParent);
        } else /* if (parent right && grandparent right) */ {
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
        node = comp(node->key, key) ? node->right : node->left;
    }

    node = new Node<K, V>(key, value);
    node->parent = parent;

    if (!parent) root = node;
    else if (comp(parent->key, key)) parent->right = node;
    else parent->left = node;

    splay(node);
}


template<typename K, typename V, typename Comparator>
Node<K, V>* SplayTree<K, V, Comparator>::find(K key, bool doSplay) {
    Node<K, V>* node = root;
    while (node) {
        if (node->key == key) {
            if (doSplay) splay(node);
            return node;
        }

        node = comp(node->key, key) ? node->right : node->left;
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
    Node<K, V>* x = find(key, false);
    if (!x) return;

    if (!x->left) replace(x, x->right);
    else if (!x->right) replace(x, x->left);
    else {
        // Join
        Node<K, V>* leftSubtree = x->left;
        Node<K, V>* rightSubtree = x->right;
        Node<K, V>* newRoot = join(leftSubtree, rightSubtree);
        replace(x, newRoot);

        // Splay parent
    }
    splay(x->parent);

    x->left = nullptr;
    x->right = nullptr;
    delete x;
}

#endif
