%{
	#include <stdio.h>
	#include <stdlib.h>
	#include "SyntaxTree.h"
	#include "syntax.tab.h"

	extern int errorLexFlag;
	
	int yycolumn = 1;
	/* define YY_USER_ACTION */
	#define YY_USER_ACTION yylloc.first_line = yylloc.last_line = yylineno; \
		yylloc.first_column = yycolumn; \
		yylloc.last_column = yycolumn + yyleng - 1; \
		yycolumn += yyleng;

%}

%option yylineno

DELIM	[ \t]
WS		{DELIM}+
DIGIT	[0-9]
LETTER	[A-Za-z]
INT		0|([1-9]{DIGIT}*)
OCT		0([0-7]+)
OCT_ERROR	0[0-9A-WYZa-wyz][0-9A-Za-z]*
HEX		0(x|X)[0-9a-fA-F]+
HEX_ERROR	[0][Xx][0-9A-Za-z]*
FLOAT	{INT}(\.{DIGIT}+)?
SFLOAT	((INT\.{DIGIT}*)|({DIGIT}*\.{DIGIT}+))[Ee][+-]?{DIGIT}+
SFLOAT_ERROR	((INT\.{DIGIT}*)|({DIGIT}*\.{DIGIT}+))[Ee][+-]?([0-9A-Za-z]*)
RELOP 	>|<|>=|<=|==|!=
ID		({LETTER}|_)({LETTER}|{DIGIT}|_)*


%%


{WS}	{ /* tabs */ }


"/*"([^\*]|(\*)*[^\*/])*(\*)*"*/"	{  /* C comments */ }

"//"	{ 
			/* C++ comment */ 
			while(input() != '\n');
		}

";"		{ 
			yylval.type_node = init("SEMI",yytext,TYPE_KEYWORD_PUNCTUATION,yylineno);
			return SEMI;
		}

","		{ 
			yylval.type_node = init("COMMA",yytext,TYPE_KEYWORD_PUNCTUATION,yylineno);
			return COMMA;
		}


"="		{ 
			yylval.type_node = init("ASSIGNOP",yytext,TYPE_KEYWORD_OPERATOR,yylineno);
			return ASSIGNOP;
		}

{RELOP}	{ 
			yylval.type_node = init("RELOP",yytext,TYPE_KEYWORD_RELOP,yylineno);
			return RELOP;
		}

"+"		{ 
			yylval.type_node = init("PLUS",yytext,TYPE_KEYWORD_OPERATOR,yylineno);
			return PLUS;
		}

"-"		{ 
			yylval.type_node = init("MINUS",yytext,TYPE_KEYWORD_OPERATOR,yylineno);
			return MINUS;
		}

"*"		{ 
			yylval.type_node = init("STAR",yytext,TYPE_KEYWORD_OPERATOR,yylineno);
			return STAR;
		}

"/"		{ 
			yylval.type_node = init("DIV",yytext,TYPE_KEYWORD_OPERATOR,yylineno);
			return DIV;
		}

"&&"	{ 
			yylval.type_node = init("AND",yytext,TYPE_KEYWORD_OPERATOR,yylineno);
			return AND;
		}

"||"	{ 
			yylval.type_node = init("OR",yytext,TYPE_KEYWORD_OPERATOR,yylineno);
			return OR;
		}

"!"		{ 
			yylval.type_node = init("NOT",yytext,TYPE_KEYWORD_OPERATOR,yylineno);
			return NOT;
		}

"."		{
			yylval.type_node = init("DOT",yytext,TYPE_KEYWORD_PUNCTUATION,yylineno);
			return DOT;
		}

"int"|"float"	{ 
					yylval.type_node = init("TYPE",yytext,TYPE_KEYWORD,yylineno);
					return TYPE;
				}

"("		{ 
			yylval.type_node = init("LP",yytext,TYPE_KEYWORD_BRACKET,yylineno);
			return LP;
		}

")"		{ 
			yylval.type_node = init("RP",yytext,TYPE_KEYWORD_BRACKET,yylineno);
			return RP;
		}

"["		{ 
			yylval.type_node = init("LB",yytext,TYPE_KEYWORD_BRACKET,yylineno);
			return LB;
		}

"]"		{ 
			yylval.type_node = init("RB",yytext,TYPE_KEYWORD_BRACKET,yylineno);
			return RB;
		}

"{"		{ 
			yylval.type_node = init("LC",yytext,TYPE_KEYWORD_BRACKET,yylineno);
			return LC;
		}

"}"		{ 
			yylval.type_node = init("RC",yytext,TYPE_KEYWORD_BRACKET,yylineno);
			return RC;
		}

"struct"	{ 
				yylval.type_node = init("STRUCT",yytext,TYPE_KEYWORD,yylineno);
				return STRUCT;
			}

"return"	{ 
				yylval.type_node = init("RETURN",yytext,TYPE_KEYWORD,yylineno);
				return RETURN;
			}

"if"	{ 
			yylval.type_node = init("IF",yytext,TYPE_KEYWORD,yylineno);
			return IF;
		}

"else"	{ 
			yylval.type_node = init("ELSE",yytext,TYPE_KEYWORD,yylineno);
			return ELSE;
		}

"while"	{ 
			yylval.type_node = init("WHILE",yytext,TYPE_KEYWORD,yylineno);
			return WHILE;
		}

{OCT}	{ 
			yylval.type_node = init("INT",yytext,TYPE_OCT,yylineno);
			return INT;
		}

{OCT_ERROR}	{ 
				printf("Error type A at line %d: Illegal octal number \'%s\'\n", yylineno, yytext); 
				errorLexFlag = 1;
			}

{HEX}	{ 
			yylval.type_node = init("INT",yytext,TYPE_HEX,yylineno);
			return INT;
		}

{HEX_ERROR}	{ 
				printf("Error type A at line %d: Illegal hexadecimal number \'%s\'\n", yylineno, yytext); 
				errorLexFlag = 1;
			}

{SFLOAT}	{
				yylval.type_node = init("FLOAT",yytext,TYPE_SFLOAT,yylineno);
				return FLOAT;	
			}

{SFLOAT_ERROR}	{
				printf("Error type A at line %d: Illegal floating point number \'%s\'\n", yylineno, yytext); 
				errorLexFlag = 1;
			}

{INT}	{ 
			yylval.type_node = init("INT",yytext,TYPE_INT,yylineno);
			return INT;
		}

{FLOAT}	{ 
			yylval.type_node = init("FLOAT",yytext,TYPE_FLOAT,yylineno);
			return FLOAT;
		}

\n		{	yycolumn = 1; }

{ID}	{ 
			yylval.type_node = init("ID",yytext,TYPE_ID,yylineno);
			return ID;
		}


. 		{ 
			printf("Error type A at line %d: Mysterious character \'%s\'\n"
			, yylineno, yytext); 
			errorLexFlag = 1;
		}


%%

/*
int main(int argc, char** argv){
	if (argc > 1)
		if (!(yyin = fopen(argv[1], "r"))){
			perror(argv[1]);
			return 1;
		}
	}
	while (yylex() != 0) ;
	return 0;
}
*/
