#ifndef voronoi_viz_heap_hpp
#define voronoi_viz_heap_hpp

#include <algorithm>
#include <vector>
#include <stdexcept>
#include <vector>
#include <stdexcept>
#include <functional>

template<typename E, typename Comparator = std::less<E>>
class PriorityQueue {
private:
    std::vector<E> heap;
    Comparator compare;

    void heapifyup(int index);

    void heapifydown(int index);

    static int parent(int index);

    static int left(int index);

    static int right(int index);

public:
    PriorityQueue() = default;

    void add(E element);

    E peek();

    E poll();

    [[nodiscard]] bool empty() const;
};


template<typename E, typename Comparator>
void PriorityQueue<E, Comparator>::heapifyup(int index) {
    while (index != 0 && compare(heap[parent(index)], heap[index])) {
        std::swap(heap[parent(index)], heap[index]);
        index = parent(index);
    }
}


template<typename E, typename Comparator>
void PriorityQueue<E, Comparator>::heapifydown(int index) {
    int argmin = index;
    int leftChild = left(index);
    int rightChild = right(index);

    if (leftChild < heap.size() && compare(heap[leftChild], heap[argmin])) argmin = leftChild;
    if (rightChild < heap.size() && compare(heap[rightChild], heap[argmin])) argmin = rightChild;

    if (argmin == index) return;

    std::swap(heap[index], heap[argmin]);
    heapifydown(argmin);
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
    heapifyup(index);
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
    if (!heap.empty()) heapifydown(0);
    return min;
}


template<typename E, typename Comparator>
bool PriorityQueue<E, Comparator>::empty() const {
    return heap.empty();
}


#endif
