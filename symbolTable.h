
#ifndef PROJECT_SYMBOLTABLE_H
#define PROJECT_SYMBOLTABLE_H

#include "definitions.h"

/*struct that represents symbol*/
typedef struct
{
    char name [MAX_LABEL_LENGTH];
    int data;
    bool isExternal;
    symbolType type;

}symbol;


typedef struct symbolNode
{
    symbol symbol1;
    struct symbolNode *next;

}symbolNode;

/*represents a table that contains a nodes of symbol*/
typedef struct
{
    symbolNode *head;
}symbolTable;

/*info about these functions are inside the C file*/
symbolTable * createSymbolTable();
bool addSymbol(symbolTable *table, char *name, int data, bool isExternal, symbolType type);
bool isSymbolNameExists(char *name, symbolTable *table);
void updateSymbolTable(symbolTable *table, int ic);
int getSymbolValue(symbolTable *table, char *name);
bool isSymbolExternal(symbolTable *table, char *name);
void freeNodesSymbol(symbolTable *table);


void printTable(symbolTable *table);/*Debug*/


#endif