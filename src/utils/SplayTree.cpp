#include <cassert>
#include "utils/SplayTree.hpp"
#include "geometry/Vertex.hpp"

void splayTreeTest1() {
    std::cout << "Testing SplayTree, case 1" << std::endl;
    SplayTree<int, const char*> tree;

    auto a = new Node(36, "a");
    auto b = new Node(34, "b");
    auto c = new Node(32, "c");
    auto d = new Node(30, "d");
    auto e = new Node(60, "e");
    auto f = new Node(20, "f");
    auto g = new Node(70, "g");
    auto h = new Node(80, "h");
    auto i = new Node(90, "i");

    tree.root = i;

    // Example 1, slides 2b
    i->setLeftChild(h);
    i->setRightChild(new Node(91, "_J"));

    h->setLeftChild(g);
    h->setRightChild(new Node(81, "_I"));

    g->setLeftChild(f);
    g->setRightChild(new Node(71, "_H"));

    f->setLeftChild(new Node(19, "_A"));
    f->setRightChild(e);

    e->setLeftChild(d);
    e->setRightChild(new Node(61, "_G"));

    d->setLeftChild(new Node(29, "_B"));
    d->setRightChild(c);

    c->setLeftChild(new Node(31, "_C"));
    c->setRightChild(b);

    b->setLeftChild(new Node(33, "_D"));
    b->setRightChild(a);

    a->setLeftChild(new Node(35, "_E"));

//    std::cout << tree.toString() << std::endl;
    assert(tree.toString() ==
           "90->i\n"
           "|\t80->h\n"
           "|\t|\t70->g\n"
           "|\t|\t|\t20->f\n"
           "|\t|\t|\t|\t19->_A\n"
           "|\t|\t|\t|\t60->e\n"
           "|\t|\t|\t|\t|\t30->d\n"
           "|\t|\t|\t|\t|\t|\t29->_B\n"
           "|\t|\t|\t|\t|\t|\t32->c\n"
           "|\t|\t|\t|\t|\t|\t|\t31->_C\n"
           "|\t|\t|\t|\t|\t|\t|\t34->b\n"
           "|\t|\t|\t|\t|\t|\t|\t|\t33->_D\n"
           "|\t|\t|\t|\t|\t|\t|\t|\t36->a\n"
           "|\t|\t|\t|\t|\t|\t|\t|\t|\t35->_E\n"
           "|\t|\t|\t|\t|\t|\t|\t|\t|\t--\n"
           "|\t|\t|\t|\t|\t61->_G\n"
           "|\t|\t|\t71->_H\n"
           "|\t|\t81->_I\n"
           "|\t91->_J\n"
    );

    Node<int, const char*>* n = tree.get(36);
    assert(n->key == 36);
    assert(std::string(n->value) == "a");

//    std::cout << tree.toString() << std::endl;
    assert(tree.toString() ==
           "36->a\n"
           "|\t20->f\n"
           "|\t|\t19->_A\n"
           "|\t|\t30->d\n"
           "|\t|\t|\t29->_B\n"
           "|\t|\t|\t34->b\n"
           "|\t|\t|\t|\t32->c\n"
           "|\t|\t|\t|\t|\t31->_C\n"
           "|\t|\t|\t|\t|\t33->_D\n"
           "|\t|\t|\t|\t35->_E\n"
           "|\t80->h\n"
           "|\t|\t70->g\n"
           "|\t|\t|\t60->e\n"
           "|\t|\t|\t|\t--\n"
           "|\t|\t|\t|\t61->_G\n"
           "|\t|\t|\t71->_H\n"
           "|\t|\t90->i\n"
           "|\t|\t|\t81->_I\n"
           "|\t|\t|\t91->_J\n"
    );
}


void splayTreeTest2() {
    std::cout << "Testing SplayTree, case 2" << std::endl;
    SplayTree<int, const char*> tree;

    auto a = new Node(10, "a");
    auto b = new Node(20, "b");
    auto c = new Node(30, "c");
    auto d = new Node(40, "d");
    auto e = new Node(50, "e");
    auto f = new Node(60, "f");
    auto g = new Node(70, "g");

    tree.root = g;

    g->setLeftChild(f);
    f->setLeftChild(e);
    f->setRightChild(new Node(61, "_G"));
    e->setLeftChild(d);
    e->setRightChild(new Node(51, "_F"));
    d->setLeftChild(c);
    c->setLeftChild(b);
    b->setLeftChild(a);
    a->setRightChild(new Node(11, "_B"));

//    std::cout << tree.toString() << std::endl;
    assert(tree.toString() ==
           "70->g\n"
           "|\t60->f\n"
           "|\t|\t50->e\n"
           "|\t|\t|\t40->d\n"
           "|\t|\t|\t|\t30->c\n"
           "|\t|\t|\t|\t|\t20->b\n"
           "|\t|\t|\t|\t|\t|\t10->a\n"
           "|\t|\t|\t|\t|\t|\t|\t--\n"
           "|\t|\t|\t|\t|\t|\t|\t11->_B\n"
           "|\t|\t|\t|\t|\t|\t--\n"
           "|\t|\t|\t|\t|\t--\n"
           "|\t|\t|\t|\t--\n"
           "|\t|\t|\t51->_F\n"
           "|\t|\t61->_G\n"
           "|\t--\n"
    );

    Node<int, const char*>* n = tree.get(10);
    assert(n->key == 10);
    assert(std::string(n->value) == "a");

//    std::cout << tree.toString() << std::endl;
    assert(tree.toString() ==
           "10->a\n"
           "|\t--\n"
           "|\t60->f\n"
           "|\t|\t40->d\n"
           "|\t|\t|\t20->b\n"
           "|\t|\t|\t|\t11->_B\n"
           "|\t|\t|\t|\t30->c\n"
           "|\t|\t|\t50->e\n"
           "|\t|\t|\t|\t--\n"
           "|\t|\t|\t|\t51->_F\n"
           "|\t|\t70->g\n"
           "|\t|\t|\t61->_G\n"
           "|\t|\t|\t--\n"
    );
}

