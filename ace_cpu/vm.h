#ifndef VM_H
#define VM_H

#include "bytecode_stream.h"
#include "stack_memory.h"
#include "heap_memory.h"
#include "exception.h"

#include <cstdint>
#include <array>

#define IS_VALUE_INTEGRAL(stack_value) \
        ((stack_value).m_type == StackValue::INT32 || \
        (stack_value).m_type == StackValue::INT64)

#define IS_VALUE_FLOATING_POINT(stack_value) \
        ((stack_value).m_type == StackValue::FLOAT || \
        (stack_value).m_type == StackValue::DOUBLE)

#define MATCH_TYPES(lhs, rhs) \
        ((lhs).m_type < (rhs).m_type) ? (rhs).m_type : (lhs).m_type

struct Registers {
    union {
        struct {
            StackValue *r0;
            StackValue *r1;
            StackValue *r2;
            StackValue *r3;
            StackValue *r4;
            StackValue *r5;
            StackValue *r6;
            StackValue *r7;
        };

        StackValue *reg[8];
    };
};

class VM {
public:
    VM(BytecodeStream *bs);
    VM(const VM &other) = delete;
    ~VM();

    inline Stack &GetStack() { return m_stack; }
    inline Heap &GetHeap() { return m_heap; }
    inline Registers &GetRegisters() { return m_registers; }

    void InvokeFunction(StackValue &value, uint8_t num_args);
    void HandleInstruction(uint8_t code);
    void Execute();

private:
    Stack m_stack;
    Heap m_heap;
    Registers m_registers;

    BytecodeStream *m_bs;

    void ThrowException(const Exception &exception);

    inline int64_t GetValueInt64(const StackValue &stack_value)
    {
        switch (stack_value.m_type) {
        case StackValue::INT32:
            return (int64_t)stack_value.m_value.i32;
        case StackValue::INT64:
            return stack_value.m_value.i64;
        case StackValue::FLOAT:
            return (int64_t)stack_value.m_value.f;
        case StackValue::DOUBLE:
            return (int64_t)stack_value.m_value.d;
        default:
        {
            char buffer[256];
            std::sprintf(buffer, "no conversion from '%s' to 'Int64'", 
                stack_value.GetTypeString());
            ThrowException(Exception(buffer));

            return 0;
        }
        }
    }

    inline double GetValueDouble(const StackValue &stack_value)
    {
        switch (stack_value.m_type) {
        case StackValue::INT32:
            return (double)stack_value.m_value.i32;
        case StackValue::INT64:
            return (double)stack_value.m_value.i64;
        case StackValue::FLOAT:
            return (double)stack_value.m_value.f;
        case StackValue::DOUBLE:
            return stack_value.m_value.d;
        default:
        {
            char buffer[256];
            std::sprintf(buffer, "no conversion from '%s' to 'Double'",
                stack_value.GetTypeString());
            ThrowException(Exception(buffer));

            return std::numeric_limits<double>::quiet_NaN();
        }
        }
    }
};

#endif