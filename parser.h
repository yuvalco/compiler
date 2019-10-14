
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "fileHandler.h"
#include "dataTable.h"
#include "codeTable.h"

#ifndef PROJECT_PARSER_H
#define PROJECT_PARSER_H

/*Macro used for errors*/
#define ERROR(MSG,RETURN_VAL)\
info->error = true;\
strcpy(info->errorMsg,MSG);\
RETURN_VAL;

/*Macro used for errors
 * without return and also appending our error MSG*/
#define ERROR_CAT(MSG)\
info->error = true;\
strcat(info->errorMsg,MSG);

#define ERROR_CAT_VAR(MSG,VAL,MSG1)\
info->error = true;\
strcat(info->errorMsg,MSG);\
strcat(info->errorMsg,VAL);\
strcat(info->errorMsg,MSG1);

#define TWO_OPP_CMD_COUNT 5
#define ONE_OPP_CMD_COUNT 8
#define NO_OPP_CMD_COUNT 2

#define COMMAND_TYPE_CODE(COUNT,ARR,RETURN)\
    for (i = 0 ; i < COUNT ; i++)\
    {\
        if (strcmp(commandName,ARR[i]) == 0)\
        {\
            return RETURN;\
        }\
    }\


typedef struct parserData
{
    dataTable *datTable;
    codeTable *codTable;
    symbolTable *symTable;
    bool success;
    int ic;
    int dc;

}parserData;

parserData * parse(openFileAnswer *openedFile);

bool isNumber(char c);
int getFirstGeneralCharacter(char *, int);
int getCharFirstOccurrence(char *,char);
char * getSubStringFromLocationToSize(char *line, int location, int till);
int getFirstGeneralCharacterBackwards(char *line, int start);

#endif
