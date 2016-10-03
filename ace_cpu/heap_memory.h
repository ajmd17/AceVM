#ifndef HEAP_MEMORY_H
#define HEAP_MEMORY_H

#include "heap_value.h"

#include <ostream>

struct HeapNode {
    HeapValue value;
    HeapNode *before = nullptr;
    HeapNode *after = nullptr;
};

class Heap {
    friend std::ostream &operator<<(std::ostream &os, const Heap &heap);
public:
    Heap();
    Heap(const Heap &other) = delete;
    ~Heap();

    /** Allocate a new value on the heap. */
    HeapValue *Alloc();
    /** Delete all nodes that are not marked */
    void Sweep();

private:
    HeapNode *m_head;
    size_t m_num_objects;
};

#endif