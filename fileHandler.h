
#include <stdio.h>
#include "definitions.h"
#include "symbolTable.h"

#ifndef PROJECT_FILEHANDLER_H
#define PROJECT_FILEHANDLER_H

/*Macro*/
#define FALSE_FILE_HANDLER answer->success = false;\
return answer;

typedef struct
{
    char fileName [MAX_FILE_NAME];
    bool success;
    FILE *file;
}openFileAnswer;

/*Opens the file and returns the struct openfileAnswer,
 if the operation was successful than return the pointer to the opened file in the struct field file*/
openFileAnswer * openExistingFile(char *);
openFileAnswer * createTempEntryFile(char *name);
openFileAnswer * createExternalFile(char *name);
openFileAnswer * createObjectFile(char *name);

void writeToEntry(char *label,openFileAnswer *entryFile);
void writeToExternal(int counter,FILE *file,char *label);
bool updateEntry(openFileAnswer *tempEntry, char *fileName, symbolTable *sym);
#endif