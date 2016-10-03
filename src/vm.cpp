#include <acevm/vm.h>
#include <acevm/instructions.h>

#include <cstdio>
#include <cassert>

VM::VM(BytecodeStream *bs)
    : m_bs(bs)
{
    size_t num_registers = sizeof(m_registers.reg) / sizeof(m_registers.reg[0]);
    
    // initalize all registers to nullptr
    for (size_t i = 0; i < num_registers; i++) {
        m_registers.reg[i] = nullptr;
    }
}

VM::~VM()
{
}

void VM::InvokeFunction(StackValue &value, uint8_t num_args)
{
    size_t num_registers = sizeof(m_registers.reg) / sizeof(m_registers.reg[0]);
    
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
        // push values to stack from registers
        for (int i = num_args - 1; i >= 0; i--) {
            m_stack.Push(*m_registers.reg[i]);
        }

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
    std::printf("Runtime error: %s\n", exception.ToString().c_str());
    // seek to end of bytecode stream
    m_bs->Seek(m_bs->Size());
}

void VM::HandleInstruction(uint8_t code)
{
    switch (code) {
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
        m_stack.Push(*m_registers.reg[reg]);

        break;
    }
    case POP:
    {
        m_stack.Pop();

        break;
    }
    case MOV:
    {
        uint16_t index;
        m_bs->Read(&index, 2);

        uint8_t reg;
        m_bs->Read(&reg, 1);

        m_registers.reg[reg] = &m_stack[index];

        break;
    }
    case JMP:
    {
        uint8_t address_index;
        m_bs->Read(&address_index, 1);

        StackValue *addr = m_registers.reg[address_index];
        assert(addr != nullptr && "register value must not be null");
        assert(addr->m_type == StackValue::ADDRESS && "register must hold an address");

        m_bs->Seek(addr->m_value.addr);

        break;
    }
    case JT:
    {
        uint8_t condition_index;
        m_bs->Read(&condition_index, 1);

        uint8_t address_index;
        m_bs->Read(&address_index, 1);

        StackValue *cond = m_registers.reg[condition_index];
        assert(cond != nullptr && "register value must not be null");

        StackValue *addr = m_registers.reg[address_index];
        assert(addr != nullptr && "register value must not be null");
        assert(addr->m_type == StackValue::ADDRESS && "register must hold an address");

        if (GetValueInt64(*cond)) {
            m_bs->Seek(addr->m_value.addr);
        }

        break;
    }
    case JF:
    {
        uint8_t condition_index;
        m_bs->Read(&condition_index, 1);

        uint8_t address_index;
        m_bs->Read(&address_index, 1);

        StackValue *cond = m_registers.reg[condition_index];
        assert(cond != nullptr && "register value must not be null");

        StackValue *addr = m_registers.reg[address_index];
        assert(addr != nullptr && "register value must not be null");
        assert(addr->m_type == StackValue::ADDRESS && "register must hold an address");

        if (!GetValueInt64(*cond)) {
            m_bs->Seek(addr->m_value.addr);
        }

        break;
    }
    case CALL:
    {
        uint8_t reg;
        m_bs->Read(&reg, 1);

        uint8_t num_args;
        m_bs->Read(&num_args, 1);

        StackValue *func = m_registers.reg[reg];
        assert(func != nullptr && "register value must not be null");

        InvokeFunction(*func, num_args);

        break;
    }
    case ADD:
    {
        // load values from registers
        StackValue *lhs = m_registers.r0;
        StackValue *rhs = m_registers.r1;

        assert(lhs != nullptr && "left-hand side of equation cannot be null");
        assert(rhs != nullptr && "right-hand side of equation cannot be null");

        const StackValue &left_ref = *lhs;
        const StackValue &right_ref = *rhs;

        StackValue result;
        result.m_type = MATCH_TYPES(left_ref, right_ref);

        if (IS_VALUE_INTEGRAL(left_ref) && IS_VALUE_INTEGRAL(right_ref)) {
            int64_t left = GetValueInt64(left_ref);
            int64_t right = GetValueInt64(right_ref);
            int64_t result_value = left + right;

            if (result.m_type == StackValue::INT32) {
                result.m_value.i32 = (int32_t)result_value;
            } else {
                result.m_value.i64 = result_value;
            }

        } else if (IS_VALUE_FLOATING_POINT(left_ref) || IS_VALUE_FLOATING_POINT(right_ref)) {
            double left = GetValueDouble(left_ref);
            double right = GetValueDouble(right_ref);
            double result_value = left + right;

            if (result.m_type == StackValue::FLOAT) {
                result.m_value.f = (float)result_value;
            } else {
                result.m_value.d = result_value;
            }
        } else {
            char buffer[256];
            std::sprintf(buffer, "cannot add '%s' with '%s'",
                left_ref.GetTypeString(), right_ref.GetTypeString());

            ThrowException(Exception(buffer));
        }

        // push the resulting value onto the stack
        m_stack.Push(result);

        break;
    }
    default:
        std::printf("Unknown instruction %d referenced at location: 0x%08x\n",
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
