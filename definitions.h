#ifndef PROJECT_DEFINITIONS_H
#define PROJECT_DEFINITIONS_H

/*The minimum arguments that our program gets*/
#define MINIMUM_NUM_OF_ARGUMENTS 1

/*The length of "as" extension */
#define EXTENSION_LENGTH 2

/*Error message string maximum length*/
#define MAX_ERROR_LENGTH 1024

/*label name maximum length*/
#define MAX_LABEL_LENGTH 31

/*the number of saved words there are*/
#define SAVED_WORDS_COUNT 27

/*Maximum line length */
#define MAX_LINE_LENGTH 80

/*the length of the word "DEFINE" */
#define MACRO_DEFINE_WORD_LENGTH 6

#define MAX_INSTRUCTION_WORD_LENGTH 7

/*the number of instructions there are*/
#define INSTRUCTION_COUNT 4

/*number of command possible*/
#define COMMANDS_COUNT 16

/*number or registers that exists*/
#define REGISTERS_COUNT 8

/*start of line*/
#define BEGINNING_OF_LINE 0

/*maximum file name*/
#define MAX_FILE_NAME 100


/*sings that we use in our program*/
#define PLUS '+'
#define NEGATIVE '-'
#define EQUAL_SIGN '='
#define SPACE ' '
#define TAB '\t'
#define NEW_LINE '\n'
#define COMMA ','
#define COMMA_STR ","
#define DOT '.'
#define COMMENT_SIGN ';'
#define COLON ':'
#define R 'r'
#define APOSTROPHES_SIGN_STR "\""
#define APOSTROPHES_SIGN '"'
#define SHARP_SIGN '#'
#define LEFT_BRACKET '['
#define RIGHT_BRACKET ']'

#define END_STRING_CHAR '\0'
#define END_STRING "\0"

#define CHARACTER_NOT_FOUND -1

/* Not found value */
#define NOT_FOUND -1

/*the length of the word "destination" */
#define MAX_OPERAND_WORD_LENGTH 12

/*registers r0-r7 name length*/
#define REGISTER_WORD_LENGTH 2

/*external label address*/
#define UNKNOWN_ADDRESS 0

/*checks if memory allocation succeeded*/
#define MEMORY_ALLOC_CHECK(VAL)\
    if (VAL == NULL )\
    {\
        printf("problem allocating memory");\
        exit(0);\
    }

typedef enum {false,true}bool;

/*type of symbol*/
typedef enum  {SYMBOL_DATA,SYMBOL_CODE,SYMBOL_MACRO,SYMBOL_EXTERNAL} symbolType;

/*type of operand*/
typedef enum {SOURCE_OPERAND,DEST_OPERAND} operands;

/*command number or operands*/
typedef enum {NO_OPERANDS,ONE_OPERANDS,TWO_OPERANDS,WRONG_NUM_OF_OPERANDS} commandOperandsCount;

/* enum that classify addressing type*/
typedef enum {Addressing_immediate,Addressing_direct,Addressing_index,Addressing_register,ADDRESSING_WRONG} addressingType;

/* enum that classify instruction type*/
typedef enum {INSTRUCTION_DATA,INSTRUCTION_STRING,INSTRUCTION_ENTRY,INSTRUCTION_EXTERN}instructionType;

/*
 * contains all the info about our line.
 * each line has it's own struct each run of analyzeLine.
 * a pointer to the struct is passed to let each function modify the info about the current line.
 *
 * macroNameEndLocation - is the location of the end of macro name in the current line, if no macro will be empty.
 * macroDefineWordEndLocation -is the location of the end of ".define" in the current line, if no macro will be empty.
 * instructionDefinitionEndLocation - is the location of the end of .data/.string/.entry/.extern in the current line, if no instruction will be empty.
 * */
struct lineInfo
{
    bool error;
    bool warning;
    bool isLabel;
    int labelEndLocation;

    bool isCommand;
    const char *command;
    int commandNumber;
    /*one operand , two , or no Operands*/
    commandOperandsCount operandsCountType;

    addressingType sourceOperandAddress;
    char sourceOperandData[MAX_LINE_LENGTH];
    char destOperandData[MAX_LINE_LENGTH];

    addressingType destOperandAddress;

    bool isMacro;
    int macroValue;

    bool isInstruction;
    instructionType instructionType1;
    /* for extern or entry instructions only*/
    char instructionLabelName[MAX_LABEL_LENGTH];
    /*for instructions of type String*/
    char instructionStringData[MAX_LINE_LENGTH];
    /*for instructions of type Data*/
    int * instructionData;
    int pointerLength;

    int commandEndLocation;
    int instructionDefinitionEndLocation;
    int macroDefineWordEndLocation;
    int macroNameEndLocation;

    char labelName [MAX_LABEL_LENGTH];
    char macroName [MAX_LABEL_LENGTH];
    char warningMSG [MAX_ERROR_LENGTH];
    char errorMsg [MAX_ERROR_LENGTH];
};

#endif