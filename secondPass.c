#include "secondPass.h"

#define ABSOLUTE 0
#define EXTERNAL 1
#define RELOCATABLE 2

codeNode *changeIndexValue(codeNode *node, symbolTable *sym, operands kind, int counter, openFileAnswer *file);
codeNode *changeDirectValue(codeNode *node, symbolTable *sym, operands kind, int counter, openFileAnswer *file);
char *getIndexLabel(char *string);
int getIndexValue(char *string,symbolTable *sym);

bool writenToExternal = false;
bool errorFlag = false;

/*
 * checking each node in our code table, if it's a command,than check for a Direct or index operand.
 * if so changes their value to be the addresses from the symbol table.
 *
 * parameter -sym is a pointer to our symbol table.
 * parameter -cod is a pointer to our code table.
 * returns errorFlag value, value is set to false if not error found.
 * */
bool secondPass( char *fileName,parserData data)
{
/*gets the file name so we can have the same name with extension ext for our external file*/
    openFileAnswer *createdFile = createExternalFile(fileName);
    codeNode *current;
    symbolTable *sym = data.symTable;
    codeTable *cod = data.codTable;


    int counter = 100;
    addressingType source = 0, destination=0;
    bool isSource = false,isDestination = false;

    writenToExternal = false;
    errorFlag = false;

    current= cod->head;

    while (current != NULL)
    {
        /*checks if the current node is a command, if so checks if it's operands needs an update.*/
        if (current->memoryUnit.usage == COMMAND)
        {
            if(current->memoryUnit.content.cmd.sourceOperand == Addressing_direct ||
                    current->memoryUnit.content.cmd.sourceOperand == Addressing_index)
            {
                isSource = true;
                source = current->memoryUnit.content.cmd.sourceOperand;
            }
            if(current->memoryUnit.content.cmd.destinationOperand == Addressing_direct ||
               current->memoryUnit.content.cmd.destinationOperand == Addressing_index)
            {
                isDestination = true;
                destination = current->memoryUnit.content.cmd.destinationOperand;
            }
        }

        /*changes source or destination operands according to their type.*/
        if (current->memoryUnit.usage == OPPERAND)
        {
            if (isSource)
            {
                isSource = false;
                if (source == Addressing_direct)
                {
                    current = changeDirectValue(current, sym, SOURCE_OPERAND, counter, createdFile);
                    counter++;
                    continue;
                }
                else
                {
                    current = changeIndexValue(current, sym, SOURCE_OPERAND, counter, createdFile);
                    counter++;
                    continue;
                }
            }
            if (isDestination)
            {
                isDestination = false;
                if (destination == Addressing_direct)
                {
                    current = changeDirectValue(current, sym, DEST_OPERAND, counter, createdFile);
                    counter++;
                    continue;
                }
                else
                {
                    current = changeIndexValue(current, sym, DEST_OPERAND, counter, createdFile);
                    counter++;
                    continue;
                }
            }

        }
        current = current->next;
        counter++;
    }

    fclose(createdFile->file);

    /* if nothing was writen to .ext removes it*/
    if (writenToExternal == false)
        remove(createdFile->fileName);

    free(createdFile);
    return errorFlag;
}

/*
 * searches in the symbol table for the address of a label and replaces it's value with the address.
 * parameter -node is the node we need to change the direct value for.
 * parameter -sym is a pointer to our symbol table.
 * parameter -kind is the kind of operand we are changing it's value.
 * returns the location of the next node after our node that we changed.
 * */
