/*
Assignment:
HW4 - Complete Parser and Code Generator for PL/0
(with Procedures, Call, and Else)
Author(s): Jaden Simmons
Language: C (only)
To Compile:
Scanner:
gcc -O2 -std=c11 -o lex lex.c
Parser/Code Generator:
gcc -O2 -std=c11 -o parsercodegen_complete parsercodegen_complete.c
Virtual Machine:
gcc -O2 -std=c11 -o vm vm.c
To Execute (on Eustis):
./lex <input_file.txt>
./parsercodegen_complete
./vm elf.txt
where:
<input_file.txt> is the path to the PL/0 source program
Notes:
- lex.c accepts ONE command-line argument (input PL/0 source file)
- parsercodegen_complete.c accepts NO command-line arguments
- Input filename is hard-coded in parsercodegen_complete.c
- Implements recursive-descent parser for extended PL/0 grammar
- Supports procedures, call statements, and if-then-else
- Generates PM/0 assembly code (see Appendix A for ISA)
- VM must support EVEN instruction (OPR 0 11)
- All development and testing performed on Eustis
Class: COP3402 - System Software - Fall 2025
Instructor: Dr. Jie Lin
Due Date: Friday, November 21, 2025 at 11:59 PM ET
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Globals
#define MAX_SYMBOL_TABLE_SIZE 500
#define MAX_CODE_LENGTH 1000
typedef struct Token
{
    int type;       // Holds the actual token
    char value[12]; // Only used for identifiers and numbers
} Token;
typedef struct Symbol
{
    int kind;      // 1 = const, 2 = var
    char name[12]; // The variables name "x"
    int val;       // Only used for const holds the vars value (Ex. "5")
    int level;     // L levels
    int addr;      // stores the address for variables (Not used for constants)
    int mark;      // 0 = unused, 1 = used
} Symbol;
typedef struct Instruction
{
    int op;
    int l;
    int m;
} Instruction;
typedef enum
{
    skipsym = 1,
    identsym,
    numbersym,
    plussym,
    minussym,
    multsym,
    slashsym,
    eqsym,
    neqsym,
    lessym,
    leqsym,
    gtrsym,
    geqsym,
    lparentsym,
    rparentsym,
    commasym,
    semicolonsym,
    periodsym,
    becomessym,
    beginsym,
    endsym,
    ifsym,
    fisym,
    thensym,
    whilesym,
    dosym,
    callsym,
    constsym,
    varsym,
    procsym,
    writesym,
    readsym,
    elsesym,
    evensym
} TokenType;
typedef enum
{
    LIT = 1,
    OPR,
    LOD,
    STO,
    CAL,
    INC,
    JMP,
    JPC,
    SYS
} Opcode;
Symbol symbol_table[MAX_SYMBOL_TABLE_SIZE];
Instruction code[MAX_CODE_LENGTH];
Token currentToken; // Declares the current token globally
FILE *inFile;       // Declares the input file globally
FILE *outFile;
int symbol_table_index = 0;
int codeIndex = 0;

// Helper Functions:
void ERROR(char *message)
{
    printf("%s\n", message);

    if (outFile != NULL)
    {
        fprintf(outFile, "%s\n", message);
        fclose(outFile);
    }
    exit(1);
}
void GET_TOKEN()
{
    // Store the next read token type as the current token
    if (fscanf(inFile, "%d", &currentToken.type) == EOF)
    {
        return;
    }

    if (currentToken.type == skipsym) // If the scanner produces an error stop immediately
    {
        ERROR("Error: Scanning error detected by lexer (skipsym present)");
    }

    // If the current token is an identifier or a number then store its value as well
    if (currentToken.type == identsym || currentToken.type == numbersym)
    {
        if (fscanf(inFile, "%s", currentToken.value) != 1)
        { // Stops the warning in eustis
            ERROR("ERROR: ??");
        }
    }
    // Otherwise clear the value string so that it previous strings are erased
    else
    {
        currentToken.value[0] = '\0';
    }
}
void emit(int op, int l, int m) // Stores PM/0 instructions in an array
{
    if (codeIndex >= MAX_CODE_LENGTH)
    {
        ERROR("Error: Generated code exceeds maximum length");
    }
    code[codeIndex].op = op;
    code[codeIndex].l = l;
    code[codeIndex].m = m;
    codeIndex++;
}
char *op_to_string(int op)
{
    switch (op)
    {
    case LIT:
        return "LIT";
    case OPR:
        return "OPR";
    case LOD:
        return "LOD";
    case STO:
        return "STO";
    case CAL:
        return "CAL";
    case INC:
        return "INC";
    case JMP:
        return "JMP";
    case JPC:
        return "JPC";
    case SYS:
        return "SYS";
    default:
        return "Unknown";
    }
}

// Symbol Table Functions:
int symbol_table_check(char *name) // Returns 1 if found, 0 if not
{
    for (int i = symbol_table_index - 1; i >= 0; i--) // Search backwards to find the closest match
    {
        if (strcmp(name, symbol_table[i].name) == 0)
        {
            if (symbol_table[i].mark == 0) // If the identifier is avaiable return its index
            {
                return i;
            }
        }
    }
    return -1; // No identifier was found
}
void add_symbol_table(int kind, char *name, int val, int level, int addr, int mark)
{
    symbol_table[symbol_table_index].kind = kind;
    strcpy(symbol_table[symbol_table_index].name, name);
    symbol_table[symbol_table_index].val = val;
    symbol_table[symbol_table_index].level = level;
    symbol_table[symbol_table_index].addr = addr;
    symbol_table[symbol_table_index].mark = mark;
    symbol_table_index++;
}
// Prototypes
void EXPRESSION(int level);
int BLOCK(int level);
// Primary Functions:
void FACTOR(int level)
{
    if (currentToken.type == identsym)
    {
        int symIdx = symbol_table_check(currentToken.value);
        if (symIdx == -1)
        {
            ERROR("Error: undeclared identifier");
        }
        if (symbol_table[symIdx].kind == 1) // Must be a const
        {
            emit(LIT, 0, symbol_table[symIdx].val);
        }
        else
        {
            emit(LOD, level - symbol_table[symIdx].level, symbol_table[symIdx].addr);
        }
        GET_TOKEN(); // Next Token
    }
    else if (currentToken.type == numbersym)
    {
        emit(LIT, 0, atoi(currentToken.value));
        GET_TOKEN();
    }
    else if (currentToken.type == lparentsym)
    {
        GET_TOKEN();
        EXPRESSION(level);
        if (currentToken.type != rparentsym)
        {
            ERROR("Error: right parenthesis must follow left parenthesis");
        }
        GET_TOKEN();
    }
    else
    {
        ERROR("Error: arithmetic equations must contain operands, parentheses, numbers, or symbols");
    }
}
void TERM(int level)
{
    FACTOR(level);
    while (currentToken.type == multsym || currentToken.type == slashsym)
    {
        if (currentToken.type == multsym)
        {
            GET_TOKEN();
            FACTOR(level);
            emit(OPR, 0, 3);
        }
        else if (currentToken.type == slashsym)
        {
            GET_TOKEN();
            FACTOR(level);
            emit(OPR, 0, 4);
        }
    }
}
void EXPRESSION(int level)
{
    if (currentToken.type == minussym)
    {
        GET_TOKEN();
        TERM(level);
        emit(LIT, 0, 0);
        emit(OPR, 0, 2);

        while (currentToken.type == plussym || currentToken.type == minussym)
        {
            if (currentToken.type == plussym)
            {
                GET_TOKEN();
                TERM(level);
                emit(OPR, 0, 1);
            }
            else
            {
                GET_TOKEN();
                TERM(level);
                emit(OPR, 0, 2);
            }
        }
    }
    else
    {
        if (currentToken.type == plussym)
        {
            GET_TOKEN();
        }
        TERM(level);
        while (currentToken.type == plussym || currentToken.type == minussym)
        {
            if (currentToken.type == plussym)
            {
                GET_TOKEN();
                TERM(level);
                emit(OPR, 0, 1);
            }
            else
            {
                GET_TOKEN();
                TERM(level);
                emit(OPR, 0, 2);
            }
        }
    }
}
void CONDITION(int level)
{
    if (currentToken.type == evensym)
    {
        GET_TOKEN();
        EXPRESSION(level);
        emit(OPR, 0, 11);
    }
    else
    {
        EXPRESSION(level);
        if (currentToken.type == eqsym)
        {
            GET_TOKEN();
            EXPRESSION(level);
            emit(OPR, 0, 5);
        }
        else if (currentToken.type == neqsym)
        {
            GET_TOKEN();
            EXPRESSION(level);
            emit(OPR, 0, 6);
        }
        else if (currentToken.type == lessym)
        {
            GET_TOKEN();
            EXPRESSION(level);
            emit(OPR, 0, 7);
        }
        else if (currentToken.type == leqsym)
        {
            GET_TOKEN();
            EXPRESSION(level);
            emit(OPR, 0, 8);
        }
        else if (currentToken.type == gtrsym)
        {
            GET_TOKEN();
            EXPRESSION(level);
            emit(OPR, 0, 9);
        }
        else if (currentToken.type == geqsym)
        {
            GET_TOKEN();
            EXPRESSION(level);
            emit(OPR, 0, 10);
        }
        else
        {
            ERROR("Error: condition must contain comparison operator");
        }
    }
}
void STATEMENT(int level)
{
    if (currentToken.type == identsym) // If the token is an identifier
    {
        int symIdx = symbol_table_check(currentToken.value); // Check if the identifier is declared
        if (symIdx == -1) // If not error
        {
            ERROR("Error: undeclared identifier");
        }
        if (symbol_table[symIdx].kind != 2) // Must be a var
        {
            ERROR("Error: only variable values may be altered");
        }
        GET_TOKEN();                         // Get the next token
        if (currentToken.type != becomessym) // Next token must be the become symbol
        {
            ERROR("Error: assignment statements must use :=");
        }
        GET_TOKEN(); // Next token shopuld be the value that you setting to the identifier
        EXPRESSION(level);
        emit(STO, level - symbol_table[symIdx].level, symbol_table[symIdx].addr);
        return;
    }
    if (currentToken.type == beginsym)
    {
        do
        {
            GET_TOKEN();
            STATEMENT(level);
        } while (currentToken.type == semicolonsym);
        if (currentToken.type != endsym)
        {
            ERROR("Error: begin must be followed by end");
        }
        GET_TOKEN();
        return;
    }
    if (currentToken.type == ifsym)
    {
        GET_TOKEN();
        CONDITION(level);
        int jpcIdx = codeIndex;
        emit(JPC, 0, 0);
        if (currentToken.type != thensym)
        {
            ERROR("Error: if must be followed by then");
        }
        GET_TOKEN();
        STATEMENT(level);
        if (currentToken.type != elsesym)
        {
            ERROR("Error: if statement must include else clause");
        }
        GET_TOKEN();

        int jmpIndex = codeIndex;
        emit(JMP, 0, 0);

        code[jpcIdx].m = codeIndex;

        STATEMENT(level);

        if (currentToken.type != fisym)
        {
            ERROR("Error: else must be followed by fi");
        }
        GET_TOKEN();
        code[jmpIndex].m = codeIndex;
        return;
    }
    if (currentToken.type == whilesym)
    {
        GET_TOKEN();
        int loopIdx = codeIndex;
        CONDITION(level);
        if (currentToken.type != dosym)
        {
            ERROR("Error: while must be followed by do");
        }
        GET_TOKEN();
        int jpcIdx = codeIndex;
        emit(JPC, 0, 0);
        STATEMENT(level);
        emit(JMP, 0, loopIdx);
        code[jpcIdx].m = codeIndex;
        return;
    }
    if (currentToken.type == readsym)
    {
        GET_TOKEN();
        if (currentToken.type != identsym)
        {
            ERROR("Error: const, var, and read keywords must be followed by identifier");
        }
        int symIdx = symbol_table_check(currentToken.value);
        if (symIdx == -1)
        {
            ERROR("Error: undeclared identifier");
        }
        if (symbol_table[symIdx].kind != 2)
        {
            ERROR("Error: only variable values may be altered");
        }
        GET_TOKEN();
        emit(SYS, 0, 2);
        emit(STO, level - symbol_table[symIdx].level, symbol_table[symIdx].addr);
        return;
    }
    if (currentToken.type == callsym)
    {
        GET_TOKEN(); // Must be an identifier
        if (currentToken.type != identsym)
        {
            ERROR("Error: const, var, read, procedure, and call keywords must be followed by identifier");
        }

        int symIdx = symbol_table_check(currentToken.value);
        if (symIdx == -1)
        {
            ERROR("Error: undeclared identifier");
        }

        if (symbol_table[symIdx].kind != 3) // Must be a Procedure
        {
            ERROR("Error: call statement may only target procedures");
        }
        emit(CAL, level - symbol_table[symIdx].level, symbol_table[symIdx].addr);

        GET_TOKEN();
        return;
    }
    if (currentToken.type == writesym)
    {
        GET_TOKEN();
        EXPRESSION(level);
        emit(SYS, 0, 1);
        return;
    }
}
void PROCEDURE_DECLARATION(int level)
{
    while (currentToken.type == procsym)
    {
        GET_TOKEN();                       // Next token should be an identifier
        if (currentToken.type != identsym) // If not error
        {
            ERROR("Error: const, var, read, procedure, and call keywords must be followed by identifier");
        }

        char procName[12];
        strcpy(procName, currentToken.value);
        if (symbol_table_check(procName) != -1)
        {
            ERROR("Error: symbol name has already been declared");
        }

        GET_TOKEN();                           // Next token should be semicolon
        if (currentToken.type != semicolonsym) // If not error
        {
            ERROR("Error: procedure declaration must be followed by a semicolon");
        }
        GET_TOKEN();
        add_symbol_table(3, procName, 0, level, codeIndex * 3, 0); // procedure kind = 3

        int jmpIndex = codeIndex;
        emit(JMP, 0 , 0); // Emit placeholder
        int mainBlockLocation = BLOCK(level + 1);
        code[jmpIndex].m = mainBlockLocation * 3;

        if (currentToken.type != semicolonsym)
        {
            ERROR("Error: procedure declaration must be followed by a semicolon");
        }
        GET_TOKEN();
    }
}
int VAR_DECLARATION(int level) // Returns number of variables
{
    int numVars = 0;
    // If the token is a var
    if (currentToken.type == varsym)
    {
        do
        {
            numVars++;
            GET_TOKEN();
            // The next token must be an identifier
            if (currentToken.type != identsym)
            {
                ERROR("Error: const, var, and read keywords must be followed by identifier");
            }
            // sybolTableCheck and add to table
            if (symbol_table_check(currentToken.value) != -1)
            {
                ERROR("Error: symbol name has already been declared");
            }
            add_symbol_table(2, currentToken.value, 0, level, numVars + 2, 0);
            GET_TOKEN();
        } while (currentToken.type == commasym);
        if (currentToken.type != semicolonsym)
        {
            ERROR("Error: constant and variable declarations must be followed by a semicolon");
        }
        GET_TOKEN();
    }
    return numVars;
}
void CONST_DECLARATION(int level)
{
    char constName[12];
    if (currentToken.type == constsym)
    {
        do // runs atleast once
        {
            GET_TOKEN(); // Consumes "const" or ","
            // Next token must be an identifier
            if (currentToken.type != identsym)
            {
                ERROR("Error: const, var, and read keywords must be followed by identifier");
            }
            // If the symbol is found Error
            if (symbol_table_check(currentToken.value) != -1)
            {
                ERROR("Error: symbol name has already been declared");
            }
            // Save the name given to the identifier
            strcpy(constName, currentToken.value);
            // Get next token
            GET_TOKEN();
            // Must be an "=" sign
            if (currentToken.type != eqsym)
            {
                ERROR("Error: constants must be assigned with =");
            }
            // Next token
            GET_TOKEN();
            // Must be a number
            if (currentToken.type != numbersym)
            {
                ERROR("Error: constants must be assigned an integer value");
            }
            // Add symbol to table
            int const_val = atoi(currentToken.value);
            add_symbol_table(1, constName, const_val, level, 0, 0);
            // Next token
            GET_TOKEN();
        } while (currentToken.type == commasym); // If there is a , after running then run again
        if (currentToken.type != semicolonsym) // If the last token in the line is not a semicolon
        {
            ERROR("Error: constant and variable declarations must be followed by a semicolon");
        }
        GET_TOKEN();
    }
}
int BLOCK(int level)
{
    CONST_DECLARATION(level);
    int numVars = VAR_DECLARATION(level);
    PROCEDURE_DECLARATION(level);
    
    int codeStart = codeIndex;
    // todo idk if this is right???
    // if (level > 0)
    // {
    //     emit(JMP, 0, (codeIndex * 3) + 3); // jmp to the start of the procedures logic
    // }
    emit(INC, 0, 3 + numVars);
    STATEMENT(level);

    if (level > 0) // If we are in a procedure we must return to the caller
    {
        emit(OPR, 0, 0); // emit RTN
    }

    // Marks all of the symbols in the table as available/unavailable
    for (int i = symbol_table_index - 1; i >= 0; i--)
    {
        if (symbol_table[i].level == level && symbol_table[i].mark == 0)
        {
            symbol_table[i].mark = 1; // 1 = Unavailable
        }
    }

    return codeStart;
}
void PROGRAM()
{
    int jmpIndex = codeIndex; // will always be 0 (start of code)

    emit(JMP, 0, 0); // Emit placeholder 0 for later backpatching

    int mainBlockLocation = BLOCK(0); // Block will generate the code for the whole program and then return the jmpIndex for the main block

    code[jmpIndex].m = mainBlockLocation * 3; // Multiply by 3 because each code index contains 3 slots of data

    if (currentToken.type != periodsym) // Program must end with period
    {
        ERROR("Error: program must end with period");
    }
    emit(SYS, 0, 3); // emit HALT
}
int main()
{
    inFile = fopen("lex_output.txt", "r");
    outFile = fopen("elf.txt", "w");
    GET_TOKEN(); // Gets the first token
    PROGRAM();   // Start the parser

    //* Handles printing
    printf("Assembly Code:\n\n");
    printf("Line\tOP\tL\tM\n");
    // Print the code array to elf.txt and terminal
    for (int i = 0; i < codeIndex; i++)
    {
        printf("%d\t%s\t%d\t%d\n", i, op_to_string(code[i].op), code[i].l, code[i].m);
        fprintf(outFile, "%d %d %d\n", code[i].op, code[i].l, code[i].m);
    }
    fclose(outFile);
    printf("\n"); // Seperator

    // Print the symbol table
    printf("Symbol Table:\n\n");
    printf("%-6s | %-15s | %-10s | %-6s | %-8s | %-4s\n", "Kind", "Name", "Value", "Level", "Address", "Mark");
    printf("---------------------------------------------------------------\n");
    for (int i = 0; i < symbol_table_index; i++)
    {
        printf("%-6d | %-15s | %-10d | %-6d | %-8d | %-4d\n", symbol_table[i].kind, symbol_table[i].name, symbol_table[i].val, symbol_table[i].level, symbol_table[i].addr, symbol_table[i].mark);
    }
}
