main:
	g++ -Wall -std=c++11 interpreter.cc -o interpreter
	bison -d parser.y
	flex scanner.l
	g++ -std=c++11 lex.yy.c parser.tab.c -o compiler
