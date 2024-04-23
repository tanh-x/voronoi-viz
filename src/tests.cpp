#include <iostream>
#include "tests.hpp"


void testSplayTree() {
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
    a->setRightChild(new Node(37, "_F"));

    std::cout << tree.toString() << std::endl;

    auto n = tree.find(36);
    printf("Found: %d->%s\n\n", n->key, n->value);

    std::cout << tree.toString() << std::endl;
}