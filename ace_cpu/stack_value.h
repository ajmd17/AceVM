#ifndef STACK_VALUE_H
#define STACK_VALUE_H

#include "heap_value.h"

#include <stdexcept>
#include <cstdint>

struct Function {
    uint32_t address;
    uint8_t num_args;
};

struct StackValue {
    enum {
        INT32,
        INT64,
        FLOAT,
        DOUBLE,
        HEAP_POINTER,
        FUNCTION,
        ADDRESS,
    } m_type;

    union {
        int32_t i32;
        int64_t i64;
        float f;
        double d;
        HeapValue *ptr;
        Function func;
        uint32_t addr;
    } m_value;

    StackValue();
    explicit StackValue(const StackValue &other);

    inline const char *GetTypeString() const
    {
        switch (m_type) {
        case INT32:
            return "Int32";
        case INT64:
            return "Int64";
        case FLOAT:
            return "Float";
        case DOUBLE:
            return "Double";
        case HEAP_POINTER:
            return "Reference";
        case FUNCTION:
            return "Function";
        default:
            return "Undefined";
        }
    }
};

#endif