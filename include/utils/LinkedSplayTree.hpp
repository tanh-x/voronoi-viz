#ifndef VORONOI_VIZ_LINKEDSPLAYTREE_HPP
#define VORONOI_VIZ_LINKEDSPLAYTREE_HPP

#include <iostream>
#include <sstream>
#include <functional>

void linkedSplayTreeTest1();

void linkedSplayTreeTest2_3();

template<typename K, typename V = std::less<K>>
class LinkedNode {
public:
    K key;
    V value;

    LinkedNode<K, V>* left {nullptr};
    LinkedNode<K, V>* right {nullptr};
    LinkedNode<K, V>* parent {nullptr};
    LinkedNode<K, V>* prev {nullptr};
    LinkedNode<K, V>* next {nullptr};

    explicit LinkedNode(K key, V data = V()) : key(key), value(data) {};

    void setLeftChild(LinkedNode<K, V>* child);

    void setRightChild(LinkedNode<K, V>* child);

    void linkNext(LinkedNode<K, V>* nextNode);

    void linkPrev(LinkedNode<K, V>* prevNode);

    void wedgeBefore(LinkedNode<K, V>* node);

    void wedgeAfter(LinkedNode<K, V>* node);

    void dissolveLinks();

    LinkedNode<K, V>* rightmost();

    LinkedNode<K, V>* leftmost();
};

template<typename K, typename V>
void LinkedNode<K, V>::linkNext(LinkedNode<K, V>* nextNode) {
    this->next = nextNode;
    if (nextNode != nullptr) nextNode->prev = this;
}


template<typename K, typename V>
void LinkedNode<K, V>::linkPrev(LinkedNode<K, V>* prevNode) {
    this->prev = prevNode;
    if (prevNode != nullptr) prevNode->next = this;
}


template<typename K, typename V>
void LinkedNode<K, V>::wedgeBefore(LinkedNode<K, V>* node) {
    if (node == nullptr) throw std::invalid_argument("Node cannot be null");
    auto* prevNode = node->prev;

    this->linkPrev(prevNode);
    this->linkNext(node);
}


template<typename K, typename V>
void LinkedNode<K, V>::wedgeAfter(LinkedNode<K, V>* node) {
    if (node == nullptr) throw std::invalid_argument("Node cannot be null");
    auto* nextNode = node->next;

    this->linkPrev(node);
    this->linkNext(nextNode);
}

template<typename K, typename V>
void LinkedNode<K, V>::dissolveLinks() {
    if (this->prev) this->prev->next = this->next;
    if (this->next) this->next->prev = this->prev;
}


template<typename K, typename V>
void LinkedNode<K, V>::setLeftChild(LinkedNode<K, V>* child) {
    this->left = child;
    if (child) child->parent = this;
}

template<typename K, typename V>
void LinkedNode<K, V>::setRightChild(LinkedNode<K, V>* child) {
    this->right = child;
    if (child) child->parent = this;
}

template<typename K, typename V>
LinkedNode<K, V>* LinkedNode<K, V>::rightmost() {
    LinkedNode<K, V>* node = this;
    while (node->right) node = node->right;
    return node;
}

template<typename K, typename V>
LinkedNode<K, V>* LinkedNode<K, V>::leftmost() {
    LinkedNode<K, V>* node = this;
    while (node->left) node = node->left;
    return node;
}


template<typename K, typename V, typename Comparator = std::less<K>>
class LinkedSplayTree {
public:
    LinkedNode<K, V>* root {nullptr};
    Comparator compare;

    LinkedSplayTree() : root(nullptr), compare(Comparator()) {}

    explicit LinkedSplayTree(Comparator comparator) : root(nullptr), compare(comparator) {}

    LinkedNode<K, V>* add(K key, V value, bool doSplay = true);

    LinkedNode<K, V>* get(K key);

    LinkedNode<K, V>* search(K key) const;

    void remove(K key);

    void removeNode(LinkedNode<K, V>* node, bool doSplay = true);

    void splay(LinkedNode<K, V>* node, LinkedNode<K, V>* rootParent = nullptr);

    void rotateLeft(LinkedNode<K, V>* node, LinkedNode<K, V>* rootParent = nullptr);

    void rotateRight(LinkedNode<K, V>* node, LinkedNode<K, V>* rootParent = nullptr);

