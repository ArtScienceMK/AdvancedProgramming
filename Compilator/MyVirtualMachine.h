#ifndef _MY_VIRTUAL_MACHINE_H

#define _MY_VIRTUAL_MACHINE_H

const int MaxCodeSize        = 1200;
const int MaxExecBufferSize  = 10000;
const int MaxRegisterNameLen = 3;

const int RegistersSize      = 8;
const int MemorySize         = 100; 
const int StackCapacity      = 1000;
const int CallStackCapacity  = 100;

enum OPCODES {
    PUSH      = 0,
    POP       = 1,
    OUT       = 2,
    ADD       = 3,
    SUB       = 4,
    MUL       = 5,
    DIV       = 6,
    DUMP      = 7, 
    NO_OPCODE = 8,
    SIN       = 9,
    COS       = 10,
    SQUARE    = 11,
    SQRT      = 12,
    HALT      = 13,
    JMP       = 14,
    JA        = 15,
    JB        = 16,
    JAE       = 17,
    JBE       = 18,
    JE        = 19,
    JNE       = 20,
    CALL      = 21,
    RET       = 22,
    SKIP      = 23,
    IN        = 24,
    MDUMP     = 25
};

enum ARG_TYPES {
    NO_ARG    = 0,
    CONSTANT  = 1,
    VARIABLE  = 2,
    MEMORY    = 3
};

enum REGISTERS_NAMES {
    RAX       = 0,
    RBX       = 1,
    RCX       = 2,
    RDX       = 3,
    REX       = 4,
    RFX       = 5,
    RGX       = 6,
    RJX       = 7,
    NO_NAME   = 8
};

ERROR_STATUSES IsValidRegisterName(REGISTERS_NAMES registerName, bool* isValid);
ERROR_STATUSES IsValidRegisterNameString(const char* line,       bool* isValid);


#endif /*_MY_VIRTUAL_MACHINE_H*/