#include "codeTable.h"
#include "encodeCommands.h"
#include <stdlib.h>
#include <stdio.h>

codeNode *createCodeNode(memory [],int);
int operandConsumption(addressingType type);

codeTable * createCodeTable()
{
    codeTable *result;
    result = (codeTable *)malloc(sizeof(codeTable));

    MEMORY_ALLOC_CHECK(result)

    result->head = NULL;
    return result;
}

codeNode *createCodeNode(memory words[] ,int size)
{
    int i;
    codeNode *head, *current, *previous;
    head = (codeNode *)calloc(size,sizeof(codeNode));
    MEMORY_ALLOC_CHECK(head)

    current = head;
    for (i = 0; i< size; i++)
    {
        current->memoryUnit = words[i];
        previous = current;

        /*dont link to the next node if the last one*/
        if (size -1 != i)
        {
            current++;
            previous->next = current;
        }
    }
    free(words);
        return head;
}


bool addCommandsToDataTable(codeTable *table, struct lineInfo *info)
{
    int wordsCount = getIC(*info);
    codeNode *node = table->head;
    memory *words = ( memory*) calloc(wordsCount, sizeof(memory));
    MEMORY_ALLOC_CHECK(words)

    encode(words, *info);

    if (node == NULL)
        table->head = createCodeNode(words,wordsCount);
    else
    {
        while (node->next != NULL)
            node = node->next;
        node->next = createCodeNode(words,wordsCount);
    }
    return true;
}

/*calculates how many words our command takes with it's operands
 * parameter -info is the info about our line that we are checking for.
 * returns the number of words our command will take in our code table.
 * */
int getIC(struct lineInfo info)
{
    /*how many memory units the command takes*/
    int commandItself = 1;
    commandOperandsCount operandsCount = info.operandsCountType;
    addressingType sourceOperand = info.sourceOperandAddress;
    addressingType destOperand = info.destOperandAddress;

    switch (operandsCount)
    {
        case NO_OPERANDS:
            return commandItself;
        case ONE_OPERANDS:
            return (operandConsumption(destOperand) + commandItself);
        case TWO_OPERANDS:
            /*if both operands are registers will use the same memory unit for both*/
            if (sourceOperand == Addressing_register && destOperand == Addressing_register)
                return commandItself + 1;
            return (operandConsumption(sourceOperand) + operandConsumption(destOperand) + commandItself);

        default:
            return commandItself;
    }
}

/* gets the amount of words each operand takes in our code table.
 * -parameter type is the type of operand.
 * returns the consumption of words each operand takes.
 * */
int operandConsumption(addressingType type)
{
    switch (type)
    {
        case Addressing_immediate:
            return 1;
        case Addressing_register:
            return 1;
        case Addressing_index:
            return 2;
        case Addressing_direct:
            return 1;

        default:
            return 1;
    }
}
/*
 * frees all the nodes in the table.
 * */
void freeNodesCode(codeTable *table) {

    codeNode * head =table->head , *current, *toFree;
    current = head;

    while (current != NULL)
    {
        if (current->memoryUnit.usage == COMMAND)
        {
            toFree = current;
            current = current->next;
            free(toFree);
        }
        if (current)
            current = current->next;
    }
}



/*made of debug*/
void printCodeTable(codeTable *table)
{
    codeNode *curr = table->head;

    while (curr != NULL) {
        if (curr->memoryUnit.usage == COMMAND)
            printf("\n\nARE %d ,OPP %d, SOURCE %d, DEST %d", curr->memoryUnit.content.cmd.ARE,
                   curr->memoryUnit.content.cmd.opCode,
                   curr->memoryUnit.content.cmd.sourceOperand,
                   curr->memoryUnit.content.cmd.destinationOperand);

        if (curr->memoryUnit.usage == WORD)
        {
            printf(" \nARE %d  ,",curr->memoryUnit.content.wrd.ARE);
            printf(" Word %d",curr->memoryUnit.content.wrd.data);
        }

        if (curr->memoryUnit.usage == REGISTER)
            printf(" \nRegister source:%d, destination:%d",curr->memoryUnit.content.reg.source
                    ,curr->memoryUnit.content.reg.destination);
        if (curr->memoryUnit.usage == OPPERAND)
            printf(" \ndata %s",curr->memoryUnit.content.opp.destination);
        curr = curr->next;
    }
}
