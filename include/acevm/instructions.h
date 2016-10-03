#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

// parameters are marked in square brackets
// i8 = 8-bit integer (1 byte)
// i32 = 32-bit integer (4 bytes)
// i64 = 64-bit integer (8 bytes)
// u8 = 8-bit unsigned integer (1 byte)
// u32 = 32-bit unsigned integer (4 bytes)
// u64 = 64-bit unsigned integer (8 bytes)
// f = float (4 bytes)
// d = double (8 bytes)
// $ = local variable index (2 bytes)
// % = register (1 byte)
// @ = address (4 bytes)

enum Instructions : char {
    PUSH_I32, // push_i32 [i32 value]
    PUSH_I64, // push_i64 [i64 value]
    PUSH_FLOAT, // push_f [f value]
    PUSH_DOUBLE, // push_d [d value]
    PUSH_ADDRESS, // push_addr [@ address]
    PUSH_REG, // push_reg [% register]
    POP, // pop

    MOV, // mov [$ index, % register]

    JMP, // jmp [% condition, % address]
    JT, // jt [% condition, % address]
    JF, // jf [% condition, % address]

    CALL, // call [% function, u8 argc ]
    RET, // ret

    ADD, 
    SUB,
    MUL,
    DIV,
    MOD,

    EXIT,

};

#endif