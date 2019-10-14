#include "parser.h"
#include "symbolTable.h"
#include "definitions.h"

struct lineInfo analyzeLine(char *);
void initializeParameter(struct lineInfo *info);

/*parsing functions */
bool isLegalCharacter(char c);
bool isStringContainsLegalCharacter(char *string);
bool isLetter(char c);

bool isFirstCharacter(char *,char);
int getFirstCharacter(char *,char,int);
int getFirstSpace(char *, int);
int getFirstSpaceWithReturnValueNotFound(char *line, int start);

int getCharFirstOccurrenceBackwards(char *,char);
int getCharFirstGeneralOccurrenceBackwards(char *, char);
int getCharFirstOccurrenceBackwardsFromLocation(char *,char ,int);

/*line analysing functions */
bool isWhiteSpaceLine(char *);
bool isComment(char *);
bool isLabel(char *);

/*label functions*/
bool isLabelHasError(char *line, struct lineInfo *info);
bool isLabelNameValid(char *label);

/*Macro functions*/
bool isMacro(char *, struct lineInfo*);
bool isMacroHasError(char *line, struct lineInfo *info);
bool isMacroExists(char *);
bool isMacroValueDecimal(char *, struct lineInfo *);
void extractMacroName(char *line,struct lineInfo* info);

/*Instruction functions*/
bool isInstruction(char * ,struct lineInfo *);
bool isInstructionHasError(char *, struct lineInfo *);
bool isInstructionStringHasError(char *, struct lineInfo *);
bool isInstructionDataHasError(char *, struct lineInfo *);
bool isInstructionEntryOrExternHasError(char *line, struct lineInfo *);

/*command functions*/
bool isCommand(char * line , struct lineInfo *info);
bool isCommandHasError(char * ,struct lineInfo *info);

/*operands functions*/
commandOperandsCount getCommandType(const char *commandName);
bool isNumberOfOperandsFitCommandType(char * , struct lineInfo *);
bool isNoOperands(char *operands, struct lineInfo *info);
bool isOneOperand(char *operands, struct lineInfo *info);
bool isTwoOperands(char *operands, struct lineInfo *info);
void extractOperandOfTypeImmediate(char *operand, operands operandKind, struct lineInfo *info);
void extractOperandOfTypeRegister(char *operand, operands operandKind, struct lineInfo *info);
void extractOperandOfTypeIndex(char *operand, operands operandKind, struct lineInfo *info);
void extractOperandOfTypeDirect(char *operand, operands operandKind, struct lineInfo *info);
void formatOperandAndSetInfoValue(char *operand, operands operandKind, struct lineInfo *info);
addressingType getAddressType(char *operand);
bool isAddressTypeFitCommand(addressingType type ,  int command, operands operandKind,struct lineInfo *info);
bool isOperandValueValid(char *operand,addressingType addressingType1 , operands operandKind, struct lineInfo *info);
bool isDirectAddressingValid(char *operand, struct lineInfo *info);
bool isImmediateAddressingValid(char *operand, struct lineInfo *info);
bool isIndexAddressingValid(char *operand, struct lineInfo *info);
bool isRegisterAddressingValid(char *operand, struct lineInfo *info);

bool isDecimalValueValid(char *string,int,int);
bool isDataHasError(char *token, int start, int end, struct lineInfo *info);

/*String manipulation functions*/
bool isSavedWord(char *);
char * getSubStringFromTo(char *line, int start, int end);
char * removeNewLineCharacterFromString(char *string);
char * removeTaborSpaceCharacterFromEndOfString(char *string);
char * formatStringToNameOnly(char *string);
char * getStringWithoutTab(char * string, bool toFree);
bool lineIsWrongSyntax(char *,struct lineInfo *);
char * trimTab(char * string,bool toFree);

static codeTable *codTable;
static dataTable *datTable;
static symbolTable *symTable;

openFileAnswer *tempEntryFile;
bool entryWriten = false;
int IC = 100,DC = 0;

/*going over the file that was sent as an argument and analyzes each line by calling analyzeLine function.
 * also prints errors if there are any and initializing variables
 * parameter opendFile is a pointer to the struct that contains all the data we need about the opend file, it's name pointer to the actual file and status.
 * return a pointer to parserData struct.
 * */
parserData * parse(openFileAnswer *openedFile)
{
    /*Flag rises when there is an error in the ASM openedFile*/
    bool isFileWithoutError = true;
    /*the current line we are working on , MAX_LINE_LENGTH is 80*/
    char line [MAX_LINE_LENGTH];

    /*the result from analyzing the line will be stored in this variable*/
    struct lineInfo lineResult;
    int lineNumber = 1;

    /*return value for this function*/
    parserData *data =(parserData *) calloc(1, sizeof(parserData));

    /*creates entry file , if nothing writen will be later deleted.*/
    tempEntryFile = createTempEntryFile(openedFile->fileName);

    entryWriten = false;
    IC = 100;
    DC = 0;

    /*creates symbol,data and code tables*/
    symTable = createSymbolTable();
    datTable = createDataTable();
    codTable = createCodeTable();

    while (fgets(line, sizeof(line), openedFile->file) != NULL )
    {
        /*checks if line is the at it's maximum size*/
        if (strlen(line) == MAX_LINE_LENGTH-1)
        {
            /*if so, is the last character is NEW_LINE character, if not, line is exceeding it's max length*/
            if (line[MAX_LINE_LENGTH -2] != NEW_LINE)
            {
                isFileWithoutError = false;
                printf("\nline number :%d is exceeding the maximum length, other errors may be cause by this error.\n",lineNumber);
            }
        }

        lineResult = analyzeLine(line);

        /*prints warnings */
        if(lineResult.warning)
        {
            printf("\nWarning in line:%d\n",lineNumber);
            printf("Warning Message: %s", lineResult.warningMSG);
        }

        /*if there is error at the line we analyzed print it and update flag isFileWithoutError */
        if (lineResult.error)
        {
            isFileWithoutError = false;
            printf("\nError in line:%d\n",lineNumber);
            printf("Error Message: %s", lineResult.errorMsg);
        }
     lineNumber++;
    }

    /*updates data,and symbol tables addresses at the end of pass 1*/
    updateDataTable(datTable,IC);
    updateSymbolTable(symTable,IC);

    fclose(tempEntryFile->file);
    fclose(openedFile->file);

    if (isFileWithoutError)
    {
        data->datTable = datTable;
        data->codTable = codTable;
        data->symTable = symTable;
        data->ic = IC;
        data->dc = DC;
        data->success = true;

        if(entryWriten)
        {
            if(!updateEntry(tempEntryFile,openedFile->fileName, symTable))
            {
                data->success = false;
            }
        }
        else
            remove(tempEntryFile->fileName);
    }
    else
    {
        data->success = false;
        remove(tempEntryFile->fileName);
    }
    if(tempEntryFile)
        free(tempEntryFile);


    return data;
}

/*Initializing parameters for ParseInfo struct that we use in order to pass data about line from one function to another
 * -Parameter is a pointer to the struct.*/
void initializeParameter(struct lineInfo *info) {

    info->error = false;
    info->warning = false;
    info->isLabel = false;
    info->isInstruction = false;
    info->isMacro = false;
    info->isCommand = false;

    strcpy(info->labelName, END_STRING);
    strcpy(info->macroName, END_STRING);
    strcpy(info->instructionLabelName, END_STRING);
    strcpy(info->instructionStringData, END_STRING);
    strcpy(info->errorMsg, END_STRING);
    strcpy(info->warningMSG, END_STRING);

    strcpy(info->sourceOperandData,END_STRING);
    strcpy(info->destOperandData,END_STRING);

    info->pointerLength = 0;

}

/*Analyze each line , and decides what kind of line it is.
 * -parameter -line, is the string of the line that we are Analyzing
 * -returns a struct that contains data about our line*/
