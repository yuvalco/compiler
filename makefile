project :main.o codeTable.o dataTable.o encodeCommands.o encodeObject.o fileHandler.o parser.o secondPass.o symbolTable.o
	gcc -g main.o codeTable.o dataTable.o encodeCommands.o encodeObject.o fileHandler.o parser.o secondPass.o symbolTable.o -ansi -Wall -pedantic -o project


codeTable.o : codeTable.c
	gcc -c codeTable.c -ansi -Wall -pedantic -o codeTable.o

dataTable.o : dataTable.c
	gcc -c dataTable.c -ansi -Wall -pedantic -o dataTable.o

encodeCommands.o : encodeCommands.c
	gcc -c encodeCommands.c -ansi -Wall -pedantic -o encodeCommands.o

encodeObject.o : encodeObject.c
	gcc -c encodeObject.c -ansi -Wall -pedantic -o encodeObject.o

fileHandler.o : fileHandler.c
	gcc -c fileHandler.c -ansi -Wall -pedantic -o fileHandler.o

parser.o : parser.c
	gcc -c parser.c -ansi -Wall -pedantic -o parser.o

secondPass.o : secondPass.c
	gcc -c secondPass.c -ansi -Wall -pedantic -o secondPass.o

symbolTable.o : symbolTable.c
	gcc -c symbolTable.c -ansi -Wall -pedantic -o symbolTable.o

main.o : main.c
	gcc -c main.c -ansi -Wall -pedantic -o main.o
