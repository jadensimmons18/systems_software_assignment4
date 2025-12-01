# Custom-Virtual-Machine-Language-Parser

**Author:** Jaden Simmons  
**Language:** C (C11 Standard)

## Project Overview

This project is a complete implementation of a Compiler and Virtual Machine for the **PL/0** programming language (a simplified version of Pascal). It demonstrates the full lifecycle of software execution, from raw source code analysis to low-level stack machine execution.

The system is built as a pipeline of three distinct components:
1.  **Lexical Analyzer (Scanner):** Tokenizes the input source code.
2.  **Recursive Descent Parser & Code Generator:** Validates syntax and generates P-Code (Assembly) for the abstract machine.
3.  **Virtual Machine (PM/0):** A stack-based architecture that executes the generated assembly code.

## Architecture & Features

### 1. Lexical Analysis (`lex.c`)
* **Tokenization:** Converts raw character streams into meaningful tokens (identifiers, reserved words, operators).
* **Comment Handling:** Supports C-style comments (`/* ... */`).
* **Error Detection:** Identifies invalid symbols and variable naming constraints (e.g., max length).

### 2. Parser & Code Generation (`parsercodegen_complete.c`)
* **Recursive Descent Parsing:** Implements a top-down parser to handle extended PL/0 grammar.
* **Control Flow:** Fully supports complex structures including `if-then-else` conditionals and `while-do` loops.
* **Procedures & Scope:** Manages nested procedure declarations (`procedure`) and calls (`call`).
* **Symbol Table:** Tracks variables, constants, and procedures with support for scoping levels.
* **Code Generation:** Emits **PM/0 Assembly** (P-Code) to an ELF (Executable and Linkable Format) file.

### 3. Virtual Machine (`vm.c`)
* **ISA Implementation:** Simulates a stack-based computer (PM/0) with a specific Instruction Set Architecture.
* **Memory Management:** Manages a process stack with **Activation Records (AR)** for function calls.
* **Registers:** Simulates `PC` (Program Counter), `BP` (Base Pointer), and `SP` (Stack Pointer).
* **Static Link Traversal:** Implements logic to traverse stack frames to access variables in parent scopes.

## ⚙️ Installation & Compilation

Ensure you have a GCC compiler installed. Clone the repository and compile the components using the following commands:

```bash
# 1. Compile the Lexical Analyzer
gcc -O2 -std=c11 -o lex lex.c

# 2. Compile the Parser and Code Generator
gcc -O2 -std=c11 -o parsercodegen_complete parsercodegen_complete.c

# 3. Compile the Virtual Machine
gcc -O2 -std=c11 -o vm vm.c
```
## Execution

```bash
# 1. Run the Lexical Analyzer with the input file as a command line argument
./lex input.txt

# 2. Run the Parser and Code Generator
./parsercode_complete

# 3. Run the Virtual Machine
./vm elf.txt
```

## Instruction Set Architecture

01,LIT,Push a literal constant onto the stack.
02,OPR,"Arithmetic/Logic (ADD, SUB, MUL, DIV, EQL, NEQ, LSS, LEQ, GTR, GEQ, ODD)."
03,LOD,Load value to top of stack from a specific offset.
04,STO,Store value from top of stack to a specific offset.
05,CAL,Call a procedure (generates new Activation Record).
06,INC,Increment stack pointer (allocate memory).
07,JMP,Unconditional jump (Program Counter modification).
08,JPC,Jump conditional (Jump if top of stack is 0).
09,SYS,"System calls (1=Print, 2=Read, 3=Halt).
