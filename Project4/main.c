#include "SyntaxTree.h"
#include "parse.h"
#include "HashTable.h"
#include "Semantic.h"
#include "InterCode.h"
#include "string.h"

Node *tree;
int errorLexFlag;
int errorSyntaxFlag;

int main(int argc, char** argv){
	if(argc <= 1)
			return 1;
	FILE *f = fopen(argv[1], "r");
	if(!f){
		perror(argv[1]);
		return 1;
	}

	zeroStr = malloc(sizeof(char[2]));
	memset(zeroStr, 0, sizeof(zeroStr));
	strcpy(zeroStr, "0");
	oneStr = malloc(sizeof(char[2]));
	memset(oneStr, 0, sizeof(oneStr));
	strcpy(oneStr, "1");
	neStr = malloc(sizeof(char[3]));
	memset(neStr, 0, sizeof(neStr));
	strcpy(neStr, "!=");

	tree = NULL;
	errorLexFlag = 0;
	errorSyntaxFlag = 0;
	yylineno = 1;
	yyrestart(f);
	yyparse();

	if(errorLexFlag == 0 && errorSyntaxFlag==0){
		initTable();
//		traverseTree(tree);
		Program(tree);
		optimizeGoto();
		deleteLabel();
		figureCon();
		printMips(argv[2]);
	}

	cleanTree(tree);
	return 0;

}
