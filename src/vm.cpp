#include <acevm/vm.h>
#include <acevm/instructions.h>

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
#if 0
    case PUSH_I32:
    {
        StackValue value;
        value.m_type = StackValue::INT32;

        // read 32-bit integer
        m_bs->Read(&value.m_value.i32, 4);

        // push to stack
        m_stack.Push(value);

        break;
    }
    case PUSH_I64:
    {
        StackValue value;
        value.m_type = StackValue::INT64;

        // read 64-bit integer
        m_bs->Read(&value.m_value.i64, 8);

        // push to stack
        m_stack.Push(value);

        break;
    }
    case PUSH_FLOAT:
    {
        StackValue value;
        value.m_type = StackValue::FLOAT;

        // read float
        m_bs->Read(&value.m_value.f, 4);

        // push to stack
        m_stack.Push(value);

        break;
    }
    case PUSH_DOUBLE:
    {
        StackValue value;
        value.m_type = StackValue::DOUBLE;

        // read double
        m_bs->Read(&value.m_value.d, 8);

        // push to stack
        m_stack.Push(value);

        break;
    }
    case PUSH_ADDRESS:
    {
        StackValue value;
        value.m_type = StackValue::ADDRESS;

        // read address
        m_bs->Read(&value.m_value.addr, 4);

        // push to stack
        m_stack.Push(value);

        break;
    }
    case PUSH_REG:
    {
        uint8_t reg;
        // read the register index
        m_bs->Read(&reg, 1);

        // push register value to stack
        m_stack.Push(m_registers[reg]);

        break;
    }
    case MOV:
    {
        uint16_t offset;
        m_bs->Read(&offset, 2);

        uint8_t reg;
        m_bs->Read(&reg, 1);

        // copy value at offset into register
        m_registers[reg] = m_stack[m_stack.GetStackPointer() - offset - 1];

        break;
    }
#endif
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
        uint8_t dest;
        m_bs->Read(&dest);

        uint8_t src;
        m_bs->Read(&src);

        // load values from registers
        StackValue &lhs = m_registers[dest];
        StackValue &rhs = m_registers[src];

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
            std::sprintf(buffer, "cannot add '%s' with '%s'",
                lhs.GetTypeString(), rhs.GetTypeString());

            ThrowException(Exception(buffer));
        }

        // set the desination register to be the result
        m_registers[dest] = result;

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
