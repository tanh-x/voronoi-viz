#ifndef VORONOI_VIZ_HEAP_HPP
#define VORONOI_VIZ_HEAP_HPP

#include <algorithm>
#include <vector>
#include <stdexcept>
#include <vector>
#include <stdexcept>
#include <functional>

void priorityQueueTest1();

void priorityQueueTest2();

void priorityQueueTest3();

void priorityQueueTest4();

void priorityQueueTest5();

void priorityQueueTest6();

void priorityQueueTest7();

void priorityQueueTest8();

template<typename E, typename Comparator = std::less<E>>
class PriorityQueue {
private:
    Comparator compare;

    void heapifyUp(int index);

    void heapifyDown(int index);

    static int parent(int index);

    static int left(int index);

    static int right(int index);

public:
    PriorityQueue() : compare(Comparator()) {};

    explicit PriorityQueue(Comparator comparator) : compare(comparator) {};

    void add(E element);

    E peek();

    E poll();

    [[nodiscard]] bool empty() const;

    std::vector<E> heap;
};


template<typename E, typename Comparator>
void PriorityQueue<E, Comparator>::heapifyUp(int index) {
    while (index != 0 && compare(heap[index], heap[parent(index)])) {
        std::swap(heap[parent(index)], heap[index]);
        index = parent(index);
    }
}


template<typename E, typename Comparator>
void PriorityQueue<E, Comparator>::heapifyDown(int index) {
    int argmin = index;
    int leftChild = left(index);
    int rightChild = right(index);

    if (leftChild < static_cast<int>(heap.size())
        && compare(heap[leftChild], heap[argmin]))
        argmin = leftChild;
    if (rightChild < static_cast<int>(heap.size())
        && compare(heap[rightChild], heap[argmin]))
        argmin = rightChild;

    if (argmin == index) return;

    std::swap(heap[index], heap[argmin]);
    heapifyDown(argmin);
}


template<typename E, typename Comparator>
int PriorityQueue<E, Comparator>::parent(int index) {
    return (index - 1) / 2;
}


template<typename E, typename Comparator>
int PriorityQueue<E, Comparator>::left(int index) {
    return 2 * index + 1;
}


template<typename E, typename Comparator>
int PriorityQueue<E, Comparator>::right(int index) {
    return 2 * index + 2;
}


template<typename E, typename Comparator>
void PriorityQueue<E, Comparator>::add(E element) {
    heap.push_back(element);
    int index = static_cast<int>(heap.size()) - 1;
    heapifyUp(index);
}


template<typename E, typename Comparator>
E PriorityQueue<E, Comparator>::peek() {
    if (heap.empty()) throw std::out_of_range("PriorityQueue is empty");
    return heap[0];
}


template<typename E, typename Comparator>
E PriorityQueue<E, Comparator>::poll() {
    if (heap.empty()) throw std::out_of_range("PriorityQueue is empty");

    E min = heap[0];
    heap[0] = heap.back();
    heap.pop_back();
    if (!heap.empty()) heapifyDown(0);
    return min;
}


template<typename E, typename Comparator>
bool PriorityQueue<E, Comparator>::empty() const {
    return heap.empty();
}

#endif
