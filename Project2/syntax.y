%{
	#include <stdio.h>
	#include <stdlib.h>
	#include "parse.h"
	#include "SyntaxTree.h"
%}

/* declared types */
%union {
	int type_int;
	float type_float;
	double type_double;
	Node* type_node;
}

/* declared tokens */
%token <type_node> INT
%token <type_node> FLOAT
%token <type_node> ID
%token <type_node> SEMI
%token <type_node> COMMA
%token <type_node> ASSIGNOP 
%token <type_node> RELOP
%token <type_node> PLUS 
%token <type_node> MINUS 
%token <type_node> STAR 
%token <type_node> DIV
%token <type_node> AND 
%token <type_node> OR 
%token <type_node> NOT
%token <type_node> DOT
%token <type_node> TYPE
%token <type_node> LP 
%token <type_node> RP 
%token <type_node> LB 
%token <type_node> RB 
%token <type_node> LC 
%token <type_node> RC
%token <type_node> STRUCT
%token <type_node> RETURN
%token <type_node> IF 
%token <type_node> ELSE
%token <type_node> WHILE

/* declared non-terminals */
%type <type_node> Program ExtDefList ExtDef ExtDecList
%type <type_node> Specifier StructSpecifier OptTag Tag
%type <type_node> VarDec FunDec VarList ParamDec
%type <type_node> CompSt StmtList Stmt
%type <type_node> DefList Def DecList Dec
%type <type_node> Exp Args

%right ASSIGNOP
%left OR
%left AND
%left RELOP 
%left PLUS 
%left STAR DIV
%right NOT
%left LP RP LB RB DOT

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE


%%

/* High-level Definitions */
Program	: ExtDefList { 
			$$ = init("Program", NULL, TYPE_NONTERMINAL, @$.first_line);
			insert($$, $1);
			tree = $$;
		}
		;
ExtDefList	: ExtDef ExtDefList{
		   		$$ = init("ExtDefList", NULL, TYPE_NONTERMINAL, @$.first_line);
				insert($$,$1);insert($$,$2);
			}
		   	| /* empty */{ $$ = init("ExtDefList", NULL, TYPE_NONTERMINAL, @$.first_line); }
			;
ExtDef	: Specifier ExtDecList SEMI{
	   		$$ = init("ExtDef", NULL, TYPE_NONTERMINAL, @$.first_line);
			insert($$,$1);insert($$,$2);insert($$,$3);
		}
		| Specifier SEMI{
	   		$$ = init("ExtDef", NULL, TYPE_NONTERMINAL, @$.first_line);
			insert($$,$1);insert($$,$2);
		}
		| Specifier FunDec SEMI{
			$$ = init("ExtDef", NULL, TYPE_NONTERMINAL, @$.first_line);
			insert($$,$1);insert($$,$2);insert($$,$3);
		}
		| Specifier FunDec CompSt{
	   		$$ = init("ExtDef", NULL, TYPE_NONTERMINAL, @$.first_line);
			insert($$,$1);insert($$,$2);insert($$,$3);
		}
		| error SEMI { errorSyntaxFlag=2; }
		;
ExtDecList	: VarDec{
		   		$$ = init("ExtDecList", NULL, TYPE_NONTERMINAL, @$.first_line);
				insert($$, $1);
			}
		    | VarDec COMMA ExtDecList{
	   			$$ = init("ExtDecList", NULL, TYPE_NONTERMINAL, @$.first_line);
				insert($$,$1);insert($$,$2);insert($$,$3);
			}
			;

/* Specifiers */
Specifier	: TYPE{
		  		$$ = init("Specifier", NULL, TYPE_NONTERMINAL, @$.first_line);
				insert($$,$1);
			}
		  	| StructSpecifier{
				$$ = init("Specifier", NULL, TYPE_NONTERMINAL, @$.first_line);
				insert($$,$1);
			}
			;
StructSpecifier	: STRUCT OptTag LC DefList RC{
					$$ = init("StructSpecifier", NULL, TYPE_NONTERMINAL, @$.first_line);
					insert($$,$1);insert($$,$2);insert($$,$3);insert($$,$4);insert($$,$5);
				}
				| STRUCT Tag{
					$$ = init("StructSpecifier", NULL, TYPE_NONTERMINAL, @$.first_line);
					insert($$,$1);insert($$,$2);
				}
				;
OptTag	: ID{
			$$ = init("OptTag", NULL, TYPE_NONTERMINAL, @$.first_line);
			insert($$,$1);
		}
	    | /* empty */{$$ = init("OptTag", NULL, TYPE_NONTERMINAL, @$.first_line);}
		;
