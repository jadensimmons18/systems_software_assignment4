#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

/*
Assignment :
lex - Lexical Analyzer for PL /0
Author : Jaden Simmons
Language : C ( only )
To Compile :
gcc - O2 - std = c11
-o lex lex . c
To Execute ( on Eustis ):
./ lex < input file >
where :
< input file > is the path to the PL /0 source program
Notes :
- Implement a lexical analyser for the PL /0 language .
- The program must detect errors such as
- numbers longer than five digits
- identifiers longer than eleven characters
- invalid characters .
- The output format must exactly match the specification .
- Tested on Eustis .
Class : COP 3402 - System Software - Fall 2025
Instructor : Dr . Jie Lin
Due Date : Friday , October 3 , 2025 at 11:59 PM ET
*/

#define MAX_WORD 11
#define MAX_NUMBER 5

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

typedef struct Token
{
    char *lexeme;
    int token;
} Token;

Token reservedWordArr[] = 
{
    {"begin", beginsym},
    {"end", endsym},
    {"if", ifsym},
    {"fi", fisym},
    {"then", thensym},
    {"while", whilesym},
    {"do", dosym},
    {"call", callsym},
    {"const", constsym},
    {"var", varsym},
    {"procedure", procsym},
    {"write", writesym},
    {"read", readsym},
    {"else", elsesym},
    {"even", evensym}
};

Token specialSymbolArr[] = 
{
    {"+", plussym},
    {"-", minussym},
    {"*", multsym},
    {"/", slashsym},
    {"=", eqsym},
    {"<", lessym},
    {">", gtrsym},
    {"(", lparentsym},
    {")", rparentsym},
    {",", commasym},
    {";", semicolonsym},
    {".", periodsym}
};

int copySrcToArray(FILE *fp, char **arr, int *arrSize)
{
    int curIndex = 0;
    while (1)
    {
        int c = fgetc(fp); // Get each char from the file

        if (c == EOF)
        {
            return curIndex;
        }

        if (curIndex >= *arrSize)
        {
            *arrSize *= 2;
            *arr = realloc(*arr, *arrSize * sizeof(char));

            if (*arr == NULL)
            {
                printf("Reallocation failed\n");
                return curIndex;
            }
        }

        (*arr)[curIndex] = c;
        curIndex++;
    }
    return curIndex;
}

int getToken(char *word, int reservedWordArrLen)
{
    for (int j = 0; j < reservedWordArrLen; j++) // For the length of reserved words
    {
        if (strcmp(word, reservedWordArr[j].lexeme) == 0) // If the word is in reserved words print its token
        {
            return reservedWordArr[j].token; // Stops the loop once the word is found
        }
    }
    return identsym;
}

int getSingleSymbol(char symbol, int specialSymbolArrLen)
{
    char s[2] = {symbol, '\0'};
    for (int j = 0; j < specialSymbolArrLen; j++)
    {
        if (strcmp(s, specialSymbolArr[j].lexeme) == 0)
        {
            return specialSymbolArr[j].token;
        }
    }
    return skipsym;
}

void addToken(Token *tokenList, int *tokenListIndex, char *lexeme, int token)
{
    tokenList[*tokenListIndex].lexeme = malloc(strlen(lexeme) + 1);
    strcpy(tokenList[*tokenListIndex].lexeme, lexeme);

    tokenList[*tokenListIndex].token = token;

    (*tokenListIndex)++;
}

