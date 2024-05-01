#include <cassert>
#include "utils/LinkedSplayTree.hpp"
#include "geometry/Vertex.hpp"
#include "utils/SplayTree.hpp"

void linkedSplayTreeTest1() {
    std::cout << "Testing LinkedSplayTree, case 1" << std::endl;
    LinkedSplayTree<int, const char*> tree;

    std::vector<int> keys = {136, 134, 250, 130, 170, 180, 190, 160, 120, 150};
    std::vector<const char*> values = {"z", "a", "M", "c", "d", "e", "f", "g", "h", "i"};

    for (int i = 0; i < static_cast<int>(keys.size()); i++) {
        tree.add(keys[i], values[i]);
    }

//    std::cout << tree.toString() << std::endl;
    assert(tree.toString() ==
           "150->i\n"
           "|\t120->h\n"
           "|\t|\t--\n"
           "|\t|\t136->z\n"
           "|\t|\t|\t130->c\n"
           "|\t|\t|\t|\t--\n"
           "|\t|\t|\t|\t134->a\n"
           "|\t|\t|\t--\n"
           "|\t160->g\n"
           "|\t|\t--\n"
           "|\t|\t190->f\n"
           "|\t|\t|\t170->d\n"
           "|\t|\t|\t|\t--\n"
           "|\t|\t|\t|\t180->e\n"
           "|\t|\t|\t250->M\n"
    );

    sort(keys.begin(), keys.end());

    int c = 0;
    LinkedNode<int, const char*>* previous = nullptr;
    LinkedNode<int, const char*>* node = tree.root->leftmost();
    while (node != nullptr) {
        int ans = keys[c];
        assert(node->key == ans);
        assert(node->prev == previous);
        if (c > 0) {
            assert(previous->next == node);
            assert(previous->key == keys[c - 1]);
        }

        previous = node;
        node = node->next;
        c++;
    }

    assert(previous->key == 250);
}


struct VertexTestComparator {
    bool operator()(Vertex* a, Vertex* b) const {
        return a->x() < b->x();
    }
};


void vertexTreeToStringRecursive(LinkedNode<Vertex*, const char*>* node, int depth, std::stringstream &oss) {
    for (int i = 0; i < depth; i++) oss << "|\t";
    if (node == nullptr) {
        oss << "--" << std::endl;
        return;
    }
    oss << node->key->toString() << "->" << node->value << std::endl;
    if (node->leftChild == nullptr && node->rightChild == nullptr) return;
    vertexTreeToStringRecursive(node->leftChild, depth + 1, oss);
    vertexTreeToStringRecursive(node->rightChild, depth + 1, oss);
}

std::string vertexTreeToString(LinkedSplayTree<Vertex*, const char*, VertexTestComparator> tree) {
    std::stringstream oss;
    vertexTreeToStringRecursive(tree.root, 0, oss);
    return oss.str();
}


void linkedSplayTreeTest2_3() {
    std::cout << "Testing LinkedSplayTree, case 2" << std::endl;
    VertexTestComparator comp;
    auto tree = LinkedSplayTree<Vertex*, const char*, VertexTestComparator>(comp);

    auto v30 = new Vertex(3, {30, 80});
    tree.add(new Vertex(0, {10, -4}), "x=10");
    tree.add(new Vertex(1, {15, 23}), "x=15");
    tree.add(new Vertex(2, {20, 12}), "x=20");
    tree.add(v30, "x=30");
    tree.add(new Vertex(4, {40, 17}), "x=40");
    tree.add(new Vertex(5, {50, -8}), "x=50");
    tree.add(new Vertex(6, {60, -2}), "x=60");
    tree.add(new Vertex(7, {70, -1}), "x=70");
    tree.add(new Vertex(9, {90, 47}), "x=90");
    tree.add(new Vertex(10, {100, 56}), "x=100");

    auto v80 = new Vertex(8, {80, 32});
    const char* k80 = "x=80";
    tree.add(v80, k80);
    assert(tree.root->key == v80);
    assert(tree.get(v80)->key->x() == 80);

    std::vector<int> expectedXs = {10, 15, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    int c = 0;
    LinkedNode<Vertex*, const char*>* previous = nullptr;
    LinkedNode<Vertex*, const char*>* node = tree.root->leftmost();
    while (node != nullptr) {
        int ans = expectedXs[c];
        assert(node->key->x() == ans);
        assert(node->prev == previous);
        if (c > 0) {
            assert(previous->next == node);
            assert(previous->key->x() == expectedXs[c - 1]);
        }

        previous = node;
        node = node->next;
        c++;
    }


    std::cout << "Testing LinkedSplayTree, case 3" << std::endl;

    tree.remove(v30);

    assert(tree.get(v30) == nullptr);

    expectedXs = {10, 15, 20, 40, 50, 60, 70, 80, 90, 100};
    c = 0;
    previous = nullptr;
    node = tree.root->leftmost();
    while (node != nullptr) {
        int ans = expectedXs[c];
        assert(node->key->x() == ans);
        assert(node->prev == previous);
        if (c > 0) {
            assert(previous->next == node);
            assert(previous->key->x() == expectedXs[c - 1]);
        }

        previous = node;
        node = node->next;
        c++;
    }

    assert(previous->key);

    std::cout << "Finished (Linked)SplayTree tests" << std::endl;
}