struct lineInfo analyzeLine(char * line)
{
    /*lineInfo contains data about our line. like if it's a macro,label, command and so one.
     * we pass it as a pointer to each method that needs to update info about our line.
     * initializeParameter is initializing data for this struct.
     * */
    struct lineInfo parserAns;
    initializeParameter(&parserAns);

    /*line is white space , no need to do anything*/
    if (isWhiteSpaceLine(line))
        return parserAns;

    /*line is comment , no need to do anything*/
    if (isComment(line))
        return parserAns;

    /*line has label, checks the label for errors, and raise the flag isLabel in our parserAns variable*/
    if (isLabel(line))
    {
        parserAns.isLabel = true;
        if(isLabelHasError(line, &parserAns) == true)
            parserAns.error = true;
    }

    /*line is a macro definition*/
    if (isMacro(line,&parserAns))
    {
        parserAns.isMacro = true;
        /*if line also contains label will output an error*/
        if (parserAns.isLabel) {
            parserAns.error = true;
            strcat(parserAns.errorMsg, "Macro and label Can't be in the same line");
        }
        /*if line is a valid macro adds it to the symbol table*/
        if (!isMacroHasError(line, &parserAns))
        {
            if(!addSymbol(symTable,parserAns.macroName,parserAns.macroValue,false,SYMBOL_MACRO))
            {
                strcat(parserAns.errorMsg, "Could not add macro to the symbol table, macro is already exits");
            }
        }
    }

    /*if line is instruction*/
    if (!parserAns.isMacro && isInstruction(line,&parserAns))
    {
        if(isInstructionHasError(line,&parserAns))
        {
            parserAns.error = true;
        }
        switch (parserAns.instructionType1)
        {
            case INSTRUCTION_EXTERN:
                if (!addSymbol(symTable,parserAns.instructionLabelName,UNKNOWN_ADDRESS,true,SYMBOL_EXTERNAL)) {
                    strcat(parserAns.errorMsg, "Could not add extern label to the symbol table, label is already exits");
                }
                break;
            case INSTRUCTION_ENTRY:
                writeToEntry(parserAns.instructionLabelName, tempEntryFile);
                entryWriten = true;
                break;
            default:
                break;
        }

        /*if instruction has string declaration or data adds it to the data table and updates DC*/
        if (parserAns.instructionType1 == INSTRUCTION_STRING || parserAns.instructionType1 ==INSTRUCTION_DATA)
        {
            if (parserAns.isLabel)
                if(!addSymbol(symTable,parserAns.labelName,DC,false,SYMBOL_DATA))
                    parserAns.error = true;
            DC += addData(datTable,DC,parserAns);
        }
    }

    /*if line is command*/
    if ( !parserAns.isInstruction && !parserAns.isMacro && isCommand(line,&parserAns))
    {
        if(isCommandHasError(line,&parserAns))
            parserAns.error = true;

        /*adds our command label to the symbols table*/
        if (parserAns.isLabel)
            if(!addSymbol(symTable,parserAns.labelName,IC,false,SYMBOL_CODE))
                parserAns.error = true;

        IC += getIC(parserAns);

        addCommandsToDataTable(codTable, &parserAns);
    }

    /* line is unrecognized therefor has a syntax error*/
    if (!parserAns.isCommand && !parserAns.isInstruction && !parserAns.isMacro) {
        lineIsWrongSyntax(line, &parserAns);
    }

    return parserAns;
}

/*checks if the first character that that it is encountered and (is not TAB or SPACE), is the character that we sent as parameter.
 * -parameter -line, is the string of the line that we are Analyzing.
 * -parameter -character is the character that we are checking.
 * -return true if the first character is "character" else return false.
 * */
bool isFirstCharacter(char *line, char character)
{
    int i = 0 , length = strlen(line);
    while (i < length)
    {
        if (line[i] != SPACE && line[i] != TAB)
        {
            if(line[i] == character)
                return true;
            else
                return false;
        }
        i++;
    }
    return false;
}

/* gets the location of the first character that encountered that is not SPACE or TAB and equals to parameter "character"
 * -parameter -line, is the string of the line that we are Analyzing
 * -parameter -character is the character that we are checking
 * -parameter -start is the location that we are start checking in the line
 * -return the location of the first encounter*/
int getFirstCharacter(char *line ,char character ,int start)
{
    int i = start , length = strlen(line);

    while (i <= length && line[i] != character && (line[i] == SPACE ||line[i] == TAB ))
        i++;

    return i;
}

/* gets specific character first occurrence or NOT_FOUND value if the character not found.
 * parameter line is a pointer to the string we are checking.
 * parameter character is the character we are checking for.
 * return the location of the character if found, or NOT_FOUND value.
 * */
int getCharFirstOccurrence(char *line,char character)
{
    int i = BEGINNING_OF_LINE, length = strlen(line);

    while (i <= length && line[i] != character)
        i++;

    if (i > length)
        return NOT_FOUND;

    return i;
}

/* gets the location of the first character that encountered backwards(starts from the end of line) that is not SPACE or TAB or NEW_LINE
 * -parameter -line, is the string of the line that we are Analyzing
 * -parameter -character is the character that we are checking
 * -return the location of the first encounter*/
int getCharFirstOccurrenceBackwards(char *line,char character)
{
    int end = strlen(line)-1;

    while (end > 0 && line[end] != character && (line[end] == SPACE ||line[end] == TAB || line[end] == NEW_LINE))
        end--;

    return end;
}

int getCharFirstGeneralOccurrenceBackwards(char *line, char character)
{
    int end = strlen(line)-1;

    while (end > 0 && line[end] != character)
        end--;

    return end;
}

/* gets the location of the first character that encountered backwards(starts from the end of line) that is not SPACE or TAB or NEW_LINE,
 * starts from starts and going backwards
 * -parameter -line, is the string of the line that we are Analyzing.
 * -parameter -character is the character that we are checking.
 * -parameter -start is the location that are are starting from to go backwards.
 * -return the location of the first encounter*/
int getCharFirstOccurrenceBackwardsFromLocation(char *line,char character ,int start)
{
    int end = start;

    while (end > 0 && line[end] != character && (line[end] == SPACE || line[end] == TAB || line[end] == NEW_LINE))
        end--;

    return end;
}

/* gets the location of the first SPACE or TAB that encountered
 * -parameter -line, is the string of the line that we are Analyzing.
 * -parameter -start is the location that are are starting from.
 * -return the location of the first encounter*/
int getFirstSpace(char *line, int start)
{
    while (strlen(line) > start && line[start] != SPACE && line[start] != TAB)
        start++;
    return start;
}

/* gets the location of the first SPACE or TAB that encountered, if no SPACE or TAB returns CHARACTER_NOT_FOUND
 * -parameter -line, is the string of the line that we are Analyzing.
 * -parameter -start is the location that are are starting from.
 * -return the location of the first encounter or CHARACTER_NOT_FOUND*/
int getFirstSpaceWithReturnValueNotFound(char *line, int start)
{
    int i = start;

    while (strlen(line) > i && line[i] !=  SPACE && line[i] !=  TAB)
        i++;

    if (strlen(line) == i)
        return NOT_FOUND;

    return i;
}

/* gets the location of the first character that encountered that is not SPACE or TAB or NEW_LINE
 * -parameter -start is the location that are are starting from.
 * -return the location of the first character encounter  */
int getFirstGeneralCharacter(char *line, int start)
{
    int i = start , length = strlen(line);

    while (i <= length && (line[i] == SPACE ||line[i] == TAB || line[i] == NEW_LINE))
        i++;
    return i;
}

int getFirstGeneralCharacterBackwards(char *line, int start)
{
    while (start > BEGINNING_OF_LINE && (line[start] == SPACE ||line[start] == TAB ||line[start] == NEW_LINE ))
        start--;

    return start;
}

