parser:
	bison -d syntax.y
	flex lexical.l
	gcc -c lex.yy.c -o lex.o
	gcc main.c syntax.tab.c SyntaxTree.c HashTable.c Semantic.c InterCode.c mips.c lex.o -lfl -ly -o parser

lexical:
	bison -d syntax.y
	flex lexical.l

syntax:
	bison -d syntax.y

clean:
	rm lex.yy.c lex.o syntax.tab.c syntax.tab.h parser out1.s out2.s

remake:
	make clean
	make

test1:
	./parser ./testcase/testcase1.txt out1.s

test2:
	./parser ./testcase/testcase2.txt out2.s

test:
	./parser ./testcase/testcase1.txt out1.s
	./parser ./testcase/testcase2.txt out2.s
