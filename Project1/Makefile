parser:
	bison -d syntax.y
	flex lexical.l
	gcc -c lex.yy.c -o lex.o
	gcc main.c syntax.tab.c SyntaxTree.c lex.o -lfl -ly -o parser

lexical:
	flex lexical.l

syntax:
	bison -d synatx.y

scanner:
	gcc main.c lex.yy.c -lfl -o scanner

clean:
	rm lex.yy.c lex.o syntax.tab.c syntax.tab.h parser
