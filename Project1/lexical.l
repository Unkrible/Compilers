%{
	#include <stdio.h>
	#include <stdlib.h>
	#include "SyntaxTree.h"
	#include "syntax.tab.h"
	
	int yycolumn = 1;
	/* define YY_USER_ACTION */
	#define YY_USER_ACTION \
		yylloc.first_line = yylloc.last_line = yylineno; \
		yylloc.first_column = yycolumn; \
		yylloc.last_column = yycolumn + yyleng - 1; \
		yycolumn += yyleng;

%}

%option yylineno

digit [0-9]

letter [_a-zA-Z]


%%


{digit}+	{ 
				printf("Integer value %d\n", atoi(yytext)); 
				return INT;
			} 

"+"	{ return PLUS; }

\n	{ 
		printf("Lines should be ++"); 
		yycolumn = 1;
	}

"//"	{
			char c = input();
			while(c!='\n') c=input();
		}

. 	{ 
		printf("Error type A at Line %d: Mysterious characters \'%s\'\n",
		yylineno,yytext);	
	}


%%

/*
int main(int argc, char** argv){
	if(argc > 1){
		if( !(yyin = fopen(argv[1], "r")) ){
			perror(argv[1]);
			return 1;
		}
	}
	yylex();
	return 0;
}
*/
