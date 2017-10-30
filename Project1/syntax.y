%{
	#include <stdio.h>
%}

/* declared types */
%union {
	int type_int;
	float type_float;
	double type_double;
}

/* declared tokens */
%token <type_int> INT
%token <type_float> FLOAT
%token ADD SUB MUL DIV

%type <type_double> Exp Factor Term

%right ASSIGN
%left ADD SUB
%left MUL DIV
%left LP RP

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE


%%

Stmt	: IF LP Exp RP Stmt %prec LOWER_THAN_ELSE
		| IF LP Exp RP Stmt ELSE Stmt
		;

Calc: /* empty */
	| Exp { printf("= %d\n", $1); }
	;

Exp	: Factor
	| Exp ADD Factor { $$ = $1 + $3; }
	| Exp SUB Factor { $$ = $1 - $3; }
	;

Factor	: Term
		| Factor MUL Term { $$ = $1 * $3; }
		| Factor DIV Term { $$ = $1 / $3; }
		;

Term	: INT
	 	| FLOAT
		;


%%

#include "lex.yy.c"
int main(){
	yyparse();
}
yyerror(char* msg){
	fprintf(stderr, "error: %s\n",msg);
}
