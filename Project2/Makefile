parser:
	bison -d syntax.y
	flex lexical.l
	gcc -c lex.yy.c -o lex.o
	gcc main.c syntax.tab.c SyntaxTree.c HashTable.c Semantic.c lex.o -lfl -ly -o parser

lexical:
	bison -d syntax.y
	flex lexical.l

syntax:
	bison -d syntax.y

clean:
	rm lex.yy.c lex.o syntax.tab.c syntax.tab.h parser

remake:
	make clean
	make

test:
	./parser ./pretest/1.txt
	./parser ./pretest/2.txt
	./parser ./pretest/3.txt
	./parser ./pretest/4.txt
	./parser ./pretest/5.txt
	./parser ./pretest/6.txt
	./parser ./pretest/7.txt
	./parser ./pretest/8.txt
	./parser ./pretest/9.txt
	./parser ./pretest/10.txt
	./parser ./pretest/11.txt
	./parser ./pretest/12.txt
	./parser ./pretest/13.txt
	./parser ./pretest/14.txt
	./parser ./pretest/15.txt
	./parser ./pretest/16.txt
	./parser ./pretest/17.txt
	./parser ./pretest/18.txt
	./parser ./pretest/19.txt
	./parser ./pretest/20.txt
	./parser ./pretest/21.txt
	./parser ./pretest/22.txt
	./parser ./pretest/23.txt