/* checks if line is only SPACE
 * -parameter -line, is the string of the line that we are Analyzing.
 * -return true if line is only SPACE else returns false*/
bool isWhiteSpaceLine(char * line)
{
    return strlen(line) == getFirstGeneralCharacter(line,BEGINNING_OF_LINE);
}

/* checks if line is only comments line
 * -parameter -line, is the string of the line that we are Analyzing.
 * -return true if line is comment else returns false*/
bool isComment(char * line)
{
    return isFirstCharacter(line,COMMENT_SIGN);
}

/* checks if line contains label
 * -parameter -line, is the string of the line that we are Analyzing.
 * -return true if line contains label else returns false*/
bool isLabel(char * line)
{

  int start = BEGINNING_OF_LINE ,end;

  start = getFirstGeneralCharacter(line,start);
  end = getFirstSpace(line,start);

  return line[getCharFirstOccurrenceBackwardsFromLocation(line,COLON,end)] == COLON;
}

/* checks if line that contains the label has error in the label part of the line.
 * -parameter -line, is the string of the line that we are Analyzing.
 * -parameter -info is a pointer to the struct that contains all the info about the current line.
 * -return true if has error else returns false*/
bool isLabelHasError(char *line, struct lineInfo *info)
{
    int start = getFirstGeneralCharacter(line,BEGINNING_OF_LINE);
    int end = getFirstSpace(line,start) - 1;
    bool errorFlag = false;

    int labelLength = end-start;

    /*char labelName [labelLength+1];*/
    char *labelName = calloc(labelLength+1, sizeof(char));
    MEMORY_ALLOC_CHECK(labelName)

    memcpy(labelName,line+start,labelLength);
    labelName[labelLength] = END_STRING_CHAR;

    /*sets the location where this label definition ends*/
    info->labelEndLocation = end+1;

    if (labelLength > MAX_LABEL_LENGTH)
    {
        ERROR_CAT("Label length exceeds the maximum length of 31 characters\n")
        free(labelName);
        errorFlag = true;
    }
    if (isSavedWord(labelName))
    {
        ERROR_CAT("Label is saved word\n")
        free(labelName);
        errorFlag = true;
    }
    if (!isStringContainsLegalCharacter(labelName))
    {
        ERROR_CAT("Label has unpremeditated character(or space),only numbers and letters are allowed\n")
        free(labelName);
        errorFlag = true;
    }
    if (isSymbolNameExists(labelName, symTable))
    {
        ERROR_CAT_VAR("Label name already exists ",labelName,"\n")
        free(labelName);
        errorFlag = true;
    }
    else
        strcpy(info->labelName,labelName);
    free(labelName);
    return errorFlag;
}

/*checks if a certain character is a Number or Letter , any other character is illegal
 * -parameter c is the character that we are checking
 * -return true if legal else false*/
bool isLegalCharacter(char c)
{
    return (c >= 'A' && c<= 'Z') || (c >= 'a' && c<= 'z') || (c >= '0' && c <= '9');
}

/*checks if a  character is a Letter
 * -parameter c is the character that we are checking
 * -return true if is a letter else false*/
bool isLetter(char c)
{
    return (c >= 'A' && c<= 'Z') || (c >= 'a' && c<= 'z') ;
}

/*checks if a character is a number
 * -parameter c is the character that we are checking
 * -return true if is a number else false*/
bool isNumber(char c)
{
    return (c >= '0' && c <= '9');
}

/*checks if a string is only contains numbers and letters, any other character is illegal
 * -parameter string is the string that we are checking
 * -return true if legal else false*/
bool isStringContainsLegalCharacter(char *string)
{
    int length = strlen(string), i;

    for (i = 0 ; i < length ; i++)
    {
        if (!isLegalCharacter(string[i]) && (string[i] != NEW_LINE && string[i] != END_STRING_CHAR))
            return false;
    }
    return true;
}

/*checks if a word is is one of saved words in our assembler
 * -parameter word is the word that we are checking.
 * -return true if is saved word else false. */
bool isSavedWord(char *word)
{
    /*Array of all the saved words*/
    const char *savedWords [27] = {"mov","cmp","add","sub","not","clr","lea","inc","dec","jmp","bne","red","prn","jsr","rts","stop",
                                          "r1","r2","r3","r4","r5","r6","r7",
                                          ".data",".string",".entry",".extern"};

    int i;
    for (i=0 ; i < SAVED_WORDS_COUNT ; i++)
        if (strcmp(word,savedWords[i]) == 0)
            return true;

    return false;
}

char * getSubStringFromLocationToSize(char *line, int location, int till)
{
    char *subString = (char *)calloc(MAX_LINE_LENGTH, sizeof(char));
    MEMORY_ALLOC_CHECK(subString)

    subString[0] = END_STRING_CHAR;

    strncpy(subString , line+location, till);
    return subString;
}

char * getSubStringFromTo(char *line, int start, int end)
{
    char *subString = (char *)calloc(end-start, sizeof(char));
    MEMORY_ALLOC_CHECK(subString)

    strncpy(subString , line+start, end);
    return subString;
}

/* checks if line contains is a Macro definition
 * -parameter -line, is the string of the line that we are Analyzing.
 * -parameter -info is a pointer to the struct that contains all the info about the current line.
 * -return true if line is Macro definition else returns false*/
bool isMacro(char * line, struct lineInfo* info)
{
    const char * macroDeclaration = "define";
    char macroDefinitionString[MACRO_DEFINE_WORD_LENGTH], *subString;
    int i = 0;

    while (i <= strlen(line) && (line[i] != DOT)/* && (line[i] == TAB || line[i] == SPACE)*/)
        i++;

    if (line[i] == DOT)
    {
        i++;
        subString = getSubStringFromLocationToSize(line, i, MACRO_DEFINE_WORD_LENGTH);
        strcpy(macroDefinitionString,subString);

        if (strcmp(macroDefinitionString,macroDeclaration) == 0)
        {
            info->isMacro = true;
            info->macroDefineWordEndLocation = strlen(macroDefinitionString)+i;
            free(subString);
            return true;
        }
        free(subString);
    }
    return false;
}

/* checks if line that contains the  Macro has error such as:
 * *Macro is a saved word  , *Macro value is not decimal , *Macro is already defined.
 *
 * -parameter -line, is the string of the line that we are Analyzing.
 * -parameter -info is a pointer to the struct that contains all the info about the current line.
 * -return true if Macro has error else returns false*/
bool isMacroHasError(char *line, struct lineInfo *info)
{
    bool errorFlag = false;

    extractMacroName(line,info);

    if (isMacroExists(info->macroName))
    {
        errorFlag = true;
        ERROR_CAT_VAR("\n Macro Name already exits: ",info->macroName,"")
    }
    if (isSavedWord(info->macroName))
    {
        errorFlag = true;
        ERROR_CAT_VAR("\n Macro Name is saved word: ",info->macroName,"")
    }

    if(isMacroValueDecimal(line,info) == false)
    {
        errorFlag = true;
        ERROR_CAT_VAR("\n Macro value is not decimal or wrong, macro name: ",info->macroName,"")
    }

    return errorFlag;
}

/*Extract macro name and puts it in info -> macroName*/
void extractMacroName(char *line, struct lineInfo* info)
{
    char name[MAX_LABEL_LENGTH] = END_STRING;
    int end1 , start = info->macroDefineWordEndLocation;

    while (line[start] != SPACE && line[start] != TAB && strlen(line) > start)
        start++;

    if ((line[start] == SPACE || line[start] == TAB)&& strlen(line) > start)
        while ((line[start] == SPACE || line[start] == TAB)&& strlen(line) > start)
            start++;

    end1 = start;

    while (line[end1] != SPACE && line[end1] != TAB && line[end1] != '=' && strlen(line) > end1)
        end1++;

    info->macroNameEndLocation = (start+(end1-start));
    memcpy(name,line+start,end1-start);
    strcpy(info->macroName,name);

}

 /*checks if the macro is already exits in the symbol table.
 * -parameter -name, is the name of the macro.
 * -return true if macro is in symbol table else false.*/
