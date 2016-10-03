#include "heap_value.h"

#include <iostream>

HeapValue::HeapValue()
    : m_holder(nullptr),
      m_ptr(nullptr),
      m_flags(0)
{
}

HeapValue::~HeapValue()
{
    if (m_holder != nullptr) {
        delete m_holder;
    }

    std::cout << "~HeapValue()\n";
}