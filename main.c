#include <stdio.h>
#include "fileHandler.h"
#include "parser.h"
#include "secondPass.h"
#include "encodeObject.h"
#include "symbolTable.h"
#include "dataTable.h"

/*
 * authors
 *
 * Yuval cohen
 * Ortal peleg
 *
 * ~~~~~Project description and workflow~~~~
 *
 * ~~~~parse()~~~~ (first pass)
 *
 * the program starts by opening the files that were given as arguments by the commands line.
 * if the files don't exist the program will notify the user.
 * if they are found the and they open successfully than the program sends a pointer of the struct "openFileAnswer" that found in in fileHandler.h to the function parser().
 * the function parse that found in parser.h is our "first pass", the function goes on each line in the file that we opened and send that line as an argument to the function analyzeLine().
 *
 * the function analyzeLine() analyzes each line and understands what it contains(command,instruction,comment, and so on), each line is than analyzed for errors, and all the information we need to know
 * about the line is entered into a struct that is found in definitions.h that called lineInfo. after we collected all the info about our line we return an instance of the struct lineInfo to our parse function.
 *
 * also when we encounter an instruction of type entry we write it to a temp file.
 * later on when we have the addresses of each label we delete the temp file and set each label that were in temp with it's address to .ent file.
 *
 * if there are errors after we finish parse() we print them when we get an answer from analyze line function.(we also print warnings if there are any)
 * when we are done reading the file and all it's lines, if there are errors we stop, and the program ends.

 * if there are no errors we update the data and the symbol tables. the data tables is updated to contain all the addresses of the data section. that is calculated after we know the exacts number of instructions we
 * have in the memory. after that we update the symbol table to contain the updated addresses of our labels.
 *
 * we than return to main() a struct called parserData that contains a pointer to the code,data, and symbol tables, success status, and counter of data and instructions.
 * if our success status is true we than send the data we got in parserData to secondPass().
 *
 * ~~~~secondPass()~~~~
 *
 * in the secondPass() we search for operands with direct or index addressing so we can lookup at the symbol table and change their address to the real address,
 * at this point if we encounter an external label we write to an external file with it's real address.
 *
 * after the secondPass() is done and we changed all the labels , if there are symbol we didn't find the program will notify the user and end.
 * if everything is found than we continue to createObject() function that takes the code table and the data table and translate them into our special base.
 *
 * at this point if everything is correct in the files the program will create 3 files, obj. ent. ext.
 * the program will do all of the above for all the arguments(files) given to it until there are not more arguments.
 *
 *
 * */

int main(int argc, char * argv [] ) {

    int i;

    if (argc <= MINIMUM_NUM_OF_ARGUMENTS)
        printf("No arguments were given");
    else
        for (i = argc - 1 ; i >= MINIMUM_NUM_OF_ARGUMENTS ; i--)
        {
            openFileAnswer *answer = openExistingFile(argv[i]);
            parserData *parserData;

            if (answer->success)
            {
                parserData =parse(answer);
                if (parserData->success)
                {
                    if(secondPass(answer->fileName,*parserData) == false)
                    {
                        printf("\nfiles for %s created successfully\n",answer->fileName);
                        createObject(parserData->codTable, parserData->datTable, answer->fileName, parserData->ic,parserData->dc);

                    }
                }
                /*frees all the tables and other data type that we allocated*/
                if (parserData->symTable)
                {
                    freeNodesSymbol(parserData->symTable);
                    free(parserData->symTable);
                }

                if (parserData->datTable)
                {
                    freeNodesData(parserData->datTable);
                    free(parserData->datTable);
                }

                if (parserData->codTable)
                {
                    freeNodesCode(parserData->codTable);
                    free(parserData->codTable);
                }

                free(parserData);
            }
                free(answer);
        }

    return 0;
}