bool isMacroExists(char *name)
{
    return isSymbolNameExists(name,symTable);
}

/*  checks if the macro value is a valid decimal value
 * -parameter -line, is the string of the line that we are Analyzing.
 * -parameter -info is a pointer to the struct that contains all the info about the current line.
 * -return true if the value is valid else false. */
bool isMacroValueDecimal(char *line, struct lineInfo *info)
{
    /*Macro declaration ended, next part of the line is value and name*/
    int equalSign = getCharFirstOccurrence(line,EQUAL_SIGN);

    if (equalSign == CHARACTER_NOT_FOUND)
        return false;

    if (isDecimalValueValid(line,equalSign+1,strlen(line)))
    {
        /*get the value*/
        info->macroValue = atoi(line+equalSign+1);
        return true;
    }
    return false;
}

/* checks if line is a an instruction
 * -parameter -line, is the string of the line that we are Analyzing.
 * -parameter -info is a pointer to the struct that contains all the info about the current line.
 * -return true if line is instruction else returns false*/
bool isInstruction(char * line , struct lineInfo *info)
{
    const char *instructions [4] = {".data",".string",".entry",".extern"};
    char textToCompere[MAX_LABEL_LENGTH] , *subString;
    int i = 0,j,start,end;

    strcpy(textToCompere,END_STRING);

    if (info->isLabel)
    {
        i = info->labelEndLocation;
    }

    i = getFirstCharacter(line,DOT,i) ;
    if (line[i] == DOT) {
        start = i;

        i = getFirstSpace(line, i);

        end = i;
        subString = getSubStringFromLocationToSize(line, start, i - start);
        strcpy(textToCompere, subString);

        for (j = 0; j < INSTRUCTION_COUNT; j++) {
            if (!strcmp(textToCompere, instructions[j])) {
                info->isInstruction = true;
                info->instructionType1 = j;
                info->instructionDefinitionEndLocation = end;
                free(subString);
                return true;
            }
        }
        free(subString);
    }

    return false;
}

bool isInstructionStringHasError(char *line, struct lineInfo *info) {

    char *stringData;
    int i = info->instructionDefinitionEndLocation;
    int closeApostrophe;

    if (strstr(line,APOSTROPHES_SIGN_STR) == NULL) {
        ERROR("instruction of type string has no apostrophes or string value",return true)
    }

    i = getFirstCharacter(line,APOSTROPHES_SIGN,i);

    if (line[i] != APOSTROPHES_SIGN)
    {
        ERROR("instruction of type string missing apostrophes",return true)
    }
    else
    {
        closeApostrophe = getCharFirstOccurrenceBackwards(line,APOSTROPHES_SIGN);

        if (line[closeApostrophe] != APOSTROPHES_SIGN || closeApostrophe == i)
        {
            ERROR("no closing apostrophe to string",return true)
        }

        stringData = getSubStringFromLocationToSize(line,i+1,closeApostrophe-i-1);
        /* getSubStringFromTo(line,i+1,closeApostrophe);*/
        /*stringData = formatStringToNameOnly(stringData);
        stringData = removeApostropheCharacterFromEndOfString(stringData);
        */
        strcpy(info->instructionStringData,stringData);
        free(stringData);
        return false;
    }
}

/* checks if line that contains instruction has error.
 * by calling the relevant function of the instruction type.
 *
 * -parameter -line, is the string of the line that we are Analyzing.
 * -parameter -info is a pointer to the struct that contains all the info about the current line.
 * -return true if has error else returns false*/
bool isInstructionHasError(char * line, struct lineInfo *info)
{
    switch (info->instructionType1) {
        case INSTRUCTION_DATA:
            if (isInstructionDataHasError(line,info))
                return true;
            break;
        case INSTRUCTION_STRING:
            if (isInstructionStringHasError(line, info))
                return true;
            break;
        case INSTRUCTION_ENTRY:
            if (isInstructionEntryOrExternHasError(line, info))
                return true;
            break;
        case INSTRUCTION_EXTERN:
            if (isInstructionEntryOrExternHasError(line, info))
                return true;
            break;
    }
    return false;
}

/* gets tokens of data for instruction .data and analyze each token
 * -parameter -string, the token.
 * -parameter -start is the location that are are starting from.
 * -parameter -end is the location that ends the
 * -return */
bool isDataHasError(char *token, int start , int end, struct lineInfo *info)
{
    char character,*macroName;
    int firstChar = getFirstGeneralCharacter(token, start), index = firstChar;
    character = token[index];

    if (character == PLUS || character == NEGATIVE  || isNumber(character) )
    {
        if (!isDecimalValueValid(token, start, end))
        {
            ERROR("Wrong value in data", return true)
        }
        else
            return false;
    }

    if (isLetter(character)) {
        while (isLegalCharacter(token[index]) && index < end)
            index++;

        if (index == end)
            macroName = getSubStringFromLocationToSize(token, firstChar, end);

        else if (((token[index] == SPACE || token[index] == TAB || token[index] == NEW_LINE) &&
                  isWhiteSpaceLine(token + index)))
        {
            macroName = getSubStringFromLocationToSize(token, firstChar, index - firstChar);
        }

        else{
            ERROR("Macro name is illegal",return true)
        }

        if (!isMacroExists(macroName)) {
            ERROR_CAT("Macro name: ")
            strcat(info->errorMsg, macroName);
            strcat(info->errorMsg, " doesn't exists, or it isn't declared yet");
            free(macroName);
            return true;
        }
        else
        {
            free(macroName);
            return false;
        }
    }
    else
    {
        ERROR("Should have macro name or a number in data values",return true)
    }
}

/*
 *  checks for errors in instruction of kind data.
 *  also if instruction is valid initialize the pointer of info.instructionData, with the data
 *  and sets up info.pointerLength to be the size of instructionData pointer.
 *  the pointer is later used by the function addData() to add the data we just collected to the data table.
 *  addData() will also free it's memory after it's done using it.
 *
 * -parameter -line, is the string of the line that we are Analyzing.

 * -return true if instruction of kind data have error else return false.
 * */
