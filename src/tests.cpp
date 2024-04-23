#include <iostream>
#include "tests.hpp"
#include "utils/PriorityQueue.hpp"


void runAllTests() {
    std::cout << "-- Running tests --\n" << std::endl;

    // Splay tree slides, example 1
    splayTreeTest1();
    // Splay tree slides, example 2
    splayTreeTest2();
    // Splay tree slides, insertion and deletion example
    splayTreeTest3_4();

    priorityQueueTest1();
    priorityQueueTest2();
    priorityQueueTest3();
    priorityQueueTest4();
    priorityQueueTest5();
    priorityQueueTest6();
    priorityQueueTest7();
    priorityQueueTest8();

    std::cout << "\n-- All assertions passed --\n" << std::endl;
}