#include "SyntaxTree.h"
#include "parse.h"

Node *tree;
int errorFlag;
int main(int argc, char** argv){
	if(argc <= 1)
			return 1;
	FILE *f = fopen(argv[1], "r");
	if(!f){
		perror(argv[1]);
		return 1;
	}


	tree = NULL;	
	errorFlag = 0;
	yylineno = 1;
	yyrestart(f);
	yyparse();

	traverseTree(tree);
	return 0;

}