bool isInstructionDataHasError(char *line, struct lineInfo *info) {

    /*the function splits each data by using strtok, than sends each piece of data to data error for a check*/
    int start = info->instructionDefinitionEndLocation,length;
    char *token;/*lineCopy[]*/
    char * lineCopy = (char *)calloc(strlen(line), sizeof(char));
    MEMORY_ALLOC_CHECK(lineCopy)

    strcpy(lineCopy,line);
    token = strtok(lineCopy+start,COMMA_STR);

    length = strlen(token);

    /*only one value , no comma found*/
    if (strchr(line,COMMA) == NULL) {
        if (isDataHasError(lineCopy + start, BEGINNING_OF_LINE, length, info))
        {
            free(lineCopy);
            return true;
        }
        else
        {
            /*initialize the data about what number to add to the data table*/
            info->instructionData = (int *)malloc(sizeof(int));
            MEMORY_ALLOC_CHECK(info->instructionData)

            *info->instructionData =atoi(lineCopy+start);
            free(lineCopy);
            return false;
        }
    }
    /*more than one value, will get each value by using strtok*/
    else
    {
        /*pointer to the data that later will be stored in info->instructionData*/
        int *index = NULL;
        bool once = true;
        int size = 1, counter = 0;
        while (token != NULL)
        {
            /*there is an error with data, return true for error.*/
            if (isDataHasError(token, BEGINNING_OF_LINE, length, info))
            {
                free(lineCopy);
                return true;
            }

             /*first run will initialize our pointer */
             if (once)
             {
                 info->instructionData = (int *)calloc(size, sizeof(int));
                 index =info->instructionData;
                 MEMORY_ALLOC_CHECK(info->instructionData)

                 once = false;
                 *index = atoi(token);
             }
             /* second rune and above will resize the memory block so it will fit any new token*/
             else
             {
                 info->instructionData= (int *) realloc(info->instructionData, sizeof(int) * size);
                 MEMORY_ALLOC_CHECK(info->instructionData)

                 /*points to the new allocation of realloc*/
                 index = info->instructionData;
                 /*advances the pointer to the newly created byte*/
                 index += counter;

                 /*this is a macro extract the value*/
                 if (!isDecimalValueValid(token,BEGINNING_OF_LINE,strlen(token)))
                 {
                     int macroValue;
                     char *macroName;
                     int start = getFirstGeneralCharacter(token,BEGINNING_OF_LINE);
                     int end = getFirstGeneralCharacterBackwards(token,strlen(token));
                     macroName = getSubStringFromLocationToSize(token, start, end);
                     macroName = formatStringToNameOnly(macroName);

                     macroValue = getSymbolValue(symTable, macroName);
                     *index = macroValue;
                     if(macroName)
                         free(macroName);
                 }
                 /*data is a regular string , use atoi to get it's integer value.*/
                 else
                     *index = atoi(token);
             }

             /*gets next token, it's length and update vars to next loop iteration*/
             token = strtok(NULL,COMMA_STR);
             if (token)
                 length = strlen(token);
            info->pointerLength = size;
            size++;
            counter++;
        }
    }

    free(lineCopy);
    return false;
}

/*checks if entry or extern instructions has error*/
bool isInstructionEntryOrExternHasError(char *line, struct lineInfo *info)
{
    char *instructionLabel;
    int i = info->instructionDefinitionEndLocation;

    if (info->isLabel)
    {
        info->warning = true;
        strcpy(info->warningMSG ,"label is redundant for entry or external instruction");
    }

    /*Gets first char that is not space or tab from the location of instruction end defintion*/
    i = getFirstGeneralCharacter(line, i);
    instructionLabel = getSubStringFromLocationToSize(line,i, strlen(line));
    instructionLabel = formatStringToNameOnly(instructionLabel);
    strcpy(info->instructionLabelName,instructionLabel);

    if(!isLabelNameValid(instructionLabel))
    {
        free(instructionLabel);
        ERROR("label in entry or extern is illegal ",return true)
    }

    if (instructionLabel)
        free(instructionLabel);

    return false;
}

/* checks if line is a command
 * -parameter -line, is the string of the line that we are Analyzing.
 * -parameter -info is a pointer to the struct that contains all the info about the current line.
 * -return true if line is command else returns false*/
bool isCommand(char * line , struct lineInfo *info)
{
    const char *commands [16] = {"mov","cmp","add","sub","not","clr","lea","inc","dec","jmp","bne","red","prn","jsr","rts","stop"};
    int start = 0, end,i, commandEnd,length;
    char *command , *toFree;

    /*gets the location that our line starts from, if there is label will start after it*/
    if (info->isLabel)
        start = info->labelEndLocation;

    /*start is command beginning location, end is the end*/
    start = getFirstGeneralCharacter(line,start);
    end = getFirstSpace(line,start);

    /*extracts the command name from line*/
    command = getSubStringFromLocationToSize(line,start,end-start);
    commandEnd = start + (end-start);

    /*if NEW_LINE character is right after the command without space will delete NEW_LINE character from command , good for commands without operands*/
    length = strlen(command) -1;
    if (command[length] == NEW_LINE)
    {
        toFree = command;
        command = getSubStringFromLocationToSize(command,BEGINNING_OF_LINE,length);
        free(toFree);
    }

    /*comperes commands to our substring that has the command*/
    for (i = 0; i < COMMANDS_COUNT ; i++)
    {
        if (strcmp(command,commands[i]) == 0)
        {
            if (command)
                free(command);
            info->isCommand = true;
            info->command = commands[i];
            info->commandNumber = i;
            info->operandsCountType = getCommandType(commands[i]);
            info->commandEndLocation = commandEnd;
            return true;
        }
    }

    if (command)
    {
        free(command);
    }

    return false;
}

/*checks if command contains an error
 * -parameter -line, is the string of the line that we are Analyzing.
 * -parameter -info is a pointer to the struct that contains all the info about the current line.
 * -return true if has error else returns false*/
bool isCommandHasError(char * line,struct lineInfo *info)
{
    char *operands, *copyOperands= NULL,*operandSource,*operandDest;
    int start = info->commandEndLocation, length = strlen(line);
    addressingType operandSourceAddressType,operandDestAddressType;

    /*gets into new string only the operands of the line*/
    operands = getSubStringFromLocationToSize(line,start,length-start);

    if (!isNumberOfOperandsFitCommandType(operands,info))
        return true;

    /*if we have two operands */
    if (info->operandsCountType == TWO_OPERANDS)
    {
        /*initialize dest and source strings with their operands*/
        copyOperands = malloc(strlen(operands));
        MEMORY_ALLOC_CHECK(copyOperands)
        strcpy(copyOperands,operands);
        operandSource = strtok(copyOperands,COMMA_STR);

        /*since we are free memory at the end. we need each operand to be separate*/
        operandDest = malloc(sizeof(strtok(NULL,COMMA_STR)));
        MEMORY_ALLOC_CHECK(operandDest)
        strcpy(operandDest,strtok(NULL,COMMA_STR));

        /*gets the type of address method that is used in each operand*/
        operandSourceAddressType = getAddressType(operandSource);
        operandDestAddressType = getAddressType(operandDest);

        if (operandSourceAddressType == ADDRESSING_WRONG || operandDestAddressType == ADDRESSING_WRONG)
        {
            ERROR_CAT("Operand syntax is wrong")
            return true;
        }

        /*checks that source and dest operands fit the command*/
        if( isAddressTypeFitCommand(operandSourceAddressType,info->commandNumber,SOURCE_OPERAND,info) == false||
            isAddressTypeFitCommand(operandDestAddressType,info->commandNumber,DEST_OPERAND,info) == false)
            return true;
        if (!isOperandValueValid(operandSource,operandSourceAddressType,SOURCE_OPERAND,info))
            return true;
        if (!isOperandValueValid(operandDest,operandDestAddressType,DEST_OPERAND,info))
            return true;

        /*sets the address type in lineinfo for later*/
        info->sourceOperandAddress = operandSourceAddressType;
        info->destOperandAddress = operandDestAddressType;

        formatOperandAndSetInfoValue(operandSource,SOURCE_OPERAND,info);
        formatOperandAndSetInfoValue(operandDest,DEST_OPERAND,info);

        free(operandDest);
    }
    /*doing same as above for one operand*/
    else if (info->operandsCountType == ONE_OPERANDS)
    {
        operandDest = line+start;
        operandDestAddressType = getAddressType(operandDest);

        if (operandDestAddressType == ADDRESSING_WRONG)
        {
            ERROR_CAT("Operand syntax is wrong")
            return true;
        }

        if(isAddressTypeFitCommand(operandDestAddressType,info->commandNumber,DEST_OPERAND,info) == false)
            return true;

        if (!isOperandValueValid(operandDest,operandDestAddressType,DEST_OPERAND,info))
            return true;

        /*sets the address type in lineinfo for later*/
        info->destOperandAddress = operandDestAddressType;
        formatOperandAndSetInfoValue(operandDest,DEST_OPERAND,info);;
    }
    /*No operands*/
    else
    {
        if (!isWhiteSpaceLine(line+start))
        {
            ERROR_CAT_VAR("Command",info->command,"should not have operands")
            return true;
        }
    }

    if (operands) {
        free(operands);
    }
    if (copyOperands) {
        free(copyOperands);
    }
    return false;
}

