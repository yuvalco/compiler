#include "dataTable.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>



dataNode *  addNumber(int *,int,int);
dataNode * addString(char * string,int );
dataNode *createDataNode(int address, int word);

int DcCounter;

dataTable * createDataTable()
{
    dataTable *result;
    result = (dataTable *)malloc(sizeof(dataTable));

    MEMORY_ALLOC_CHECK(result)

    result->head = NULL;
    return result;

}

dataNode *createDataNode(int address, int word) {
    dataNode *node;
    node = (dataNode *)calloc(1,sizeof(dataNode));

    MEMORY_ALLOC_CHECK(node)

    node->address = address;
    node->wrd.data = word;
    return node;
}

/*adds String or numbers to our data table
 * -parameter table is a pointer to the table we are working on.
 * -parameter DC is the current location of our data counter.
 * -return the counter for the data we entered*/
int addData(dataTable *table,int DC,struct lineInfo info)
{
    /*gets the last node to add data to*/
    dataNode *current =  table->head;
    if (current != NULL)
    {
        while (current->next != NULL)
            current = current->next;
    }

    switch (info.instructionType1)
    {
        case INSTRUCTION_STRING:
            if (current == NULL)
                table->head = addString(info.instructionStringData,DC);
            else
                current->next = addString(info.instructionStringData,DC);
            /*returns the string we just added +1 for the character 0 to end the string*/
            return (strlen(info.instructionStringData) +1);

        case INSTRUCTION_DATA:
            if (current == NULL)
                table->head = addNumber(info.instructionData,info.pointerLength,DC);
            else
                current->next = addNumber(info.instructionData,info.pointerLength,DC);
            return DcCounter;
        default:

            return 0;
    }
}

/*  adding a string to the table
 * -parameter string is the string we are adding.
 * -parameter DC is the current location of our data counter.
 * -return a pointer to the newly created node*/
dataNode * addString(char * string,int dc)
{
    int length = strlen(string);
    int i=0;
    dataNode *startNode, *current;

    startNode = createDataNode(dc,string[i]);
    current = startNode;
    dc++;

    for (i = 1; i<length ; i++)
    {
        current->next = createDataNode(dc,string[i]);
        current = current->next;
        dc++;
    }

    current->next = createDataNode(dc,0);
    return startNode;
}

/*  adding a number to the table
 * -parameter data is pointer to the data we are adding, the data may contain more than one number.
 * -parameter size is the size of the pointer data.
 * -parameter DC is the current location of our data counter.
 * -return a pointer to the newly created node*/
dataNode * addNumber(int *data,int size,int dc)
{
    int i=0;
    dataNode *startNode, *current;

    /*global var that will count how many values we added*/
    DcCounter = 0;

    startNode = createDataNode(dc,*data);
    current = startNode;
    dc++;
    DcCounter++;

    for (i = 1; i < size;i++)
    {
        current->next = createDataNode(dc,*(data+i));
        current = current->next;
        dc++;
        DcCounter++;
    }

    free(data);
    return startNode;
}

/*  updates the data table addresses after we finish first pass.
 * -parameter table is a pointer to the table we are working on.
 * -parameter ic , our new addresses.*/
void updateDataTable(dataTable *table,int ic)
{
    dataNode *current = table->head;

    while (current != NULL)
    {
        current->address += ic;
        current = current->next;
    }
}

/*
 * frees all the nodes in the table.
 * */
void freeNodesData(dataTable *table) {

    dataNode * head =table->head , *current, *toFree;
    current = head;

    while (current != NULL)
    {
        toFree = current;
        current = current->next;
        free(toFree);
    }
}


/*made for debug*/
void printTableData(dataTable *table)
{
    dataNode *curr = table->head;

    while (curr != NULL)
    {
        printf("%d", curr->address);
        printf(" data %d ->\n", curr->wrd.data);
        curr = curr->next;
    }
}