#include <iostream>
#include <fstream>

#include <acevm/vm.h>
#include <acevm/bytecode_stream.h>
#include <acevm/instructions.h>

#include <acevm/ace_string.h>

#include <chrono>

void Test()
{
    std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
    start = std::chrono::high_resolution_clock::now();

    const char *filename = "bytecode.bin";

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

    std::cout << "registers[0] (int32): " << vm.GetRegisters()[0].m_value.i32 << "\n";

    delete[] bytecodes;

    end = std::chrono::high_resolution_clock::now();
    auto elapsed_ms = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1>>>(end - start).count();
    std::cout << "elapsed time: " << elapsed_ms << "s\n";
}

int main()
{
    Test();

    std::cout << "Press enter to exit...";
    std::cin.get();
    return 0;
}
