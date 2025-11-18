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
#include <string.h>

// PAS ARRAY
#define PAS_SIZE 500
int pas[PAS_SIZE] = {0};

// Instruction Register
typedef struct IR
{
    int op, l, m;
} IR;

int base(int BP, int L)
{
    int arb = BP; // activation record base
    while (L > 0)
    {
        arb = pas[arb]; // follow static link
        L--;
    }
    return arb;
}

void print(char opr[4], int l, int m, int pc, int bp, int sp, int stackStart)
{
    printf("%s\t%d\t%d\t%d\t%d\t%d", opr, l, m, pc, bp, sp);

    // Keeps track of the AR's
    int markAR[PAS_SIZE] = {0};

    int curBP = bp;
    while (1) {
        markAR[curBP] = 1;
        int dynLink = pas[curBP - 1]; // caller's bp
        if (dynLink == curBP){
            break;
        }
        if (dynLink < 0 || dynLink > stackStart){ // checks if the link is out of bounds
            break; 
        } 
        curBP = dynLink;
    }

    // Print stack 
    printf("\t");
    for (int i = stackStart; i >= sp; i--) {
        if (i != stackStart && markAR[i]) {
            printf("|");
        }
        printf("%d ", pas[i]);
    }
    printf("\n");
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Error no input file given");
        return 1;
    }

    // Open the file
    FILE *fp;
    fp = fopen(argv[1], "r");

    if (fp == NULL) // If the file can't be opened
    {
        printf("Error Could'nt open file\n");
        return 1;
    }

    int index = PAS_SIZE - 1; // start at the last slot

    while (fscanf(fp, "%d", &pas[index]) == 1 && index >= 0)
    {
        index--; // move backwards
    }
    fclose(fp);

    // Initialize pointers
    int pc = 499;
    int sp = index + 1;
    int bp = sp - 1;

    // Print the initial values
    printf("\tL\tM\tPC\tBP\tSP\tstack\n");
    printf("Initial values: \t%d\t%d\t%d\n", pc, bp, sp);

    char opr[4] = ""; // keeps track opr for printing
    IR ir;            // Creates instance
    while (1)
    { // While True

        // Fetch
        ir.op = pas[pc];
        ir.l = pas[pc - 1];
        ir.m = pas[pc - 2];
        pc -= 3;

        // Execute
        switch (ir.op)
        {
        case 1: // LIT
            sp = sp - 1;
            pas[sp] = ir.m;
            strcpy(opr, "LIT");
            break;
        case 2: // OPR
            switch (ir.m)
            {
            case 0: // RTN
                sp = bp + 1;
                bp = pas[sp - 2];
                pc = pas[sp - 3];
                strcpy(opr, "RTN");
                break;
            case 1: // ADD
                pas[sp + 1] = (pas[sp + 1] + pas[sp]);
                sp = sp + 1;
                strcpy(opr, "ADD");
                break;
            case 2: // SUB
                pas[sp + 1] = (pas[sp + 1] - pas[sp]);
                sp = sp + 1;
                strcpy(opr, "SUB");
                break;
            case 3: // MUL
                pas[sp + 1] = (pas[sp + 1] * pas[sp]);
                sp = sp + 1;
                strcpy(opr, "MUL");
                break;
            case 4: // DIV
                pas[sp + 1] = (pas[sp + 1] / pas[sp]);
                sp = sp + 1;
                strcpy(opr, "DIV");
                break;
            case 5: // EQL
                pas[sp + 1] = (pas[sp + 1] == pas[sp]);
                sp = sp + 1;
                strcpy(opr, "EQL");
                break;
            case 6: // NEQ
                pas[sp + 1] = (pas[sp + 1] != pas[sp]);
                sp = sp + 1;
                strcpy(opr, "NEQ");
                break;
            case 7: // LSS
                pas[sp + 1] = (pas[sp + 1] < pas[sp]);
                sp = sp + 1;
                strcpy(opr, "LSS");
                break;
            case 8: // LEQ
                pas[sp + 1] = (pas[sp + 1] <= pas[sp]);
                sp = sp + 1;
                strcpy(opr, "LEQ");
                break;
            case 9: // GTR
                pas[sp + 1] = (pas[sp + 1] > pas[sp]);
                sp = sp + 1;
                strcpy(opr, "GTR");
                break;
            case 10: // GEQ
                pas[sp + 1] = (pas[sp + 1] >= pas[sp]);
                sp = sp + 1;
                strcpy(opr, "GEQ");
                break;
            case 11:
                pas[sp] = (pas[sp] % 2 == 0);
                break;
            default:
                break;
            }
            break;
        case 3: // LOD
            sp = sp - 1;
            pas[sp] = pas[base(bp, ir.l) - ir.m];
            strcpy(opr, "LOD");
            break;
        case 4: // STO
            pas[base(bp, ir.l) - ir.m] = pas[sp];
            sp = sp + 1;
            strcpy(opr, "STO");
            break;
        case 5: // CAL
            pas[sp - 1] = base(bp, ir.l);
            pas[sp - 2] = bp;
            pas[sp - 3] = pc;
            bp = sp - 1;
            pc = 499 - ir.m;
            strcpy(opr, "CAL");
            break;
        case 6: // INC
            sp = sp - ir.m;
            strcpy(opr, "INC");
            break;
        case 7: // JMP
            pc = 499 - ir.m;
            strcpy(opr, "JMP");
            break;
        case 8: // JPC
            if (pas[sp] == 0)
            {
                pc = 499 - ir.m;
            }
            sp = sp + 1;
            strcpy(opr, "JPC");
            break;
        case 9: // SYS
            switch (ir.m)
            {
            case 1: // POP
                printf("Output result is: %d\n", pas[sp]);
                sp = sp + 1;
                break;
            case 2: // Read int
            {
                int x; // initialize int being read
                sp = sp - 1;
                printf("Please Enter an Integer: ");
                int rc = scanf("%d", &x);
                if (rc != 1)
                {
                    fprintf(stderr, "Invalid integer input. Exiting.\n");
                    return 1; // or handle differently (e.g., set x=0 and continue)
                }
                pas[sp] = x;
                break;
            }
            case 3:
                print(opr, ir.l, ir.m, pc, bp, sp, index);
                return 0;

            default:
                break;
            }
            strcpy(opr, "SYS");
        default:
            break;
        }
        print(opr, ir.l, ir.m, pc, bp, sp, index);
    }
    return 0;
}