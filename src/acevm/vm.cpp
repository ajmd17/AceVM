#include <acevm/vm.h>
#include <acevm/instructions.h>
#include <acevm/stack_value.h>
#include <acevm/heap_value.h>

#include <common/utf8.h>

#include <iostream>
#include <cstdio>
#include <cassert>

VM::VM(BytecodeStream *bs)
    : m_bs(bs)
{
    m_registers.m_flags = NONE;
}

VM::~VM()
{
}

void VM::InvokeFunction(StackValue &value, uint8_t num_args)
{
    const size_t num_registers = sizeof(m_registers.m_reg) / sizeof(m_registers.m_reg[0]);
    
    if (num_args > num_registers) {
        // more arguments than there are registers
        char buffer[256];
        std::sprintf(buffer, "maximum number of arguments exceeded");

        ThrowException(Exception(buffer));
    } else if (value.m_type != StackValue::FUNCTION) {
        char buffer[256];
        std::sprintf(buffer, "cannot invoke '%s' as a function",
            value.GetTypeString());

        ThrowException(Exception(buffer));
    } else if (value.m_value.func.num_args != num_args) {
        char buffer[256];
        std::sprintf(buffer, "expected %d parameters, received %d",
            (int)num_args, (int)value.m_value.func.num_args);

        ThrowException(Exception(buffer));
    } else {
        // store current address
        uint32_t previous = m_bs->Position();
        // seek to the function's address
        m_bs->Seek(value.m_value.func.address);

        while (m_bs->Position() < m_bs->Size()) {
            uint8_t code;
            m_bs->Read(&code, 1);

            if (code != RET) {
                HandleInstruction(code);
            } else {
                // leave function and return to previous position
                m_bs->Seek(previous);
                break;
            }
        }
    }
}

void VM::ThrowException(const Exception &exception)
{
    std::printf("runtime error: %s\n", exception.ToString().c_str());
    // seek to end of bytecode stream
    m_bs->Seek(m_bs->Size());
}