struct VertexTestComparator {
    bool operator()(Vertex* a, Vertex* b) const {
        return a->x() < b->x();
    }
};


void vertexTreeToStringRecursive(Node<Vertex*, const char*>* node, int depth, std::stringstream &oss) {
    for (int i = 0; i < depth; i++) oss << "|\t";
    if (node == nullptr) {
        oss << "--" << std::endl;
        return;
    }
    oss << node->key->toString() << "->" << node->value << std::endl;
    if (node->left == nullptr && node->right == nullptr) return;
    vertexTreeToStringRecursive(node->left, depth + 1, oss);
    vertexTreeToStringRecursive(node->right, depth + 1, oss);
}

std::string vertexTreeToString(SplayTree<Vertex*, const char*, VertexTestComparator> tree) {
    std::stringstream oss;
    vertexTreeToStringRecursive(tree.root, 0, oss);
    return oss.str();
}

void splayTreeTest3_4() {
    std::cout << "Testing SplayTree, case 3" << std::endl;
    VertexTestComparator comp;
    auto tree = SplayTree<Vertex*, const char*, VertexTestComparator>(comp);

    auto v10 = new Node(new Vertex(0, {10, -4}), "x=10");
    auto v15 = new Node(new Vertex(1, {15, 23}), "x=15");
    auto v20 = new Node(new Vertex(2, {20, 12}), "x=20");
    auto v30 = new Node(new Vertex(3, {30, 80}), "x=30");
    auto v40 = new Node(new Vertex(4, {40, 17}), "x=40");
    auto v50 = new Node(new Vertex(5, {50, -8}), "x=50");
    auto v60 = new Node(new Vertex(6, {60, -2}), "x=60");
    auto v70 = new Node(new Vertex(7, {70, -1}), "x=70");
    auto v90 = new Node(new Vertex(9, {90, 47}), "x=90");
    auto v100 = new Node(new Vertex(10, {100, 56}), "x=100");

    tree.root = v50;

    v50->setLeftChild(v30);
    v50->setRightChild(v60);
    v30->setLeftChild(v10);
    v30->setRightChild(v40);
    v10->setRightChild(v20);
    v20->setLeftChild(v15);
    v60->setRightChild(v90);
    v90->setLeftChild(v70);
    v90->setRightChild(v100);

//    std::cout << vertexTreeToString(tree) << std::endl;
    assert(vertexTreeToString(tree) ==
           "v5->x=50\n"
           "|\tv3->x=30\n"
           "|\t|\tv0->x=10\n"
           "|\t|\t|\t--\n"
           "|\t|\t|\tv2->x=20\n"
           "|\t|\t|\t|\tv1->x=15\n"
           "|\t|\t|\t|\t--\n"
           "|\t|\tv4->x=40\n"
           "|\tv6->x=60\n"
           "|\t|\t--\n"
           "|\t|\tv9->x=90\n"
           "|\t|\t|\tv7->x=70\n"
           "|\t|\t|\tv10->x=100\n"
    );

    auto v80 = new Vertex(8, {80, 32});
    const char* k80 = "x=80";
    tree.add(v80, k80);
    assert(tree.root->key == v80);

//    std::cout << vertexTreeToString(tree) << std::endl;
    assert(vertexTreeToString(tree) ==
           "v8->x=80\n"
           "|\tv6->x=60\n"
           "|\t|\tv5->x=50\n"
           "|\t|\t|\tv3->x=30\n"
           "|\t|\t|\t|\tv0->x=10\n"
           "|\t|\t|\t|\t|\t--\n"
           "|\t|\t|\t|\t|\tv2->x=20\n"
           "|\t|\t|\t|\t|\t|\tv1->x=15\n"
           "|\t|\t|\t|\t|\t|\t--\n"
           "|\t|\t|\t|\tv4->x=40\n"
           "|\t|\t|\t--\n"
           "|\t|\tv7->x=70\n"
           "|\tv9->x=90\n"
           "|\t|\t--\n"
           "|\t|\tv10->x=100\n"
    );

    assert(tree.get(v80)->key->x() == 80);

    std::cout << "Testing SplayTree, case 4" << std::endl;

    tree.remove(v30->key);
    assert(tree.root->key == v50->key);

//    std::cout << vertexTreeToString(tree) << std::endl;
    assert(vertexTreeToString(tree) ==
           "v5->x=50\n"
           "|\tv2->x=20\n"
           "|\t|\tv0->x=10\n"
           "|\t|\t|\t--\n"
           "|\t|\t|\tv1->x=15\n"
           "|\t|\tv4->x=40\n"
           "|\tv6->x=60\n"
           "|\t|\t--\n"
           "|\t|\tv8->x=80\n"
           "|\t|\t|\tv7->x=70\n"
           "|\t|\t|\tv9->x=90\n"
           "|\t|\t|\t|\t--\n"
           "|\t|\t|\t|\tv10->x=100\n"
    );
}