commandOperandsCount getCommandType(const char *commandName)
{
    const char *towOppCommands [] = {"mov","cmp","add","sub","lea"};
    const char *oneOppCommands [] = {"not","clr","inc","dec","jmp","bne","red","prn","jsr"};
    const char *noOppCommands [] =  {"rts","stop"};

    int i;
    COMMAND_TYPE_CODE(TWO_OPP_CMD_COUNT,towOppCommands,TWO_OPERANDS)
    COMMAND_TYPE_CODE(ONE_OPP_CMD_COUNT,oneOppCommands,ONE_OPERANDS)
    COMMAND_TYPE_CODE(NO_OPP_CMD_COUNT,noOppCommands,NO_OPERANDS)

    return WRONG_NUM_OF_OPERANDS;
}

bool isNumberOfOperandsFitCommandType(char * operands ,struct lineInfo *info)
{
    switch (info->operandsCountType) {
        case NO_OPERANDS:
            if (!isNoOperands(operands, info))
                return false;
            break;
        case ONE_OPERANDS:
            if (!isOneOperand(operands, info))
                return false;
            break;
        case TWO_OPERANDS:
            if (!isTwoOperands(operands, info))
                return false;
            break;
        case WRONG_NUM_OF_OPERANDS:
            return false;
        default:
            return false;
    }
    return true;
}

/*checks if there are no operands in parameter operands
 * parameter operands is the string that we are checking for operands, or no operands
 * return true if no operands, else return false.
 * */
bool isNoOperands(char *operands, struct lineInfo *info)
{
    if (isWhiteSpaceLine(operands)) {
        return true;
    }
    else {

        ERROR_CAT("command ")
        strcat(info->errorMsg,info->command);
        strcat(info->errorMsg," should have no operands");
        return false;
    }
}

/*checks if there is one operand in parameter operands
 * parameter operands is the string that we are checking for operands, or no operands
 * return true if no operands, else return false.
 * */
bool isOneOperand(char *operands,struct lineInfo *info)
{
    if (strchr(operands,COMMA) != NULL)
    {
        ERROR_CAT("command ")
        strcat(info->errorMsg,info->command);
        strcat(info->errorMsg," should have one operand only");
        return false;
    }
    /*new block to for declare variables*/
    {
        int start = getFirstGeneralCharacter(operands, BEGINNING_OF_LINE);
        int end = getFirstSpace(operands, start);

        if (operands[end] == NEW_LINE)
            return true;
        else {
            if (isWhiteSpaceLine(operands + end)) {
                return true;
            } else {
                ERROR_CAT("command ")
                strcat(info->errorMsg, info->command);
                strcat(info->errorMsg, " should have one operand only and should not have space between them");
                return false;
            }
        }
    }
}

/*checks if there are two operands in parameter operands
 * parameter operands is the string that we are checking for operands, or no operands
 * return true if no operands, else return false.
 * */
bool isTwoOperands(char *operands, struct lineInfo *info)
{
    int start = getCharFirstOccurrence(operands,COMMA), length = strlen(operands) -1;

    /*no comma at all*/
    if (start ==  CHARACTER_NOT_FOUND) {
        ERROR_CAT("command ")
        strcat(info->errorMsg, info->command);
        strcat(info->errorMsg, " should have two operands with comma separating them");
        return false;
    }

    /*more than one comma*/
    if (getCharFirstOccurrence(operands+(start+1),COMMA)!= CHARACTER_NOT_FOUND)
    {
        ERROR_CAT("command ")
        strcat(info->errorMsg, info->command);
        strcat(info->errorMsg, " have too many operands with comma separating them");
        return false;
    }

    if (getCharFirstGeneralOccurrenceBackwards(operands, COMMA) == length)
    {
        ERROR_CAT("command ")
        strcat(info->errorMsg, info->command);
        strcat(info->errorMsg, " have comma at the end of line with no value after");
        return false;
    }
    return true;
}

/*checks if decimal that inside a string is valid*/
bool isDecimalValueValid(char *string,int start,int end)
{
    int index = getFirstGeneralCharacter(string,start);

    if (end == index)
    {
        return false;
    }
    if (isWhiteSpaceLine(string))
        return false;

    if (string[index] == NEGATIVE || string[index] == PLUS)
        index++;

    while (index < end)
    {
        if (!(isNumber(string[index])))
            break;
        index++;
    }

    if (string[index] == NEW_LINE)
        return true;

    if (string[index] == TAB || string[index] == SPACE)
    {
        if(!isWhiteSpaceLine(string+index))
            return false;
        else
            return true;
    }

    return  (end == index);
}

/*gets the addressing type of a certain string of operand
 * return the type of addressing the operands uses
 * */
addressingType getAddressType(char *operand)
{
    int start;
    const char *registers [REGISTERS_COUNT] = {"r0","r1","r2","r3","r4","r5","r6","r7"};
    char first,*reg;

    if (isWhiteSpaceLine(operand))
        return ADDRESSING_WRONG;

    start = getFirstGeneralCharacter(operand,BEGINNING_OF_LINE);
    first = operand[start];

    if (first == SHARP_SIGN)
        return Addressing_immediate;

    /*if the first letter of the operand is 'r' check if it's register */
    if (first == 'r')
    {
        int space = getFirstSpaceWithReturnValueNotFound(operand+start,BEGINNING_OF_LINE),i, end;
        if (space == CHARACTER_NOT_FOUND)
            end = strlen(operand) - start;
        else
            end = space;/*-start;*/

        reg = getSubStringFromTo(operand,start,end);
        reg = removeNewLineCharacterFromString(reg);

        for (i = 0 ; i< REGISTERS_COUNT ; i++)
        {
            if(strcmp(registers[i] , reg) == 0)
            {
                free(reg);
                return Addressing_register;
            }
        }
        free(reg);
    }

    if (isLetter(first))
    {
        int brackets = getCharFirstOccurrence(operand,LEFT_BRACKET);
        if (brackets != CHARACTER_NOT_FOUND)
            return Addressing_index;
        else
            return Addressing_direct;
    }

    return ADDRESSING_WRONG;
}

/* checks if string last character is NEW_LINE char, if so removes it and returns new string without it.
 * -parameter -type, enum, types of our addressing  {DIRECT,INDEX,IMMEDIATE,REGISTER}
 * -parameter -command, the command we check for if the addressing type is correct.
 * -parameter -operandKind, enum for operand kind, {SOURCE, DESTINATION}
 * -parameter -info is a pointer to the struct that contains all the info about the current line.
 * -return */