void VM::HandleInstruction(uint8_t code)
{
    switch (code) {
    case STORE_STATIC_STRING:
    {
        // get string length
        uint32_t len;
        m_bs->Read(&len);

        // read string based on length
        char *str = new char[len + 1];
        m_bs->Read(str, len);
        str[len] = 0;

        // the value will be freed on
        // the destructor call of m_static_memory
        HeapValue *hv = new HeapValue();
        hv->Assign(Utf8String(str));

        StackValue sv;
        sv.m_type = StackValue::HEAP_POINTER;
        sv.m_value.ptr = hv;

        m_static_memory.Store(sv);

        delete[] str;

        break;
    }
    case LOAD_I32:
    {
        uint8_t reg;
        m_bs->Read(&reg);

        // get register value given
        StackValue &value = m_registers[reg];
        value.m_type = StackValue::INT32;

        // read 32-bit integer into register value
        m_bs->Read(&value.m_value.i32);

        break;
    }
    case LOAD_I64:
    {
        uint8_t reg;
        m_bs->Read(&reg);

        // get register value given
        StackValue &value = m_registers[reg];
        value.m_type = StackValue::INT64;

        // read 64-bit integer into register value
        m_bs->Read(&value.m_value.i64);

        break;
    }
    case LOAD_LOCAL:
    {
        uint8_t reg;
        m_bs->Read(&reg);

        uint16_t offset;
        m_bs->Read(&offset);

        // read value from stack at (sp - offset)
        // into the the register
        m_registers[reg] = m_stack[m_stack.GetStackPointer() - offset];

        break;
    }
    case LOAD_STATIC:
    {
        uint8_t reg;
        m_bs->Read(&reg);

        uint32_t index;
        m_bs->Read(&index);

        // read value from static memory 
        // at the index into the the register
        m_registers[reg] = m_static_memory[index];

        break;
    }
    case MOV:
    {
        uint16_t offset;
        m_bs->Read(&offset);

        uint8_t reg;
        m_bs->Read(&reg);

        // copy value from register to stack value at (sp - offset)
        m_stack[m_stack.GetStackPointer() - offset] = m_registers[reg];

        break;
    }
    case PUSH:
    {
        uint8_t reg;
        m_bs->Read(&reg);

        // push a copy of the register value to the top of the stack
        m_stack.Push(m_registers[reg]);

        break;
    }
    case POP:
    {
        m_stack.Pop();

        break;
    }
    case JMP:
    {
        uint8_t reg;
        m_bs->Read(&reg);

        const StackValue &addr = m_registers[reg];
        assert(addr.m_type == StackValue::ADDRESS && "register must hold an address");

        m_bs->Seek(addr.m_value.addr);

        break;
    }
    case JE:
    {
        uint8_t reg;
        m_bs->Read(&reg);

        if (m_registers.m_flags == EQUAL) {
            const StackValue &addr = m_registers[reg];
            assert(addr.m_type == StackValue::ADDRESS && "register must hold an address");

            m_bs->Seek(addr.m_value.addr);
        }

        break;
    }
    case JNE:
    {
        uint8_t reg;
        m_bs->Read(&reg);

        if (m_registers.m_flags != EQUAL) {
            const StackValue &addr = m_registers[reg];
            assert(addr.m_type == StackValue::ADDRESS && "register must hold an address");

            m_bs->Seek(addr.m_value.addr);
        }

        break;
    }
    case JG:
    {
        uint8_t reg;
        m_bs->Read(&reg);

        if (m_registers.m_flags == GREATER) {
            const StackValue &addr = m_registers[reg];
            assert(addr.m_type == StackValue::ADDRESS && "register must hold an address");

            m_bs->Seek(addr.m_value.addr);
        }

        break;
    }
    case JGE:
    {
        uint8_t reg;
        m_bs->Read(&reg);

        if (m_registers.m_flags == GREATER || m_registers.m_flags == EQUAL) {
            const StackValue &addr = m_registers[reg];
            assert(addr.m_type == StackValue::ADDRESS && "register must hold an address");

            m_bs->Seek(addr.m_value.addr);
        }

        break;
    }
    case CALL:
    {
        uint8_t reg;
        m_bs->Read(&reg);

        uint8_t num_args;
        m_bs->Read(&num_args);

        InvokeFunction(m_registers[reg], num_args);

        break;
    }
    case CMP:
    {
        uint8_t lhs_reg;
        m_bs->Read(&lhs_reg);

        uint8_t rhs_reg;
        m_bs->Read(&rhs_reg);

        // load values from registers
        StackValue &lhs = m_registers[lhs_reg];
        StackValue &rhs = m_registers[rhs_reg];

        if (IS_VALUE_INTEGRAL(lhs) && IS_VALUE_INTEGRAL(rhs)) {
            int64_t left = GetValueInt64(lhs);
            int64_t right = GetValueInt64(rhs);

            if (left > right) {
                // set GREATER flag
                m_registers.m_flags = GREATER;
            } else if (left == right) {
                // set EQUAL flag
                m_registers.m_flags = EQUAL;
            } else {
                // set NONE flag
                m_registers.m_flags = NONE;
            }
        } else if (IS_VALUE_FLOATING_POINT(lhs) || IS_VALUE_FLOATING_POINT(rhs)) {
            double left = GetValueDouble(lhs);
            double right = GetValueDouble(rhs);

            if (left > right) {
                // set GREATER flag
                m_registers.m_flags = GREATER;
            } else if (left == right) {
                // set EQUAL flag
                m_registers.m_flags = EQUAL;
            } else {
                // set NONE flag
                m_registers.m_flags = NONE;
            }
        } else if (lhs.m_type == StackValue::HEAP_POINTER && 
            rhs.m_type == StackValue::HEAP_POINTER) {

            // compare memory addresses
            if (lhs.m_value.ptr > rhs.m_value.ptr) {
                // set GREATER flag
                m_registers.m_flags = GREATER;
            } else if (lhs.m_value.ptr == rhs.m_value.ptr) {
                // set EQUAL flag
                m_registers.m_flags = EQUAL;
            } else {
                // set NONE flag
                m_registers.m_flags = NONE;
            }
        } else if (lhs.m_type == StackValue::FUNCTION && 
            rhs.m_type == StackValue::FUNCTION) {

            // compare function address
            if (lhs.m_value.func.address > rhs.m_value.func.address) {
                // set GREATER flag
                m_registers.m_flags = GREATER;
            } else if (lhs.m_value.func.address == rhs.m_value.func.address) {
                // set EQUAL flag
                m_registers.m_flags = EQUAL;
            } else {
                // set NONE flag
                m_registers.m_flags = NONE;
            }
        } else {
            char buffer[256];
            std::sprintf(buffer, "cannot compare '%s' with '%s'",
                lhs.GetTypeString(), rhs.GetTypeString());

            ThrowException(Exception(buffer));
        }

        break;
    }
    case ADD:
    {
        uint8_t lhs_reg;
        m_bs->Read(&lhs_reg);

        uint8_t rhs_reg;
        m_bs->Read(&rhs_reg);

        uint8_t dst_reg;
        m_bs->Read(&dst_reg);

        // load values from registers
        StackValue &lhs = m_registers[lhs_reg];
        StackValue &rhs = m_registers[rhs_reg];

        StackValue result;
        result.m_type = MATCH_TYPES(lhs, rhs);

        if (IS_VALUE_INTEGRAL(lhs) && IS_VALUE_INTEGRAL(rhs)) {
            int64_t left = GetValueInt64(lhs);
            int64_t right = GetValueInt64(rhs);
            int64_t result_value = left + right;

            if (result.m_type == StackValue::INT32) {
                result.m_value.i32 = (int32_t)result_value;
            } else {
                result.m_value.i64 = result_value;
            }
        } else if (IS_VALUE_FLOATING_POINT(lhs) || IS_VALUE_FLOATING_POINT(rhs)) {
            double left = GetValueDouble(lhs);
            double right = GetValueDouble(rhs);
            double result_value = left + right;

            if (result.m_type == StackValue::FLOAT) {
                result.m_value.f = (float)result_value;
            } else {
                result.m_value.d = result_value;
            }
        } else if (lhs.m_type == StackValue::HEAP_POINTER) {
            // TODO: Check for '__OPR_ADD__' function and call it
        } else {
            char buffer[256];
            std::sprintf(buffer, "cannot add types '%s' and '%s'",
                lhs.GetTypeString(), rhs.GetTypeString());

            ThrowException(Exception(buffer));
        }

        // set the desination register to be the result
        m_registers[dst_reg] = result;

        break;
    }
    case SUB:
    {
        uint8_t lhs_reg;
        m_bs->Read(&lhs_reg);

        uint8_t rhs_reg;
        m_bs->Read(&rhs_reg);

        uint8_t dst_reg;
        m_bs->Read(&dst_reg);

        // load values from registers
        StackValue &lhs = m_registers[lhs_reg];
        StackValue &rhs = m_registers[rhs_reg];

        StackValue result;
        result.m_type = MATCH_TYPES(lhs, rhs);

        if (IS_VALUE_INTEGRAL(lhs) && IS_VALUE_INTEGRAL(rhs)) {
            int64_t left = GetValueInt64(lhs);
            int64_t right = GetValueInt64(rhs);
            int64_t result_value = left - right;

            if (result.m_type == StackValue::INT32) {
                result.m_value.i32 = (int32_t)result_value;
            } else {
                result.m_value.i64 = result_value;
            }
        } else if (IS_VALUE_FLOATING_POINT(lhs) || IS_VALUE_FLOATING_POINT(rhs)) {
            double left = GetValueDouble(lhs);
            double right = GetValueDouble(rhs);
            double result_value = left - right;

            if (result.m_type == StackValue::FLOAT) {
                result.m_value.f = (float)result_value;
            } else {
                result.m_value.d = result_value;
            }
        } else if (lhs.m_type == StackValue::HEAP_POINTER) {
            // TODO: Check for '__OPR_SUB__' function and call it
        } else {
            char buffer[256];
            std::sprintf(buffer, "cannot subtract types '%s' and '%s'",
                lhs.GetTypeString(), rhs.GetTypeString());

            ThrowException(Exception(buffer));
        }

        // set the desination register to be the result
        m_registers[dst_reg] = result;

        break;
    }
    case MUL:
    {
        uint8_t lhs_reg;
        m_bs->Read(&lhs_reg);

        uint8_t rhs_reg;
        m_bs->Read(&rhs_reg);

        uint8_t dst_reg;
        m_bs->Read(&dst_reg);

        // load values from registers
        StackValue &lhs = m_registers[lhs_reg];
        StackValue &rhs = m_registers[rhs_reg];

        StackValue result;
        result.m_type = MATCH_TYPES(lhs, rhs);

        if (IS_VALUE_INTEGRAL(lhs) && IS_VALUE_INTEGRAL(rhs)) {
            int64_t left = GetValueInt64(lhs);
            int64_t right = GetValueInt64(rhs);
            int64_t result_value = left * right;

            if (result.m_type == StackValue::INT32) {
                result.m_value.i32 = (int32_t)result_value;
            } else {
                result.m_value.i64 = result_value;
            }
        } else if (IS_VALUE_FLOATING_POINT(lhs) || IS_VALUE_FLOATING_POINT(rhs)) {
            double left = GetValueDouble(lhs);
            double right = GetValueDouble(rhs);
            double result_value = left * right;

            if (result.m_type == StackValue::FLOAT) {
                result.m_value.f = (float)result_value;
            } else {
                result.m_value.d = result_value;
            }
        } else if (lhs.m_type == StackValue::HEAP_POINTER) {
            // TODO: Check for '__OPR_MUL__' function and call it
        } else {
            char buffer[256];
            std::sprintf(buffer, "cannot multiply types '%s' and '%s'",
                lhs.GetTypeString(), rhs.GetTypeString());

            ThrowException(Exception(buffer));
        }

        // set the desination register to be the result
        m_registers[dst_reg] = result;

        break;
    }
    default:
        std::printf("unknown instruction '%d' referenced at location: 0x%08x\n",
            (int)code, (int)m_bs->Position());

        // seek to end of bytecode stream
        m_bs->Seek(m_bs->Size());
    }
}

void VM::Execute()
{
    while (m_bs->Position() < m_bs->Size()) {
        uint8_t code;
        m_bs->Read(&code, 1);

        HandleInstruction(code);
    }
}
