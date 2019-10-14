#include <stdlib.h>
#include <string.h>
#include "encodeCommands.h"

#define CMD_LOCATION 0

void encodeCommands(memory *,struct lineInfo info);
void encodeOperands(memory *,struct lineInfo info);
void encodeDestOperandOnly(memory *,struct lineInfo info);
void encodeBothOperands(memory *m,struct lineInfo info);

void encode(memory mem[], struct lineInfo info)
{
    encodeCommands(mem,info);

    if(info.operandsCountType == NO_OPERANDS)
        return;
    else
        encodeOperands(mem,info);

}

/*
 * gets an array of memory units that our encoding will take.
 * first we encode the command, than at the cells after cell 0 we will encode our operands if there any.
 *
 * since command is always the first word in our array that we encode we use
 * the cell 0-(CMD_LOCATION)
 *
 *parameter m[] is the array of that we will encode our command and it's operand.
 * it's size is the exact size that our words will take for the certain command that we are encoding.
 *
 * parameter info is all the info about our operands we need in order to encode.
 * */
void encodeCommands(memory m[],struct lineInfo info)
{
    /*data for command encoding*/
    int ARE = 0;
    int commandCode = info.commandNumber;
    int sourceOperand = 0 , destinationOperand = 0;

    switch (info.operandsCountType)
    {
        case TWO_OPERANDS:
            sourceOperand = info.sourceOperandAddress;
            destinationOperand = info.destOperandAddress;
            break;
        case ONE_OPERANDS:
            destinationOperand = info.destOperandAddress;
            break;
        case NO_OPERANDS:
            sourceOperand = 0;
            destinationOperand = 0;
            break;
        default:
            break;
    }

    m[CMD_LOCATION].usage = COMMAND;
    m[CMD_LOCATION].content.cmd.ARE = ARE;
    m[CMD_LOCATION].content.cmd.opCode = commandCode;
    m[CMD_LOCATION].content.cmd.destinationOperand = destinationOperand;
    m[CMD_LOCATION].content.cmd.sourceOperand = sourceOperand;
}

/*encode operands into a word*/
void encodeOperands(memory m[],struct lineInfo info)
{
    switch (info.operandsCountType)
    {
        case ONE_OPERANDS:
            encodeDestOperandOnly(m,info);
            break;
        case TWO_OPERANDS:
            encodeBothOperands(m,info);
            break;
        default:
            return;
    }
}

/*encodes dest operand only*/
void encodeDestOperandOnly(memory m[],struct lineInfo info)
{
    /*only one extra word therefor is in cell number 1*/
    int location = 1;

    switch (info.destOperandAddress)
    {
        case Addressing_immediate:
            m[location].content.wrd.ARE = 0;
            m[location].content.wrd.data = atoi(info.destOperandData);
            m[location].usage = WORD;
            break;
        case Addressing_register:
            m[location].content.reg.ARE = 0;
            m[location].content.reg.destination = atoi(info.destOperandData);
            m[location].usage = REGISTER;
            break;
        case Addressing_direct:
           strcpy(m[location].content.opp.destination ,info.destOperandData);
            m[location].usage = OPPERAND;
            break;
        case Addressing_index:
            strcpy(m[location].content.opp.destination ,info.destOperandData);
            m[location].usage = OPPERAND;
            location++;
            strcpy(m[location].content.opp.destination ,info.destOperandData);
            m[location].usage = OPPERAND;
            break;
        default:
            break;
    }
}
/*encodes both dest operand and source*/
void encodeBothOperands(memory m[],struct lineInfo info)
{
    int location = 1;

    if (info.sourceOperandAddress == Addressing_register && info.destOperandAddress == Addressing_register)
    {
        m[location].content.reg.ARE = 0;
        m[location].content.reg.source = atoi(info.sourceOperandData);
        m[location].content.reg.destination = atoi(info.destOperandData);
        m[location].usage = REGISTER;
        return;
    }

    switch (info.sourceOperandAddress)
    {
        case Addressing_immediate:
            m[location].content.wrd.ARE = 0;
            m[location].content.wrd.data = atoi(info.sourceOperandData);
            m[location].usage = WORD;
            break;
        case Addressing_register:
            m[location].content.reg.ARE = 0;
            m[location].content.reg.source = atoi(info.sourceOperandData);
            m[location].usage = REGISTER;
            break;
        case Addressing_direct:
            strcpy(m[location].content.opp.source ,info.sourceOperandData);
            m[location].usage = OPPERAND;
            break;
        case Addressing_index:
            strcpy(m[location].content.opp.source ,info.sourceOperandData);
            m[location].usage = OPPERAND;
            location++;
            strcpy(m[location].content.opp.source ,info.sourceOperandData);
            m[location].usage = OPPERAND;
            break;
        default:
            break;
    }

    location++;
    switch (info.destOperandAddress)
    {
        case Addressing_immediate:
            m[location].content.wrd.ARE = 0;
            m[location].content.wrd.data = atoi(info.destOperandData);
            m[location].usage = WORD;
            break;
        case Addressing_register:
            m[location].content.reg.ARE = 0;
            m[location].content.reg.destination = atoi(info.destOperandData);
            m[location].usage = REGISTER;
            break;
        case Addressing_direct:
            strcpy(m[location].content.opp.destination ,info.destOperandData);
            m[location].usage = OPPERAND;
            break;
        case Addressing_index:
            strcpy(m[location].content.opp.destination ,info.destOperandData);
            m[location].usage = OPPERAND;
            location++;
            strcpy(m[location].content.opp.destination ,info.destOperandData);
            m[location].usage = OPPERAND;
            break;
        default:
            break;
    }
}


