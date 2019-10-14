#include <stdlib.h>
#include <string.h>
#include "fileHandler.h"
#include "parser.h"

#define TEMP ".temp"
#define ENTRY ".ent"
#define EXTERNAL ".ext"
#define OBJECT ".obj"

/*Validates the file name, checks to see if the extension of the file is "as" and
 * that the file name doesn't exceed the Maximum file name length */
bool validateFileNameExtensionAs(char *);
void extractFileNameAndSetItToStruct(char *,openFileAnswer*);
openFileAnswer * createEntryFile(char *name);

openFileAnswer * openExistingFile(char *fileName)
{
    FILE *file;
    openFileAnswer *answer = (openFileAnswer *)malloc(sizeof(openFileAnswer));
    MEMORY_ALLOC_CHECK(answer)

    if (validateFileNameExtensionAs(fileName) == false)
    {
        FALSE_FILE_HANDLER
    }

    file = fopen(fileName,"r");

    if (file == NULL)
    {
        printf("\nError file name:%s is wrong, or file doesn't exist",fileName);
        FALSE_FILE_HANDLER
    }
    extractFileNameAndSetItToStruct(fileName,answer);

    answer->success= true;
    answer->file = file;
    return answer;
}

bool validateFileNameExtensionAs(char *fileName)
{
    int i = 0;

    if (fileName && strlen(fileName) <= MAX_FILE_NAME) {

        while (i < strlen(fileName) && fileName[i] != DOT)
            i++;

        if (i == strlen(fileName)) {
            printf("\nNo extension to the file, file name: %s", fileName);
            return false;
        }

        if (strlen(fileName) >= i + EXTENSION_LENGTH) {
            if (fileName[i + 1] == 'a' && fileName[i + 2] == 's')
                return true;
            else
                printf("\nError extension name: %s is invalid", fileName);
        }
    }
    else
        printf("\nError File name: %s is invalid", fileName);

    return false;
}

void extractFileNameAndSetItToStruct(char *fileName, openFileAnswer *answer)
{
     int size = getCharFirstOccurrence(fileName,DOT);
     memcpy(answer->fileName,fileName,size);
     answer->fileName[size] = END_STRING_CHAR;
}

/*
 * creates a new file with permission of type "w" (write)
 * parameter filename is the name of the file we would like to create
 * return struct of type openFileAnswer that contains data about the file we created.
 * */
openFileAnswer * createFile(char *filename)
{
    FILE *file = fopen(filename,"w");
    openFileAnswer *answer = (openFileAnswer *)malloc(sizeof(openFileAnswer));
    MEMORY_ALLOC_CHECK(answer)

    if (file == NULL)
    {
        printf("\nError could not create file %s\n", filename);
        FALSE_FILE_HANDLER
    }

    strcpy(answer->fileName ,filename);
    answer->success= true;
    answer->file = file;
    return answer;
}

/*
 * creates temp entry file with extension .temp, the file will later be deleted.
 * parameter name is the name of the file we would like to create
 * return struct of type openFileAnswer that contains data about the file we created.
 * */
openFileAnswer * createTempEntryFile(char *name)
{
    char fileName [MAX_FILE_NAME];
    strcpy(fileName, name);
    strcat(fileName,TEMP);

    return createFile(fileName);
}

/*
 * creates entry file with extension .ent, this entry file is permanent
 * parameter name is the name of the file we would like to create
 * return struct of type openFileAnswer that contains data about the file we created.
 * */
openFileAnswer * createEntryFile(char *name)
{
    char fileName [MAX_FILE_NAME];
    strcpy(fileName, name);
    strcat(fileName,ENTRY);

    return createFile(fileName);
}

/*
 * creates external file with extension .ext
 * parameter name is the name of the file we would like to create
 * return struct of type openFileAnswer that contains data about the file we created.
 * */
openFileAnswer * createExternalFile(char *name)
{
    char fileName [MAX_FILE_NAME];
    strcpy(fileName, name);
    strcat(fileName,EXTERNAL);

    return createFile(fileName);
}

/*
 * creates object file with extension .obj
 * parameter name is the name of the file we would like to create
 * return struct of type openFileAnswer that contains data about the file we created.
 * */
openFileAnswer * createObjectFile(char *name)
{
    char fileName [MAX_FILE_NAME];
    strcpy(fileName, name);
    strcat(fileName,OBJECT);

    return createFile(fileName);
}

/*
 * writes a new label without it's address to a temp entry file.
 * parameter label is the label we would like to write.
 * parameter entryFile is a structs that contains all data we need for the entry file we are writing to.
 * */
void writeToEntry(char *label,openFileAnswer *entryFile)
{
    int length = strlen(label)+1;
    char *stringToWrite = (char *)calloc(length, sizeof(char));
    MEMORY_ALLOC_CHECK(stringToWrite)

    strcpy(stringToWrite,label);
    strcat(stringToWrite,"\n");

    fputs(stringToWrite,entryFile->file);

    free(stringToWrite);
}

/*
 * deletes the old entry file , and creates a new permanent entry file.
 * parameter tempEntry is the struct that contains all the data about our old entry file
 * parameter file name is the name of file we would like to create a permanent entry file for.
 * parameter sym if a pointer to the symbol table that we are taking the labels addresses from.
 * returns true if updated else false if failed.
 * */
bool updateEntry(openFileAnswer *tempEntry, char *fileName, symbolTable *sym)
{
     char line [MAX_LINE_LENGTH];
     int address;
     char update[5];

     openFileAnswer *newEntry = createEntryFile(fileName);
     FILE *oldEntry = fopen(tempEntry->fileName,"r");

     while (fgets(line, sizeof(line), oldEntry) != NULL )
     {
         int size = strlen(line)-1;

         /*char lineToUpdate [strlen(line)+5];*/
         char *lineToUpdate = (char *)calloc(strlen(line)+5, sizeof(char));
         MEMORY_ALLOC_CHECK(lineToUpdate)

         strcpy(lineToUpdate,line);
         lineToUpdate[size] = END_STRING_CHAR;

         address = getSymbolValue(sym,lineToUpdate);

         if (address == NOT_FOUND)
         {
             printf("error, label %s not found" ,lineToUpdate);

             fclose(oldEntry);
             fclose(newEntry->file);
             remove(tempEntry->fileName);
             remove(newEntry->fileName);
             free(newEntry);
             return false;
         }

         if (address < 1000)
             sprintf(update," 0%d\n",address);
         else
             sprintf(update," %d\n",address);

         strcat(lineToUpdate,update);
         fputs(lineToUpdate,newEntry->file);
         free(lineToUpdate);
     }

     fclose(oldEntry);
     remove(tempEntry->fileName);
     fclose(newEntry->file);
     free(newEntry);
    return true;
}

/*
 * writes a label with it's address to .ext file.
 * parameter counter is the address of the label we are writing.
 * parameter file is a pointer to the file we are writing.
 * parameter label is the label we would like to write.
 * */
void writeToExternal(int counter,FILE *file,char *label)
{
    /* +6 to length so we can have space, address with 4 numbers and \n character */
    int length = strlen(label) +6;
    char address [4];
    char *stringToWrite = (char *)calloc(length, sizeof(char));
    MEMORY_ALLOC_CHECK(stringToWrite)

    /*put a zero at start*/
    if (counter < 1000)
        sprintf(address,"0%d",counter);
    else
        sprintf(address,"%d",counter);

    strcpy(stringToWrite,label);

    strcat(stringToWrite," ");
    strcat(stringToWrite,address);
    strcat(stringToWrite,"\n");

    fputs(stringToWrite,file);

    free(stringToWrite);
}