//! Remember to change this back to a command argument for the input file
int main(int argc, char *argv[])
{
    int reservedWordArrLen = sizeof(reservedWordArr) / sizeof(reservedWordArr[0]);
    int specialSymbolsArrLen = sizeof(specialSymbolArr) / sizeof(specialSymbolArr[0]);

    if (argc != 2)
    {
        fprintf(stderr, "Incorrect number of arguments\n");
        return 1;
    }

    FILE *inFile = fopen(argv[1], "r"); // Open the file
    FILE *outFile = fopen("lex_output.txt", "w"); // Open the file for writing

    if (outFile == NULL) // can't open output file
    {
        printf("Error opening output file!\n");
        return 1;
    }
    if (inFile == NULL) // can't open input file
    {
        printf("Error opening input file!\n");
        return 1;
    }

    // Allocate space for two arrays one for the input file and one for the token list
    int arrSize = 2; // Set the initial size of the dynamic array
    int tokenArrSize = 500;
    int tokenListIndex = 0;
    char *arr = malloc(sizeof(char) * arrSize); // Creates a dynamic array to store the input file
    Token *tokenList = malloc(sizeof(Token) * tokenArrSize);

    if (arr == NULL) // Check if allocation failed
    {
        printf("Memory allocation failed!\n");
        return 1;
    }
    int charsRead = copySrcToArray(inFile, &arr, &arrSize); // adds everything from src input file to array and return the last index
    // Read the array
    for (int i = 0; i < charsRead; i++)
    {
        if (isspace((unsigned char)arr[i]))
            continue;
        if (i + 1 < charsRead && arr[i] == '/' && arr[i + 1] == '*')
        {
            int j = i + 2; // start scanning after /*
            while (j + 1 < charsRead && !(arr[j] == '*' && arr[j + 1] == '/'))
            {
                j++;
            }
            if (j + 1 >= charsRead) // No closing comment delimeter was found "*/"
            {
                // Do nothing
            }
            else
            {
                i = j + 2;
                continue;
            }
            
        }

        else if (isalpha(arr[i])) // If it is a letter
        {
            char word[MAX_WORD + 1];
            int wordIndex = 0;

            while (isalpha(arr[i]) || isdigit(arr[i])) // Iterates until anything other than a letter or num is found
            {
                if (wordIndex < MAX_WORD)
                {
                    word[wordIndex] = arr[i]; // Add chars to word until max word length
                }
                i++;
                wordIndex++;
            }
            i--;
            

            if (wordIndex > MAX_WORD)
            { // word is too long
                addToken(tokenList, &tokenListIndex, "1", skipsym);
                word[MAX_WORD] = '\0';
            }
            else
            {   
                word[wordIndex] = '\0';
                int token = getToken(word, reservedWordArrLen);
                addToken(tokenList, &tokenListIndex, word, token);
            }
        }
        else if (isdigit(arr[i])) // If it is a number
        {
            char number[MAX_NUMBER + 1];
            int numberIndex = 0;

            while (isdigit(arr[i])) // Iterates until anything other than a num is found
            {
                if (numberIndex < MAX_NUMBER)
                {
                    number[numberIndex] = arr[i]; // Add numbers to number until max number length
                }
                i++;
                numberIndex++;
            }
            i--;
            
            if (numberIndex > MAX_NUMBER)
            { // number is too long
                number[MAX_NUMBER] = '\0';
                addToken(tokenList, &tokenListIndex, "1", skipsym);
            }
            else
            {   
                number[numberIndex] = '\0';
                addToken(tokenList, &tokenListIndex, number, numbersym);
            }
        }
        else // Check if its a symbol
        {
            int canDouble = (i + 1 < charsRead);

            if (arr[i] == '<' && canDouble) // Try double symbols first
            {
                if (arr[i + 1] == '>')
                {
                    addToken(tokenList, &tokenListIndex, "<>", neqsym);
                    i++;
                }
                else if (arr[i + 1] == '=')
                {
                    addToken(tokenList, &tokenListIndex, "<=", leqsym);
                    i++;
                }
            }
            else if (arr[i] == '>' && canDouble)
            {
                if (arr[i + 1] == '=')
                {
                    addToken(tokenList, &tokenListIndex, ">=", geqsym);
                    
                    i++;
                }
            }
            else if (arr[i] == ':' && canDouble)
            {
                if (arr[i + 1] == '=')
                {
                    addToken(tokenList, &tokenListIndex, ":=", becomessym);
                    
                    i++;
                }
            }
            else
            { // Try single symbols
                int symbol = getSingleSymbol(arr[i], specialSymbolsArrLen);

                if (symbol == skipsym)
                { // Symbol does not exist
                    addToken(tokenList, &tokenListIndex, "1", symbol);
                    printf("%c\tInvalid", arr[i]);
                }
                else
                {
                    char lexeme[2];
                    lexeme[0] = arr[i];
                    lexeme[1] = '\0';
                    addToken(tokenList, &tokenListIndex, lexeme, symbol);
                }
            }
        }

    }

    // Print the token list to output file
    for (int i = 0; i < tokenListIndex; i++)
    {
        if (tokenList[i].token == identsym) // If its an identifier, print the identifier symbol and then the identifier
        {
            fprintf(outFile, "2 %s ", tokenList[i].lexeme);
        }
        else if (tokenList[i].token == numbersym) // If its a number, print the number symbol and then the number
        {
            fprintf(outFile, "3 %s ", tokenList[i].lexeme);
        }
        else // Otherwise, just print the token
        {
            fprintf(outFile, "%d ", tokenList[i].token);
        }
    }
    
    // Close Files
    fclose(outFile);
    fclose(inFile);
    // Free memory
    free(arr);
    free(tokenList);
    return 0;
}