bool isAddressTypeFitCommand(addressingType type , int command, operands operandKind,struct lineInfo *info)
{
    /* 1- true , 0 - false*/
    /*  0   1   2   3*/
    const char *commandsInfoOpp1 [16][4] ={{"1","1","1","1"},/*mov*/
                                                  {"1","1","1","1"},/*cmp*/
                                                  {"1","1","1","1"},/*add*/
                                                  {"1","1","1","1"},/*sub*/
                                                  {"0","0","0","0"},/*not*/
                                                  {"0","0","0","0"},/*clr*/
                                                  {"0","1","1","0"},/*lea*/
                                                  {"0","0","0","0"},/*inc*/
                                                  {"0","0","0","0"},/*dec*/
                                                  {"0","0","0","0"},/*jmp*/
                                                  {"0","0","0","0"},/*bne*/
                                                  {"0","0","0","0"},/*red*/
                                                  {"0","0","0","0"},/*prn*/
                                                  {"0","0","0","0"},/*jsr*/
                                                  {"0","0","0","0"},/*rts*/
                                                  {"0","0","0","0"},/*stop*/
    };

    /*  0   1   2   3*/
    const char *commandsInfoOpp2 [16][4] ={{"0","1","1","1"},/*mov*/
                                                  {"1","1","1","1"},/*cmp*/
                                                  {"0","1","1","1"},/*add*/
                                                  {"0","1","1","1"},/*sub*/
                                                  {"0","1","1","1"},/*not*/
                                                  {"0","1","1","1"},/*clr*/
                                                  {"0","1","1","1"},/*lea*/
                                                  {"0","1","1","1"},/*inc*/
                                                  {"0","1","1","1"},/*dec*/
                                                  {"0","1","0","1"},/*jmp*/
                                                  {"0","1","0","1"},/*bne*/
                                                  {"0","1","1","1"},/*red*/
                                                  {"1","1","1","1"},/*prn*/
                                                  {"0","1","0","3"},/*jsr*/
                                                  {"0","0","0","0"},/*rts*/
                                                  {"0","0","0","0"},/*stop*/
    };


    if (operandKind == SOURCE_OPERAND)
    {
        if (atoi(commandsInfoOpp1[command][type]) == false)
        {
            ERROR_CAT("Source operand is wrong for the command ")
            strcat(info->errorMsg, info->command);
            return false;
        }
    }
    else
        if (atoi(commandsInfoOpp2[command][type]) == false)
        {
            ERROR_CAT("Destination operand is wrong for the command ")
            strcat(info->errorMsg, info->command);
            return false;
        }

    return true;
}

/*checks operands value validity
 * this function will be calling each operand type function.
 * */
bool isOperandValueValid(char *operand,addressingType type , operands operandKind, struct lineInfo *info)
{
    char kind [MAX_OPERAND_WORD_LENGTH ];
    if (operandKind == SOURCE_OPERAND)
        strcpy(kind,"source");
    else
        strcpy(kind,"destination");

    switch (type)
    {
        case Addressing_direct:
            if(!isDirectAddressingValid(operand, info))
                return false;
            break;
        case Addressing_immediate:
            if(!isImmediateAddressingValid(operand, info))
                return false;
            break;
        case Addressing_index:
            if(!isIndexAddressingValid(operand, info))
                return false;
            break;
        case Addressing_register:
            if(!isRegisterAddressingValid(operand, info))
                return false;
            break;
        case ADDRESSING_WRONG:
            return false;
    }
    return true;
}

/*checks validity for Direct operands values.
 * parameter operand is a pointer our the operand that we check it's validity
 * parameter info is the all info and data we need to set and know for the line in which the operand we sent is found.
 * return true if the operand value is valid else return false.
 * */
bool isDirectAddressingValid(char *operand, struct lineInfo *info)
{
    int start = getFirstGeneralCharacter(operand,BEGINNING_OF_LINE);
    int end = getFirstSpaceWithReturnValueNotFound(operand,start);
    int length = strlen(operand);
    char *label;

    if (end == CHARACTER_NOT_FOUND)
        end = length;

    label = getSubStringFromLocationToSize(operand,start,end - start);
    label = removeNewLineCharacterFromString(label);

    if (!isLabelNameValid(label))
    {
        if (label)
            free(label);
        ERROR("Label name in operand is illegal",return false);
    }

    /*if end of operand is not length than check if there are more characters after it, if so, label syntax is wrong*/
    if (end != length)
    {
        if (!isWhiteSpaceLine(operand+end))
        {
            if (label)
                free(label);
            ERROR("Label name in operand is illegal",return false);
        }
    }

    if (label)
        free(label);
    return true;
}

/*checks validity for Immediate operands values.
 * parameter operand is a pointer our the operand that we check it's validity
 * parameter info is the all info and data we need to set and know for the line in which the operand we sent is found.
 * return true if the operand value is valid else return false.
 * */
bool isImmediateAddressingValid(char *operand, struct lineInfo *info)
{
    /*skips # character at the beginning*/
    int start = getFirstGeneralCharacter(operand,BEGINNING_OF_LINE)+1;
    int length = strlen(operand);
    char *subStringMacroName;

    /*if Macro*/
    if (isLetter(operand[start]))
    {
        /*if string is empty*/
        if (length-start == 0)
            return false;

        /*gets macro name and checks if exits*/
        subStringMacroName = getSubStringFromLocationToSize(operand,start,length-start);

        /*if macro name ends with NEW LINE character cuts it out*/
        subStringMacroName = formatStringToNameOnly(subStringMacroName);

        if(isMacroExists(subStringMacroName))
        {
            free(subStringMacroName);
            return true;
        }

        else {
            ERROR_CAT_VAR(" , Macro ", subStringMacroName," isn't declared yet or doesn't exists")
            if (subStringMacroName)
                free(subStringMacroName);
            return false;
        }
    }
    if (isDecimalValueValid(operand,start,strlen(operand)))
        return true;
    else
    {
        ERROR_CAT("Operand value is not a valid decimal value")
        return false;
    }
}

/*checks validity for index operands values.
 * parameter operand is a pointer our the operand that we check it's validity
 * parameter info is the all info and data we need to set and know for the line in which the operand we sent is found.
 * return true if the operand value is valid else return false.
 * */
bool isIndexAddressingValid(char *operand, struct lineInfo *info)
{
    char *label, *bracketsValue;

    int start = getFirstGeneralCharacter(operand,BEGINNING_OF_LINE);
    int leftBracket = getCharFirstOccurrence(operand,LEFT_BRACKET);
    int rightBracket = getCharFirstOccurrence(operand,RIGHT_BRACKET);
    int end = getFirstGeneralCharacterBackwards(operand,leftBracket-1)+1;
    int size = end-start;

    label = getSubStringFromLocationToSize(operand,start,size);
    bracketsValue = getSubStringFromLocationToSize(operand,leftBracket+1,rightBracket-leftBracket-1);

    label = getStringWithoutTab(label,true);
    bracketsValue = getStringWithoutTab(bracketsValue,true);

    if(!isLabelNameValid(label))
    {
        ERROR("Label name in operand is illegal",return false);
    }
    if (!isDecimalValueValid(bracketsValue,BEGINNING_OF_LINE,strlen(bracketsValue)))
    {
        if (!isMacroExists(bracketsValue))
        {
            ERROR("value inside brackets is illegal",return false);
        }
    }

    if (bracketsValue)
        free(bracketsValue);
    if(label)
        free(label);

    return true;

}

/*checks validity for register operands values.
 * parameter operand is a pointer our the operand that we check it's validity
 * parameter info is the all info and data we need to set and know for the line in which the operand we sent is found.
 * return true if the operand value is valid else return false.
 * */
bool isRegisterAddressingValid(char *operand, struct lineInfo *info)
{
    int start = getFirstGeneralCharacter(operand,BEGINNING_OF_LINE);
    int end = getFirstSpaceWithReturnValueNotFound(operand,start);

    if (end == CHARACTER_NOT_FOUND)
        return true;
    else
    {
        if (isWhiteSpaceLine(operand+end))
            return true;
        else
        {
            ERROR("operand is illegal",return false);
        }
    }
}

/* checks if string last character is NEW_LINE char, if so removes it and returns new string without it.
 * -parameter -string, is the string we remove the character from.
 * -return new string or the original string if there is no NEW_LINE character to remove.*/
char * removeNewLineCharacterFromString(char *string)
{
    int length = strlen(string), size = length-1;

    if (string[size] == NEW_LINE)
    {
        char *newString = calloc(size, sizeof(char));
        MEMORY_ALLOC_CHECK(newString)

        memcpy(newString,string,size);
        free(string);
        return newString;
    }

    return string;
}

/* checks if string has TAB or SPACE characters at the end, if so removes it and returns new string without it.
 * -parameter -string, is the string we remove the characters from.
 * -return new string or the original string if there is no TAB or SPACE characters to remove.*/