Tag	: ID{
		$$ = init("Tag", NULL, TYPE_NONTERMINAL, @$.first_line);
		insert($$,$1);
	}
	;

/* Declarators */
VarDec : ID{
			$$ = init("VarDec", NULL, TYPE_NONTERMINAL, @$.first_line);
			insert($$,$1);
		}
	   | VarDec LB INT RB{
			$$ = init("VarDec", NULL, TYPE_NONTERMINAL, @$.first_line);
			insert($$,$1);insert($$,$2);insert($$,$3);insert($$,$4);
		}
	   ;
FunDec	: ID LP VarList RP{
			$$ = init("FunDec", NULL, TYPE_NONTERMINAL, @$.first_line);
			insert($$,$1);insert($$,$2);insert($$,$3);insert($$,$4);
		}
	   	| ID LP RP{
			$$ = init("FunDec", NULL, TYPE_NONTERMINAL, @$.first_line);
			insert($$,$1);insert($$,$2);insert($$,$3);
		}
		| error RP{ errorSyntaxFlag=2; }
		;
VarList	: ParamDec COMMA VarList{
			$$ = init("VarList", NULL, TYPE_NONTERMINAL, @$.first_line);
			insert($$,$1);insert($$,$2);insert($$,$3);
		}
		| ParamDec{
			$$ = init("VarList", NULL, TYPE_NONTERMINAL, @$.first_line);
			insert($$,$1);
		}
		;
ParamDec	: Specifier VarDec{
				$$ = init("ParamDec", NULL, TYPE_NONTERMINAL, @$.first_line);
				insert($$,$1);insert($$,$2);
			}
		 	;

/* Statements */
CompSt	: LC DefList StmtList RC{
			$$ = init("CompSt", NULL, TYPE_NONTERMINAL, @$.first_line);
			insert($$,$1);insert($$,$2);insert($$,$3);insert($$,$4);
		}
		| error RC{ errorSyntaxFlag=2; }
	   	;
StmtList	: Stmt StmtList{
				$$ = init("StmtList", NULL, TYPE_NONTERMINAL, @$.first_line);
				insert($$,$1);insert($$,$2);
			}
		 	| /* empty */{ 
				$$ = init("StmtList", NULL, TYPE_NONTERMINAL, @$.first_line);
			}
			;
Stmt	: Exp SEMI{ 
			$$ = init("Stmt", NULL, TYPE_NONTERMINAL, @$.first_line);
			insert($$,$1);insert($$,$2);
		}
	 	| CompSt{
			$$ = init("Stmt", NULL, TYPE_NONTERMINAL, @$.first_line);
			insert($$,$1);
		}
		| RETURN Exp SEMI{
			$$ = init("Stmt", NULL, TYPE_NONTERMINAL, @$.first_line);
			insert($$,$1);insert($$,$2);insert($$,$3);
		}
		| IF LP Exp RP Stmt %prec LOWER_THAN_ELSE{
			$$ = init("Stmt", NULL, TYPE_NONTERMINAL, @$.first_line);
			insert($$,$1);insert($$,$2);insert($$,$3);
			insert($$,$4);insert($$,$5);
		}
		| IF LP Exp RP Stmt ELSE Stmt{
			$$ = init("Stmt", NULL, TYPE_NONTERMINAL, @$.first_line);
			insert($$,$1);insert($$,$2);insert($$,$3);
			insert($$,$4);insert($$,$5);insert($$,$6);insert($$,$7);
		}
		| WHILE LP Exp RP Stmt{
			$$ = init("Stmt", NULL, TYPE_NONTERMINAL, @$.first_line);
			insert($$,$1);insert($$,$2);insert($$,$3);
			insert($$,$4);insert($$,$5);
		}
		| error SEMI { errorSyntaxFlag=2; }
		;

/* Local Definitions */
DefList	: Def DefList{
			$$ = init("DefList", NULL, TYPE_NONTERMINAL, @$.first_line);
			insert($$,$1);insert($$,$2);
		}
		| /* empty */{ 
			$$ = init("DefList", NULL, TYPE_NONTERMINAL, @$.first_line);
		}
		;
Def	: Specifier DecList SEMI{
		$$ = init("Def", NULL, TYPE_NONTERMINAL, @$.first_line);
		insert($$,$1);insert($$,$2);insert($$,$3);
	}
	| error SEMI{ errorSyntaxFlag=2; }
	;