    [[nodiscard]] std::string toString() const;

    static void toStringRecursive(LinkedNode<K, V>* node, int depth, std::stringstream &oss);

    void replace(LinkedNode<K, V>* x, LinkedNode<K, V>* y);

    LinkedNode<K, V>* join(LinkedNode<K, V>* left, LinkedNode<K, V>* right);
};


template<typename K, typename V, typename Comparator>
std::string LinkedSplayTree<K, V, Comparator>::toString() const {
    std::stringstream oss;
    toStringRecursive(root, 0, oss);
    return oss.str();
}

template<typename K, typename V, typename Comparator>
void LinkedSplayTree<K, V, Comparator>::toStringRecursive(LinkedNode<K, V>* node, int depth, std::stringstream &oss) {
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
void LinkedSplayTree<K, V, Comparator>::rotateLeft(LinkedNode<K, V>* node, LinkedNode<K, V>* rootParent) {
    LinkedNode<K, V>* y = node->right;
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
void LinkedSplayTree<K, V, Comparator>::rotateRight(LinkedNode<K, V>* node, LinkedNode<K, V>* rootParent) {
    LinkedNode<K, V>* leftChild = node->left;

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
void LinkedSplayTree<K, V, Comparator>::splay(LinkedNode<K, V>* node, LinkedNode<K, V>* rootParent) {
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
LinkedNode<K, V>* LinkedSplayTree<K, V, Comparator>::add(K key, V value, bool doSplay) {
    auto* newNode = new LinkedNode<K, V>(key, value);
    LinkedNode<K, V>* y = nullptr;
    LinkedNode<K, V>* x = this->root;

    while (x) {
        y = x;
        if (compare(key, x->key)) {
            x = x->left;
        } else {
            x = x->right;
        }
    }

    newNode->parent = y;
    if (!y) {
        root = newNode;
    } else if (compare(newNode->key, y->key)) {
        y->left = newNode;  // Tree handling
        newNode->wedgeBefore(y);  // Linked list handling
    } else {
        y->right = newNode;  // Tree handling
        newNode->wedgeAfter(y);  // Linked list handling
    }

    if (doSplay) splay(newNode);
    return newNode;
}


template<typename K, typename V, typename Comparator>
LinkedNode<K, V>* LinkedSplayTree<K, V, Comparator>::get(K key) {
    LinkedNode<K, V>* node = this->search(key);
    if (node) splay(node);
    return node;
}


template<typename K, typename V, typename Comparator>
LinkedNode<K, V>* LinkedSplayTree<K, V, Comparator>::search(K key) const {
    LinkedNode<K, V>* node = root;
    while (node) {
        if (node->key == key) return node;
        node = compare(node->key, key) ? node->right : node->left;
    }
    return nullptr;
}


template<typename K, typename V, typename Comparator>
void LinkedSplayTree<K, V, Comparator>::replace(LinkedNode<K, V>* x, LinkedNode<K, V>* y) {
    if (x == root) root = y;
    else if (x == x->parent->left) x->parent->setLeftChild(y);
    else x->parent->setRightChild(y);
}


template<typename K, typename V, typename Comparator>
LinkedNode<K, V>* LinkedSplayTree<K, V, Comparator>::join(LinkedNode<K, V>* left, LinkedNode<K, V>* right) {
    LinkedNode<K, V>* leftParent = left->parent;
    LinkedNode<K, V>* newRoot = left->rightmost();
    splay(newRoot, leftParent);
    newRoot->setRightChild(right);
    return newRoot;
}


template<typename K, typename V, typename Comparator>
void LinkedSplayTree<K, V, Comparator>::remove(K key) {
    LinkedNode<K, V>* node = search(key);
    removeNode(node);
}


template<typename K, typename V, typename Comparator>
void LinkedSplayTree<K, V, Comparator>::removeNode(LinkedNode<K, V>* node, bool doSplay) {
    if (node == nullptr) return;

    node->dissolveLinks();

    LinkedNode<K, V>* newRoot;
    if (!node->left) newRoot = node->right;
    else if (!node->right) newRoot = node->left;
    else newRoot = join(node->left, node->right);

    replace(node, newRoot);
    if (doSplay) splay(node->parent);

    node->left = nullptr;
    node->right = nullptr;
}


#endif
