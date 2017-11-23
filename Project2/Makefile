parser:
	bison -d syntax.y
	flex lexical.l
	gcc -c lex.yy.c -o lex.o
	gcc main.c syntax.tab.c SyntaxTree.c lex.o -lfl -ly -o parser

lexical:
	bison -d syntax.y
	flex lexical.l

syntax:
	bison -d syntax.y

clean:
	rm lex.yy.c lex.o syntax.tab.c syntax.tab.h parser
