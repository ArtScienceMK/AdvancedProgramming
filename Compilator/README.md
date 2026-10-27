<a name="top"></a>

# Assembler

[![getting started](logo.png)](#-quickstart)

## Table of contents
- [About](#-about)
- [Quickstart](#-quickstart)
- [Dependencies](#-dependencies)
- [Building](#-building)
- [What's New](#-whats-new)
- [Documentation](#-documentation)
- [Contacts](#%EF%B8%8F-contacts)

## About

**Assembler** V2.0 is a powerful tool, which allows you to translate ASM-like language into binary code. Moreover, you can use our VirtualMachine to execute such code. Here some key facts about this project:  

* minimal Turing-complete VM
* Only integers available
* Registers supported
* RAM supported (allocated in program)

## Feature list 

1. Basic math operations supported! (+, -, *, /, sin, cos, square, sqrt)
2. Jumps on the certain line supported!
3. Jumps on the numeric labels address supported!
4. Creating functions supported!
5. Comments supported!
6. Spaces and tabs on the beginning on the line supported!
7. Program, calculating roots of the quadratic equations, is available!
8. Program, recursively calculating the factorial, is available!
9. Program, sketching circle, is available!

## Quickstart

Download latest version from releases :) That's it!

## Dependencies

The list of dependencies:
1) g++ compiler 16.2.1 and newer

## Building

If you want to build this project on your machine follow this steps:

0) Check dependencies
1) ``` mkdir Assembler ```
2) ``` cd Assembler ```
3) ``` git clone https://github.com/ArtScienceMK/AdvancedProgramming/tree/main/Compilator ```
If you want to compile assembler
4) ``` g++ Assembler.cpp -o asm ``` to disable any debug output

   ```  g++ Assembler.cpp -DDEBUG -DSTACK_DEBUG -o asm ``` to enable all debug output into console and activate asserts
5) ``` ./asm ```
If you want to compile virtual machine
4) ``` g++ VirtualMachine.cpp -o asm ``` to disable any debug output

   ```  g++ VirtualMachine.cpp -DDEBUG -DSTACK_DEBUG -o asm ``` to enable all debug output into console and activate asserts
5) ``` ./asm ```

Congratulations!  


## What's New

Added processing of command line arguments! Following flags are available in assembler and virtual machine:
* -i=[Path to file] Reading from file
* -o=[Path to file] Writing to file
* -h Showing a manual

## Documentation

### Instruction structure:

``` 
struct Comand {
    OPCODES opcode;
    ARG_TYPES argType;
    StackElement_t argValue;
    REGISTERS_NAMES registerName;
    
    struct {
        int jumpLineIndex;
        int memoryIndex; 
    };
};
```

### Byte code format:

No separators, one value to one integer.

opcode1, argType1, argValue1, registerName1, jumpLineIndex1, memoryIndex1, opcode 2, ....

### Basic instructions:

1) PUSH value - pushes value into the stack. Value may be an constant integer, register name (e.g. RAX, RBX, ..., RJX) or memory index (e.g. PUSH [12] pushes value = memory[12] into the stack)
2) POP (Nothing or RAX or [memoryIndex]) - pops value out of the stack (if it's not empty), this value may be saved in register (e.g. POP RAX, saves popped element in RAX) or memory (e.g. POP [12], saves popped element in memory[12])
3) OUT - pops value and prints it to the console
4) ADD - pops first two values, adds them and pushes result in the stack
5) SUB - pops first two values, subtracts them and pushes result in the stack
6) MUL - pops first two values, multiplies them and pushes result in the stack
7) DIV - pops first two values, divides them and pushes result in the stack
8) DUMP - calling StackDump()
9) NO_OPCODE - enum init value
10) SIN - pops first value, calculates sin, pushes in the stack
11) COS - pops first value, calculates cos, pushes in the stack
12) SQUARE - pops first value, squares it, pushes in the stack
13) SQRT - pops first value, calculates square root, pushes in the stack
14) HALT - immediately stops program

### Labels:
You may created numeric labels e.g. :1, :2 to make a destination point for jump.

### Comments and spaces/tabs:
String starting with ";" (without quotes) are comments, they are ignored. Spaces and tabs are ignored, too.

### More advanced instructions

15) JMP (constant integer or numeric label (e.g. :1, :2)) - continue executing on the line, code of which you provided
16) JA  (constant integer or numeric label (e.g. :1, :2)) - op2 = POP, op1 = POP, jumps if op1 > op2
17) JB  (constant integer or numeric label (e.g. :1, :2)) - op2 = POP, op1 = POP, jumps if op1 < op2
18) JAE  (constant integer or numeric label (e.g. :1, :2)) - op2 = POP, op1 = POP, jumps if op1 >= op2
19) JBE  (constant integer or numeric label (e.g. :1, :2)) - op2 = POP, op1 = POP, jumps if op1 <= op2
16) JA  (constant integer or numeric label (e.g. :1, :2)) - op2 = POP, op1 = POP, jumps if op1 > op2
20) JE  (constant integer or numeric label (e.g. :1, :2)) - op2 = POP, op1 = POP, jumps if op1 == op2
21) JNE  (constant integer or numeric label (e.g. :1, :2)) - op2 = POP, op1 = POP, jumps if op1 != op2
22) CALL  (constant integer or numeric label (e.g. :1, :2)) enters a function (adds cur instruction pointer + 1 to call stack (destination point when returning from function), jumps to line)
23) RET (constant integer or numeric label (e.g. :1, :2)) exits a function (pops return address from call stack then jumps to it)
24) SKIP - immediately goes to next instruction 
25) IN - reading a constant integer from stdin, then pushes it to the stack
26) MDUMP - prints the memory as a matrix (by default it 10x10)

### Code sample:

``` 
;  Calculating factorial 
   IN
   POP RBX
   PUSH RBX
   CALL :1
   PUSH RAX
   OUT
   HALT
   :1
   PUSH 1
   JNE :2
   ;  IF TOP == 1
      PUSH 1
      POP RAX
      RET
   :2
   PUSH RBX
   PUSH 1
   SUB
   POP RBX
   PUSH RBX
   CALL :1
   PUSH RBX
   PUSH 1
   ADD
   POP RBX
   PUSH RBX
   PUSH RAX 
   MUL
   POP RAX
   RET
   HALT 
```


## Contacts

If you will have any questions, suggestions and so on, just text me)

Mail: 55555artem.kulakov@gmail.com
