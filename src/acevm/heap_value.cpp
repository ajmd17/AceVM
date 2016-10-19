#include <acevm/heap_value.hpp>

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
}
