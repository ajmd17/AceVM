#include <acevm/stack_value.h>

StackValue::StackValue()
    : m_type(HEAP_POINTER)
{
    // initialize to null reference
    m_value.ptr = nullptr;
}

StackValue::StackValue(const StackValue &other)
    : m_type(other.m_type),
      m_value(other.m_value)
{
}