char * removeTaborSpaceCharacterFromEndOfString(char *string)
{
    int space = getFirstSpace(string,BEGINNING_OF_LINE);
    int length = strlen(string);

    if (space == length-1 && string[space] != TAB)
        return string;
    else
    {
        char *newString = calloc(space-1, sizeof(char));
        MEMORY_ALLOC_CHECK(newString)

        memcpy(newString,string,space);
        free(string);
        return newString;
    }
}

/* removes NEW_LINE character TAB or SPACE at the end of our  string.
 * -parameter -string, is the string we want to format.
 * -return a clean string, or if the original string was clean will return it.*/
char * formatStringToNameOnly(char *string)
{
    string = removeNewLineCharacterFromString(string);
    return removeTaborSpaceCharacterFromEndOfString(string);
}

/* checks if label name is valid
 * -parameter -label, is the name of the label
 * -return true if label is valid else false*/
bool isLabelNameValid(char *label)
{
    int length = strlen(label);

    if (length > MAX_LABEL_LENGTH)
        return false;
    if (isSavedWord(label))
        return false;
    if (!isStringContainsLegalCharacter(label))
        return false;

    return true;
}

/*if we couldn't analyze the line this function will be called and an error message will be displayed*/
bool lineIsWrongSyntax(char *line,struct lineInfo *info)
{
    info->error = true;
    strcat(info->errorMsg ,"line has wrong syntax");

    if (info->isLabel)
    {
        ERROR_CAT(", line has label in it, is it command or instruction? check their syntax")
    }

    return true;
}

/*
 * sets in info the value of the operands that found in current line, for source operand puts the data in sourceOperandData
 * and for destination operand puts the data in destOperandData.
 * */
void formatOperandAndSetInfoValue(char *operand,operands operandKind, struct lineInfo *info)
{
    if (operandKind == SOURCE_OPERAND)
    {
        switch (info->sourceOperandAddress)
        {
            case Addressing_immediate:
                extractOperandOfTypeImmediate(operand, operandKind, info);
                break;
            case Addressing_register:
                extractOperandOfTypeRegister(operand,operandKind,info);
                break;
            case Addressing_index:
                extractOperandOfTypeIndex(operand,operandKind,info);
                break;
            case Addressing_direct:
                extractOperandOfTypeDirect(operand,operandKind,info);
                break;
            default:
                return;
        }
    }
    else
    {
        switch (info->destOperandAddress)
        {
            case Addressing_immediate:
                extractOperandOfTypeImmediate(operand, operandKind, info);
                break;
            case Addressing_register:
                extractOperandOfTypeRegister(operand,operandKind,info);
                break;
            case Addressing_index:
                extractOperandOfTypeIndex(operand,operandKind,info);
                break;
            case Addressing_direct:
                extractOperandOfTypeDirect(operand,operandKind,info);
                break;
            default:
                return;
        }
    }
}

/*extracts the data of operand of type immediate and set it to info source or destination data.*/
void extractOperandOfTypeImmediate(char *operand, operands operandKind, struct lineInfo *info)
{
    char *subString;
    int start = getCharFirstOccurrence(operand, SHARP_SIGN) + 1;
    if (isLetter(operand[start]))
    {
        /*this is a macro , extract value.*/
        int end = getFirstGeneralCharacterBackwards(operand, strlen(operand));
        subString = getSubStringFromLocationToSize(operand, start, end - start);
        subString = formatStringToNameOnly(subString);
        if (operandKind == SOURCE_OPERAND)
            sprintf(info->sourceOperandData, "%d", getSymbolValue(symTable, subString));
        else
            sprintf(info->destOperandData, "%d", getSymbolValue(symTable, subString));
    }
    else
    {
        int end = getFirstSpaceWithReturnValueNotFound(operand, start);
        if (end == CHARACTER_NOT_FOUND)
            subString = getSubStringFromLocationToSize(operand, start, strlen(operand));
        else
            subString = getSubStringFromLocationToSize(operand, start, end - start);

        subString = formatStringToNameOnly(subString);
        if (operandKind == SOURCE_OPERAND)
            strcpy(info->sourceOperandData, subString);
        else
            strcpy(info->destOperandData, subString);
    }
    free(subString);
}

/*extracts the data of operand of type register and set it to info source or destination data.*/
void extractOperandOfTypeRegister(char *operand, operands operandKind, struct lineInfo *info)
{
    const char *registers [REGISTERS_COUNT] = {"r0","r1","r2","r3","r4","r5","r6","r7"};
    char *subString;
    int i,start = getCharFirstOccurrence(operand, R) ;
    subString = getSubStringFromLocationToSize(operand,start, REGISTER_WORD_LENGTH);

    for (i = 0; i < REGISTERS_COUNT ; i++)
    {
        if (strcmp(registers[i],subString) == 0)
        {
            if (operandKind == SOURCE_OPERAND)
                sprintf(info->sourceOperandData, "%d", i);
            else
                sprintf(info->destOperandData, "%d", i);
        }
    }
    free(subString);
}

/*extracts the data of operand of type index and set it to info source or destination data. */
void extractOperandOfTypeIndex(char *operand, operands operandKind, struct lineInfo *info)
{
    char *subString;
    int start = getFirstGeneralCharacter(operand,BEGINNING_OF_LINE);
    int end = getCharFirstOccurrence(operand,RIGHT_BRACKET)+1;
    int size = end-start;

    subString = getSubStringFromLocationToSize(operand,start, size);
    subString = trimTab(subString,true);

    if (operandKind == SOURCE_OPERAND)
        strcpy(info->sourceOperandData ,subString);
    else
        strcpy(info->destOperandData ,subString);

    free(subString);
}

/* extracts the data of operand of type direct and set it to info source or destination data.*/
void extractOperandOfTypeDirect(char *operand, operands operandKind, struct lineInfo *info)
{
    char *subString;
    int start = getFirstGeneralCharacter(operand,BEGINNING_OF_LINE);
    int end = getFirstSpaceWithReturnValueNotFound(operand,start);
    int size;
    if (end == CHARACTER_NOT_FOUND)
        size = strlen(operand) - start;
    else
        size = end-start;

    subString = getSubStringFromLocationToSize(operand,start, size);
    subString = formatStringToNameOnly(subString);

    if (operandKind == SOURCE_OPERAND)
        strcpy(info->sourceOperandData ,subString);
    else
        strcpy(info->destOperandData ,subString);

    free(subString);
}

char * getStringWithoutTab(char * string, bool toFree)
{
    int start = getFirstGeneralCharacter(string,BEGINNING_OF_LINE);
    int end = getFirstSpaceWithReturnValueNotFound(string,start);
    int length = strlen(string);

    if (!isWhiteSpaceLine(string+end))
        return string;

    if (end != NOT_FOUND)
        length = end - start;

    if (toFree)
    {
        char *subString = getSubStringFromLocationToSize(string,start,length);
        free(string);
        return subString;
    }

    return getSubStringFromLocationToSize(string,start,length);
}

/*trims any tab the is found in the string
 * parameter string is the string we would like to trim.
 * parameter toFree if set to true will free the string we got as a parameter.
 * return a new string without tab character.
 * */
char * trimTab(char * string,bool toFree)
{
    char raw [MAX_LINE_LENGTH], *trim;
    int length,i,index = 0;

    if (strchr(string,TAB) == NULL)
        return  string;

    length = strlen(string);

    for (i=0;i<length;i++)
    {
        if (string[i] !=TAB)
        {
            raw[index] = string[i];
            index++;
        }
    }

    raw[index] = END_STRING_CHAR;
    trim = calloc(index, sizeof(char));
    MEMORY_ALLOC_CHECK(trim)

    strcpy(trim, raw);

    if (toFree)
        free(string);

    return trim;
}