# compiler
Special base assembly compiler written in C


As part of university course we were asked to build a compiler.  
The compiler translate a made up lenguage that is very simaler to assembly into special base machine code.

The program/compiler works as follow:
## First pass
the program starts by opening the files that were given as arguments by the commands line.
if the files don't exist the program will notify the user.
if they are found the and they open successfully than the program sends a pointer of the struct "openFileAnswer" that found in in fileHandler.h to the function parser().
the function parse that found in parser.h is our "first pass", the function goes on each line in the file that we opened and send that line as an argument to the function analyzeLine().

 the function analyzeLine() analyzes each line and understands what it contains(command,instruction,comment, and so on), each line is than analyzed for errors, and all the information we need to know
 about the line is entered into a struct that is found in definitions.h that called lineInfo. after we collected all the info about our line we return an instance of the struct lineInfo to our parse function.
 
 also when we encounter an instruction of type entry we write it to a temp file.
 later on when we have the addresses of each label we delete the temp file and set each label that were in temp with it's address to .ent file.

 if there are errors after we finish parse() we print them when we get an answer from analyze line function.(we also print warnings if there are any)
 when we are done reading the file and all it's lines, if there are errors we stop, and the program ends.
 if there are no errors we update the data and the symbol tables. the data tables is updated to contain all the addresses of the data section. that is calculated after we know the exacts number of instructions we
 have in the memory. after that we update the symbol table to contain the updated addresses of our labels.
 
 we than return to main() a struct called parserData that contains a pointer to the code,data, and symbol tables, success status, and counter of data and instructions.
 if our success status is true we than send the data we got in parserData to secondPass().
 
 ## Second Pass
 
 in the secondPass() we search for operands with direct or index addressing so we can lookup at the symbol table and change their address to the real address,
 at this point if we encounter an external label we write to an external file with it's real address.

after the secondPass() is done and we changed all the labels , if there are symbol we didn't find the program will notify the user and end.
if everything is found than we continue to createObject() function that takes the code table and the data table and translate them into our special base.

at this point if everything is correct in the files the program will create 3 files, obj. ent. ext.
the program will do all of the above for all the arguments(files) given to it until there are not more arguments.

## Special base machine code example:
```
25 11
0100 ****!%*  
0101 ***#%**  
0102 **%*#*%  
0103 *****%*  
0104 **%#*#*  
0105 ******#  
0106 **!****  
0107 !!!!%!*  
0108 ****%%*  
0109 **#!!#%  
0110 ****##*  
0111 **#!!#%  
0112 *****%*  
0113 ***!!!*  
0114 ****!**  
0115 ***##**  
0116 **%*#!%  
0117 *****%*  
0118 **%%*#*  
0119 ******#  
0120 **#!*#*  
0121 ******#  
0122 **%%*#*  
0123 **#%%*%  
0124 **!!***  
0125 ***#%*#  
0126 ***#%*%  
0127 ***#%*!  
0128 ***#%#*  
0129 ***#%##  
0130 ***#%#%  
0131 *******  
0132 *****#%  
0133 !!!!!#!  
0134 *****#*  
0135 ****##%  
```
