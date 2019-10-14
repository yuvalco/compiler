
#include "symbolTable.h"
#include "parser.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

symbolNode * createSymbolNode(char name[MAX_LABEL_LENGTH], int data, bool isExternal, symbolType type);

/*
 * create a table of type symbol that will contain symbolNodes
 * returns a pointer to the table.
 * */
symbolTable * createSymbolTable()
{
    symbolTable *result;
    result = (symbolTable *)malloc(sizeof(symbolTable));

    MEMORY_ALLOC_CHECK(result)

    result->head = NULL;
    return result;
}

/*
 * checks if symbol name exists in the table
 * parameter -name is the name of the symbol we would like to check.
 * parameter -table is is a pointer to the table we are checking.
 * */
bool isSymbolNameExists(char *name, symbolTable *table) {

    symbolNode * head =table->head , *index;
    index = head;

    while (index != NULL)
    {
        if (strcmp(index->symbol1.name,name) == 0)
            return true;
        index = index->next;
    }
    return false;
}

/*
 * create a node of type symbol node and initialize it with data of a symbol that was sent as parameter.
 * parameter -name is the name of the symbol we are adding.
 * parameter -data the symbols actual data, it's a number for a macro or an address of a label.
 * parameter -isExternal is true when our symbol is an external label.
 * parameter -type is the type of the symbol that we are adding, MACRO, CODE, DATA .
 * */
symbolNode *createSymbolNode(char *name, int data, bool isExternal, symbolType type) {
    symbolNode *node;
    node = (symbolNode *)calloc(1,sizeof(symbolNode));

    MEMORY_ALLOC_CHECK(node)

    strcpy(node->symbol1.name,name);
    node->symbol1.data = data;
    node->symbol1.isExternal = isExternal;
    node->symbol1.type = type;
    return node;
}

/*this function is accessible outside this C file and it adds a symbol to our list by calling create node and adding our node as the last node to the table
 * parameter -name is the name of the symbol we are adding
 * parameter -data the symbols actual data, it's a number for a macro or an address of a label.
 * parameter -isExternal is true when our symbol is an external label.
 * parameter -type is the type of the symbol that we are adding, MACRO, CODE, DATA.
 * */
bool addSymbol(symbolTable *table, char *name, int data, bool isExternal, symbolType type) {

    symbolNode *current =table->head;

    if (isSymbolNameExists(name, table) == false) {
        if (table->head == NULL)
            table->head = createSymbolNode(name, data, isExternal, type);
        else {
            while (current->next != NULL)
                current = current->next;
            current->next = createSymbolNode(name, data, isExternal, type);
        }
        return true;
    }
    else
        return false;
}
/*
 * searches our table for a symbol by it's name.
 * parameter name is the name of the symbol we are looking for.
 * parameter table is the table in which to look for the symbol.
 * returns the symbol value, or if not found return NOT_FOUND value
 * */
int getSymbolValue(symbolTable *table, char *name)
{
    symbolNode *current = table->head;

    while (current != NULL)
    {
        if (strcmp(current->symbol1.name,name) == 0)
            return current->symbol1.data;
        current = current->next;
    }

    return NOT_FOUND;
}


bool isSymbolExternal(symbolTable *table, char *name)
{
    symbolNode *current = table->head;

    while (current != NULL)
    {
        if (strcmp(current->symbol1.name,name) == 0)
            return current->symbol1.isExternal;
        current = current->next;
    }
    return false;
}

/*updates all our symbol addresses after we done with the first pass.
 *only type data is updated since any other type doesn't contain an address to update.
 *
 * ic is the counter we are updating our addresses by.
 * */
void updateSymbolTable(symbolTable *table, int ic)
{
    symbolNode *current = table->head;

    while (current != NULL)
    {
        if (current->symbol1.type == SYMBOL_DATA)
            current->symbol1.data += ic;
        current = current->next;
    }
}

/*
 * frees all the nodes in the table.
 * */
void freeNodesSymbol(symbolTable *table) {

    symbolNode * head =table->head , *current, *toFree;
    current = head;

    while (current != NULL)
    {
        toFree = current;
        current = current->next;
        free(toFree);
    }
}



/*used to print the table, made only for debug.
 * */
void printTable(symbolTable *table)
{
    symbolNode *curr = table->head;

    while (curr != NULL)
    {
        printf("%s", curr->symbol1.name);
        printf(" data %d -> ", curr->symbol1.data);
        if (curr->symbol1.isExternal)
            printf("external->  ");
        else
            printf("not external->  ");

        switch (curr->symbol1.type)
        {
            case SYMBOL_DATA:
                printf("DATA ");
                break;
            case SYMBOL_CODE:
                printf("CODE");
                break;
            case SYMBOL_MACRO:
                printf("MACRO");
                break;
            case SYMBOL_EXTERNAL:
                printf("EXTERNAL");
                break;
        }
        printf("\n");

        curr = curr->next;
    }
}
