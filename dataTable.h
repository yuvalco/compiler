
#ifndef PROJECT_DATATABLE_H
#define PROJECT_DATATABLE_H

#include "definitions.h"

typedef struct wordData
{
    signed int data:14;
}wordData;

typedef struct dataNode
{
    wordData wrd;
    int address;
    struct dataNode *next;
}dataNode;

typedef struct
{
    dataNode *head;
}dataTable;


/*functions that accessed from other C files. */
dataTable * createDataTable();
int addData(dataTable *table,int DC,struct lineInfo info);
void updateDataTable(dataTable *,int );
void freeNodesData(dataTable *table);



/*Debug*/
void printTableData(dataTable *table);

#endif

