#include <iostream>
#include <cassert>
#include "utils/PriorityQueue.hpp"
#include "geometry/Vertex.hpp"

void priorityQueueTest1() {
    std::cout << "Testing PriorityQueue, case 1" << std::endl;
    PriorityQueue<int> pq;
    pq.add(5);
    assert(pq.peek() == 5);
    assert(pq.poll() == 5);
    assert(pq.empty());
}

void priorityQueueTest2() {
    std::cout << "Testing PriorityQueue, case 2" << std::endl;
    PriorityQueue<int> pq;
    pq.add(10);
    pq.add(5);
    pq.add(20);
    assert(pq.peek() == 5);
    assert(pq.poll() == 5);
    assert(pq.peek() == 10);
    assert(pq.poll() == 10);
    assert(pq.peek() == 20);
    assert(pq.poll() == 20);
    assert(pq.empty());
}

void priorityQueueTest3() {
    std::cout << "Testing PriorityQueue, case 3" << std::endl;
    PriorityQueue<int> pq;
    assert(pq.empty() == true);
    try {
        pq.peek();
        assert(false);
    } catch (std::out_of_range &e) {
        assert(true);
    } catch (...) {
        assert(false);
    }

    try {
        pq.poll();
        assert(false);
    } catch (std::out_of_range &e) {
        assert(true);
    } catch (...) {
        assert(false);
    }
}


void priorityQueueTest4() {
    std::cout << "Testing PriorityQueue, case 4" << std::endl;
    PriorityQueue<int> pq;
    pq.add(5);
    pq.add(5);
    pq.add(5);
    assert(pq.poll() == 5);
    assert(pq.poll() == 5);
    assert(pq.poll() == 5);
    assert(pq.empty());
}

void priorityQueueTest5() {
    std::cout << "Testing PriorityQueue, case 5" << std::endl;
    PriorityQueue<std::string> pq;
    pq.add("foo");
    pq.add("bar");
    pq.add("baz");
    assert(pq.poll() == "bar");
    assert(pq.poll() == "baz");
    assert(pq.poll() == "foo");
    assert(pq.empty());
}

void priorityQueueTest6() {
    std::cout << "Testing PriorityQueue, case 6" << std::endl;
    auto pq = PriorityQueue<int, std::greater<>>(std::greater<>());
    pq.add(10);
    pq.add(5);
    pq.add(20);
    assert(pq.poll() == 20);
    assert(pq.poll() == 10);
    assert(pq.poll() == 5);
    assert(pq.empty());
}

void priorityQueueTest7() {
    std::cout << "Testing PriorityQueue, case 7" << std::endl;

    PriorityQueue<int> pq;
    pq.add(66);
    pq.add(91);
    pq.add(85);
    pq.add(7);
    pq.add(10);
    pq.add(101);
    assert(pq.peek() == 7);
    assert(pq.poll() == 7);
    assert(pq.poll() == 10);
    pq.add(102);
    pq.add(67);
    pq.add(103);
    assert(pq.poll() == 66);
    assert(pq.poll() == 67);
    assert(!pq.empty());
}

struct VertexMaxHeapTestComparator {
    bool operator()(const Vertex &a, const Vertex &b) const {
        return a.pos.norm() > b.pos.norm();
    }
};

void priorityQueueTest8() {
    std::cout << "Testing PriorityQueue, case 8" << std::endl;
    VertexMaxHeapTestComparator comp;
    auto pq = PriorityQueue<Vertex, VertexMaxHeapTestComparator>(comp);
    pq.add(Vertex(1, Vec2(1, 1)));
    pq.add(Vertex(2, Vec2(2, 2)));
    pq.add(Vertex(3, Vec2(0, 3)));

    assert(pq.peek().label == 3);
    assert(pq.poll().pos.norm() == 3);
    assert(pq.peek().label == 2);
    assert(pq.poll().pos.x == 2);
    assert(pq.peek().label == 1);
    assert(pq.poll().pos.y == 1);
}
