#include <iostream>
#include <fstream>

#include <acevm/vm.h>
#include <acevm/bytecode_stream.h>
#include <acevm/instructions.h>

#include <acevm/ace_string.h>

void Test()
{
    const char filename[] = "bytecode.bin";
#if 0
    char bytecode[] = {
        PUSH_ADDRESS,0x0B, 0x00, 0x00, 0x00, /*jumps to PUSH_I32 part*/
        MOV,         0x00, 0x00,       0x03, /*copy address to register 3*/
        JMP,         0x03,
        PUSH_I32,    0x14, 0x00, 0x00, 0x00,
        PUSH_FLOAT,  0x00, 0x00, 0x20, 0x40,
        MOV,         0x01, 0x00,       0x00, /*copy i32 to register 0*/
        MOV,         0x00, 0x00,       0x01, /*copy float to register 1*/
        ADD,         0x00/*src*/,0x01/*dest*/,
        PUSH_REG,    0x01,
        POP,
        POP,
        POP
    };
#endif

    char bytecode[] = {
        
    };

    std::ofstream out(filename, std::ios::out | std::ios::binary);
    out.write(bytecode, sizeof(bytecode));
    out.close();

    // load bytecode from file
    std::ifstream file(filename, std::ios::in | std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cout << "Could not open file " << filename << "\n";
        return;
    }

    size_t bytecode_size = file.tellg();
    file.seekg(0, std::ios::beg);

    char *bytecodes = new char[bytecode_size];
    file.read(bytecodes, bytecode_size);
    file.close();

    BytecodeStream bytecode_stream(bytecodes, bytecode_size);

    VM vm(&bytecode_stream);
    vm.Execute();

    std::cout << "top value (int32): " << vm.GetStack().Top().m_value.i32 << "\n";

    delete[] bytecodes;

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

    std::cout << "Press enter to exit...";
    std::cin.get();
    return 0;
}
