#include "dataTable.h"
#include "codeTable.h"
#include "fileHandler.h"
#include <stdio.h>
#include <stdlib.h>

#define SIZE_OF_BITS 14

int convertCommandToDecimal(codeNode *node);
void convertToSpecialBase(char *string, int value);
int convertWordToDecimal(codeNode *node);
int convertRegisterToDecimal(codeNode *node);
int twoComplement(unsigned int value);

/*
 * creates object file and converting the code and data tables into a special base that will be written in the file.
 * parameter cod is a pointer to our code table.
 * parameter dat is a pointer to our data table.
 * parameter fileName is the file name that will be creating with extension .obj
 * parameter ic is the instruction counter, this value will be written at the beginning of our object file
 * parameter dc is the data counter, this value will be written at the beginning of our object file
 * */
void createObject(codeTable *cod, dataTable *dat, char *fileName, int ic, int dc)
{
    char line[14];
    int address = 100;

    openFileAnswer *objectFile = createObjectFile(fileName);
    codeNode *currentCode = cod->head;
    dataNode *currentData = dat->head;
    ic -=100;

    sprintf(line,"%d %d\n",ic,dc);
    fputs(line,objectFile->file);

    while (currentCode != NULL)
    {
        if (address < 1000)
            sprintf(line,"0%d ",address);
        else
            sprintf(line,"%d ",address);

        if (currentCode->memoryUnit.usage == COMMAND)
            convertToSpecialBase(line, convertCommandToDecimal(currentCode));
        if (currentCode->memoryUnit.usage == WORD)
            convertToSpecialBase(line,convertWordToDecimal(currentCode));
        if (currentCode->memoryUnit.usage == REGISTER)
            convertToSpecialBase(line,convertRegisterToDecimal(currentCode));

        fputs(line,objectFile->file);

        currentCode = currentCode->next;
        address++;
    }

    while (currentData != NULL)
    {
        if (address < 1000)
            sprintf(line,"0%d ",address);
        else
            sprintf(line,"%d ",address);

        convertToSpecialBase(line,currentData->wrd.data);

        fputs(line,objectFile->file);

        currentData = currentData->next;
        address++;
    }

    fclose(objectFile->file);
    free(objectFile);
}
/*
 * converts decimal number into special base.
 * parameter string is the destination for our special base
 * parameter value is the value that we are converting.
 * */
void convertToSpecialBase(char *string, int value)
{
    int index = 11;
    int leftover;
    string[12] = '\n';

    /*if value negative than use two complement to convert it*/
    if (value < 0)
    {
        value *= -1;
        value = twoComplement(value);
    }

    while (value >= 4 && 4 < index)
    {
        leftover = (value % 4);
        value = value/4;
        switch (leftover) {
            case 0:
                string[index] = '*';
                break;
            case 1:
                string[index] = '#';
                break;
            case 2:
                string[index] = '%';
                break;
            case 3:
                string[index] = '!';
                break;
        }
        index--;
    }

    switch (value) {
        case 0:
            string[index] = '*';
            break;
        case 1:
            string[index] = '#';
            break;
        case 2:
            string[index] = '%';
            break;
        case 3:
            string[index] = '!';
            break;
    }

    index--;
    while (4 < index)
    {
        string[index] = '*';
        index--;
    }
}

/*
 * gets a decimal value and returns it's two complement value
 * parameter value is the value we would like to convert using two complement
 * return two complement value.
 * */
int twoComplement(unsigned int value)
{
    int i;
    bool first = true;
    unsigned int mask = 1;

    for (i=0 ; i< SIZE_OF_BITS; i++)
    {
        if ((value & mask) == mask)
        {
            if (first)
                first = false;
            else
            {
                mask = ~mask;
                value = (value & mask);
                mask = ~mask;
            }
        }
        else
            value = (mask | value);

        mask <<= 1;
    }

    return value;
}

/* converts a command to a decimal value
 * parameter node is a pointer to the node that contains the command.
 * return decimal value of the the command.
 * */
int convertCommandToDecimal(codeNode *node)
{
    unsigned int result = 0;

    if (( node->memoryUnit.content.cmd.ARE & 1) == 1)
        result += 1;
    if (( node->memoryUnit.content.cmd.ARE & 2) == 2)
        result += 2;
    if (( node->memoryUnit.content.cmd.destinationOperand & 1) == 1)
        result += 4;
    if (( node->memoryUnit.content.cmd.destinationOperand & 2) == 2)
        result += 8;
    if (( node->memoryUnit.content.cmd.sourceOperand & 1) == 1)
        result += 16;
    if (( node->memoryUnit.content.cmd.sourceOperand & 2) == 2)
        result += 32;
    if (( node->memoryUnit.content.cmd.opCode & 1) == 1)
        result += 64;
    if (( node->memoryUnit.content.cmd.opCode & 2) == 2)
        result += 128;
    if (( node->memoryUnit.content.cmd.opCode & 4) == 4)
        result += 256;
    if (( node->memoryUnit.content.cmd.opCode & 8) == 8)
        result += 512;

    return result;
}

/* converts a word to a decimal value
 * parameter node is a pointer to the node that contains the word.
 * return decimal value of the the word.
 * */
int convertWordToDecimal(codeNode *node)
{
    unsigned int result = 0;

    if (( node->memoryUnit.content.cmd.ARE & 1) == 1)
        result += 1;
    if (( node->memoryUnit.content.cmd.ARE & 2) == 2)
        result += 2;
    if( (node->memoryUnit.content.wrd.data & 1) == 1)
        result += 4;
    if( (node->memoryUnit.content.wrd.data & 2) == 2)
        result += 8;
    if( (node->memoryUnit.content.wrd.data & 4) == 4)
        result += 16;
    if( (node->memoryUnit.content.wrd.data & 8) == 8)
        result += 32;
    if( (node->memoryUnit.content.wrd.data & 16) == 16)
        result += 64;
    if( (node->memoryUnit.content.wrd.data & 32) == 32)
        result += 128;
    if( (node->memoryUnit.content.wrd.data & 64) == 64)
        result += 256;
    if( (node->memoryUnit.content.wrd.data & 128) == 128)
        result += 512;
    if( (node->memoryUnit.content.wrd.data & 256) == 256)
        result += 1024;
    if( (node->memoryUnit.content.wrd.data & 512) == 512)
        result += 2048;
    if( (node->memoryUnit.content.wrd.data & 1024) == 1024)
        result += 4096;
    if( (node->memoryUnit.content.wrd.data & 2048) == 2048)
        result += 8192;

    return result;
}

/* converts a register to a decimal value
 * parameter node is a pointer to the node that contains the register.
 * return decimal value of the the word.
 * */
int convertRegisterToDecimal(codeNode *node)
{
    unsigned int result = 0;

    if (( node->memoryUnit.content.reg.destination & 1) == 1)
        result += 4;
    if (( node->memoryUnit.content.reg.destination & 2) == 2)
        result += 8;
    if (( node->memoryUnit.content.reg.destination & 4) == 4)
        result += 16;
    if (( node->memoryUnit.content.reg.source & 1) == 1)
        result += 32;
    if (( node->memoryUnit.content.reg.source & 2) == 2)
        result += 64;
    if (( node->memoryUnit.content.reg.source & 4) == 4)
        result += 128;

    return result;

}