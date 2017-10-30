lexical:
	flex lexical.l
parser:
	flex lexical.l
	gcc main.syntax.tab.c -lfl -ly -o parser
scanner:
	gcc main.c lex.yy.c -lfl -o scanner
clean:
	rm lex.yy.c