DecList	: Dec{
			$$ = init("DecList", NULL, TYPE_NONTERMINAL, @$.first_line);
			insert($$,$1);
		}
		| Dec COMMA DecList{
			$$ = init("DecList", NULL, TYPE_NONTERMINAL, @$.first_line);
			insert($$,$1);insert($$,$2);insert($$,$3);
		}
		;
Dec	: VarDec{
		$$ = init("Dec", NULL, TYPE_NONTERMINAL, @$.first_line);
		insert($$,$1);
	}
	| VarDec ASSIGNOP Exp{
		$$ = init("Dec", NULL, TYPE_NONTERMINAL, @$.first_line);
		insert($$,$1);insert($$,$2);insert($$,$3);
	}

/* Expressions */
Exp	: Exp ASSIGNOP Exp{
		$$ = init("Exp", NULL, TYPE_NONTERMINAL, @$.first_line);
		insert($$,$1);insert($$,$2);insert($$,$3);
	}
	| Exp AND Exp{
		$$ = init("Exp", NULL, TYPE_NONTERMINAL, @$.first_line);
		insert($$,$1);
	}
	| Exp OR Exp{
		$$ = init("Exp", NULL, TYPE_NONTERMINAL, @$.first_line);
		insert($$,$1);insert($$,$2);insert($$,$3);
	}
	| Exp RELOP Exp{
		$$ = init("Exp", NULL, TYPE_NONTERMINAL, @$.first_line);
		insert($$,$1);insert($$,$2);insert($$,$3);
	}
	| Exp PLUS Exp{
		$$ = init("Exp", NULL, TYPE_NONTERMINAL, @$.first_line);
		insert($$,$1);insert($$,$2);insert($$,$3);
	}
	| Exp MINUS Exp{
		$$ = init("Exp", NULL, TYPE_NONTERMINAL, @$.first_line);
		insert($$,$1);insert($$,$2);insert($$,$3);
	}
	| Exp STAR Exp{
		$$ = init("Exp", NULL, TYPE_NONTERMINAL, @$.first_line);
		insert($$,$1);insert($$,$2);insert($$,$3);
	}
	| Exp DIV Exp{
		$$ = init("Exp", NULL, TYPE_NONTERMINAL, @$.first_line);
		insert($$,$1);insert($$,$2);insert($$,$3);
	}
	| LP Exp RP{
		$$ = init("Exp", NULL, TYPE_NONTERMINAL, @$.first_line);
		insert($$,$1);insert($$,$2);insert($$,$3);
	}
	| MINUS Exp{
		$$ = init("Exp", NULL, TYPE_NONTERMINAL, @$.first_line);
		insert($$,$1);insert($$,$2);
	}
	| NOT Exp{
		$$ = init("Exp", NULL, TYPE_NONTERMINAL, @$.first_line);
		insert($$,$1);insert($$,$2);
	}
	| ID LP Args RP{
		$$ = init("Exp", NULL, TYPE_NONTERMINAL, @$.first_line);
		insert($$,$1);insert($$,$2);insert($$,$3);insert($$,$4);
	}
	| ID LP RP{
		$$ = init("Exp", NULL, TYPE_NONTERMINAL, @$.first_line);
		insert($$,$1);insert($$,$2);insert($$,$3);
	}
	| Exp LB Exp RB{
		$$ = init("Exp", NULL, TYPE_NONTERMINAL, @$.first_line);
		insert($$,$1);insert($$,$2);insert($$,$3);insert($$,$4);
	}
	| Exp DOT ID{
		$$ = init("Exp", NULL, TYPE_NONTERMINAL, @$.first_line);
		insert($$,$1);insert($$,$2);insert($$,$3);
	}
	| ID{
		$$ = init("Exp", NULL, TYPE_NONTERMINAL, @$.first_line);
		insert($$,$1);
	}
	| INT{
		$$ = init("Exp", NULL, TYPE_NONTERMINAL, @$.first_line);
		insert($$,$1);
	}
	| FLOAT{
		$$ = init("Exp", NULL, TYPE_NONTERMINAL, @$.first_line);
		insert($$,$1);
	}
	;
Args	: Exp COMMA Args{
			$$ = init("Args", NULL, TYPE_NONTERMINAL, @$.first_line);
			insert($$,$1);insert($$,$2);insert($$,$3);
		}
	 	| Exp{
			$$ = init("Args", NULL, TYPE_NONTERMINAL, @$.first_line);
			insert($$,$1);
		}
		;

/* Comments */

%%


void yyerror(char* msg){
	if(errorLexFlag==0)
		fprintf(stderr, "Error type B at Line %d:  %s\n",yylineno,msg);
}
