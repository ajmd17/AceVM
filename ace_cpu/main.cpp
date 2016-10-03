#include <iostream>

#include "vm.h"
#include "bytecode_stream.h"
#include "instructions.h"

void Test()
{
    char bytecode[] = {
        PUSH_I32,    0x14, 0x00, 0x00, 0x00,
        MOV,         0x00, 0x00,       0x00,
        PUSH_FLOAT,  0x00, 0x00, 0x20, 0x40,
        MOV,         0x01, 0x00,       0x01,
        ADD
    };

    BytecodeStream bytecode_stream(bytecode, sizeof(bytecode));

    VM vm(&bytecode_stream);
    vm.Execute();

    std::cout << "top value (float): " << vm.GetStack().Top().m_value.f << "\n";

    /*HeapValue *v1 = vm.GetHeap().Alloc();

    StackValue sv1;
    sv1.m_type = StackValue::FLOAT;
    sv1.m_value.f = 3.5f;

    StackValue sv2;
    sv2.m_type = StackValue::INT64;
    sv2.m_value.i64 = 49942;

    vm.GetStack().Push(sv1);
    vm.GetStack().Push(sv2);

    

    std::cout << "Heap Dump: \n\n" << vm.GetHeap() << "\n\n";
    std::cout << "Sweep()\n\n";

    vm.GetHeap().Sweep();

    std::cout << "Heap Dump: \n\n" << vm.GetHeap() << "\n\n";*/
}

int main()
{
    Test();

    system("pause");
    return 0;
}