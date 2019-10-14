
#include "definitions.h"

#ifndef PROJECT_CODETABLE_H
#define PROJECT_CODETABLE_H

typedef enum {COMMAND,WORD,REGISTER,OPPERAND}usageUnion;

typedef struct command
{
    unsigned int ARE:2;
    unsigned int destinationOperand:2;
    unsigned int sourceOperand:2;
    unsigned int opCode:4;
}command;

typedef struct word
{
    unsigned int ARE:2;
    int data:12;
}word;

typedef struct regi
{
    unsigned int ARE:2;
    unsigned int destination :3;
    unsigned int source :3;

}regi;

typedef struct operands
{
    char source[MAX_LINE_LENGTH];
    char destination[MAX_LINE_LENGTH];
}operandsData;

typedef struct memory
{
    union content
    {
        /*for encoding a command*/
        command cmd;
        /*for encoding operand data*/
        word wrd;
        /*for encoding register operand*/
        regi reg;

        /*data to encode for second pass*/
        operandsData opp;
    }content;
    /*to know which data type is used in the union*/
    usageUnion usage;

}memory;

typedef struct codeNode
{
    memory memoryUnit;
    struct codeNode *next;

}codeNode;

typedef struct
{
    codeNode *head;
}codeTable;

int getIC(struct lineInfo info);
bool addCommandsToDataTable(codeTable *table, struct lineInfo *);
codeTable * createCodeTable();
void freeNodesCode(codeTable *table);

void printCodeTable(codeTable *table);

#endif