codeNode *changeDirectValue(codeNode *node, symbolTable *sym, operands kind, int counter, openFileAnswer *file)
{
    int value,ARE;
    if (kind == SOURCE_OPERAND)
    {
        value = getSymbolValue(sym,node->memoryUnit.content.opp.source);
        if (isSymbolExternal(sym,node->memoryUnit.content.opp.source))
        {
            writeToExternal(counter,file->file,node->memoryUnit.content.opp.source);
            writenToExternal = true;
            ARE = EXTERNAL;
        }
        else
            ARE = RELOCATABLE;
    }
    else
    {
        value = getSymbolValue(sym, node->memoryUnit.content.opp.destination);
        if (isSymbolExternal(sym, node->memoryUnit.content.opp.destination))
        {
            writeToExternal(counter,file->file,node->memoryUnit.content.opp.destination);
            writenToExternal = true;
            ARE = EXTERNAL;
        }
        else
            ARE = RELOCATABLE;
    }


    if (value == NOT_FOUND)
    {
        errorFlag = true;
        if (kind == SOURCE_OPERAND)
        printf("Error: symbol %s not found at second pass\n", node->memoryUnit.content.opp.source);
        else
            printf("Error: symbol %s not found at second pass\n", node->memoryUnit.content.opp.destination);
    }

    node->memoryUnit.usage = WORD;
    node->memoryUnit.content.wrd.ARE = ARE;
    node->memoryUnit.content.wrd.data = value;

    return node->next;
}

/*
 * searches in the symbol table for the address of a index and it's brackets value and replaces it's value with the address.
 * parameter -node is the node we need to change the direct value for.
 * parameter -sym is a pointer to our symbol table.
 * parameter -kind is the kind of operand we are changing it's value.
 * returns the location of the next node after our node that we changed.
 * */
codeNode *changeIndexValue(codeNode *node, symbolTable *sym, operands kind, int counter, openFileAnswer *file)
{
    char *label;
    int firstWord, secondWord,ARE;

    if (kind == SOURCE_OPERAND)
    {
        label = getIndexLabel(node->memoryUnit.content.opp.source);
        secondWord = getIndexValue(node->memoryUnit.content.opp.source,sym);
    }
    else
    {
        label = getIndexLabel(node->memoryUnit.content.opp.destination);
        secondWord = getIndexValue(node->memoryUnit.content.opp.destination,sym);
    }
    firstWord = getSymbolValue(sym, label);

    if(isSymbolExternal(sym,label))
    {
        writenToExternal = true;
        writeToExternal(counter,file->file,label);
        ARE = EXTERNAL;
    }
    else
        ARE = RELOCATABLE;

    if (firstWord == NOT_FOUND)
    {
        errorFlag = true;
        printf("Error: symbol %s not found at second pass\n",label);
    }

    free(label);

    node->memoryUnit.usage = WORD;
    node->memoryUnit.content.wrd.ARE = ARE;
    node->memoryUnit.content.wrd.data = firstWord;
    node = node->next;

    node->memoryUnit.usage = WORD;
    node->memoryUnit.content.wrd.ARE = ABSOLUTE;
    node->memoryUnit.content.wrd.data = secondWord;
    return node;
}

/*
 * gets the label of an operand of type index.
 * parameter -string is the operand we are extracting the label from.
 * returns the string of a label
 * */
char *getIndexLabel(char *string)
{
    char *label;
    int start,end,size;
    start = getFirstGeneralCharacter(string, BEGINNING_OF_LINE);
    end = getCharFirstOccurrence(string,LEFT_BRACKET);

    end = getFirstGeneralCharacterBackwards(string,end);
    size = end - start;
    label = getSubStringFromLocationToSize(string,start,size);

    return label;
}

/*
 * gets the value inside the brackets of an index.
 * parameter -string is the operand we are extracting the brackets value from.
 * returns the value of the brackets.
 * */
int getIndexValue(char *string,symbolTable *sym)
{
    /*brackets value*/
    char *bracketsValue , *tempToFree;
    int start,end,size,result;

    start = getCharFirstOccurrence(string,LEFT_BRACKET)+1;
    end = getCharFirstOccurrence(string,RIGHT_BRACKET);
    size = end - start;

    bracketsValue = getSubStringFromLocationToSize(string,start,size);
    tempToFree = bracketsValue;

    start = getFirstGeneralCharacter(bracketsValue,BEGINNING_OF_LINE);
    end = getFirstGeneralCharacterBackwards(bracketsValue,strlen(bracketsValue));

    if (isNumber(bracketsValue[start]))
    {
        int value =atoi(bracketsValue);
        free(bracketsValue);
        return value;
    }


    size = end - start;
    bracketsValue = getSubStringFromLocationToSize(bracketsValue,start,size);
    free(tempToFree);


    result = getSymbolValue(sym,bracketsValue);
    free(bracketsValue);
    return result;
}
