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
%left PLUS MINUS 
%left STAR DIV
%right NOT
%left LP RP LB RB DOT

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE


%%

/* High-level Definitions */
Program	: ExtDefList { 
			$$ = init("Program", "", TYPE_KEYWORD, @$.first_line);
			insert($$, $1);
			tree = $$;
		}
		;
ExtDefList	: ExtDef ExtDefList{
		   		$$ = init("ExtDefList", "", TYPE_KEYWORD, @$.first_line);
				insert($$,$1);insert($$,$2);
			}
		   	| /* empty */{ $$=NULL; }
			;
ExtDef	: Specifier ExtDecList SEMI{
	   		$$ = init("ExtDef", "", TYPE_KEYWORD, @$.first_line);
			insert($$,$1);insert($$,$2);insert($$,$3);
		}
		| Specifier SEMI{
	   		$$ = init("ExtDef", "", TYPE_KEYWORD, @$.first_line);
			insert($$,$1);insert($$,$2);
		}
		| Specifier FunDec CompSt{
	   		$$ = init("ExtDef", "", TYPE_KEYWORD, @$.first_line);
			insert($$,$1);insert($$,$2);insert($$,$3);
		}
		;
ExtDecList	: VarDec{
		   		$$ = init("ExtDecList", "", TYPE_KEYWORD, @$.first_line);
				insert($$, $1);
			}
		    | VarDec COMMA ExtDecList{
	   			$$ = init("ExtDecList", "", TYPE_KEYWORD, @$.first_line);
				insert($$,$1);insert($$,$2);insert($$,$3);
			}
			;

/* Specifiers */
Specifier	: TYPE{
		  		$$ = init("Specifier", "", TYPE_KEYWORD, @$.first_line);
				insert($$,$1);
			}
		  	| StructSpecifier{
				$$ = init("StructSpecifier", "", TYPE_KEYWORD, @$.first_line);
				insert($$,$1);
			}
			;
StructSpecifier	: STRUCT OptTag LC DefList RC{
					$$ = init("StructSpecifier", "", TYPE_KEYWORD, @$.first_line);
					insert($$,$1);insert($$,$2);insert($$,$3);insert($$,$4);insert($$,$5);
				}
				| STRUCT Tag{
					$$ = init("StructSpecifier", "", TYPE_KEYWORD, @$.first_line);
					insert($$,$1);insert($$,$2);
				}
				;
OptTag	: ID{
			$$ = init("OptTag", "", TYPE_KEYWORD, @$.first_line);
			insert($$,$1);
		}
	    | /* empty */{ $$=NULL; }
		;
Tag	: ID{
		$$ = init("Tag", "", TYPE_KEYWORD, @$.first_line);
		insert($$,$1);
	}
	;

/* Declarators */
VarDec : ID{
			$$ = init("VarDec", "", TYPE_KEYWORD, @$.first_line);
			insert($$,$1);
		}
	   | VarDec LB INT RB{
			$$ = init("VarDec", "", TYPE_KEYWORD, @$.first_line);
			insert($$,$1);insert($$,$2);insert($$,$3);insert($$,$4);
		}
	   ;
FunDec	: ID LP VarList RP{
			$$ = init("FunDec", "", TYPE_KEYWORD, @$.first_line);
			insert($$,$1);insert($$,$2);insert($$,$3);insert($$,$4);
		}
	   	| ID LP RP{
			$$ = init("FunDec", "", TYPE_KEYWORD, @$.first_line);
			insert($$,$1);insert($$,$2);insert($$,$3);
		}
		;
VarList	: ParamDec COMMA VarList{
			$$ = init("VarList", "", TYPE_KEYWORD, @$.first_line);
			insert($$,$1);insert($$,$2);insert($$,$3);
		}
		| ParamDec{
			$$ = init("VarList", "", TYPE_KEYWORD, @$.first_line);
			insert($$,$1);
		}
		;
ParamDec	: Specifier VarDec{
				$$ = init("ParamDec", "", TYPE_KEYWORD, @$.first_line);
				insert($$,$1);insert($$,$2);
			}
		 	;

/* Statements */
CompSt	: LC DefList StmtList RC{
			$$ = init("CompSt", "", TYPE_KEYWORD, @$.first_line);
			insert($$,$1);insert($$,$2);insert($$,$3);insert($$,$4);
		}
	   	;
StmtList	: Stmt StmtList{
				$$ = init("StmtList", "", TYPE_KEYWORD, @$.first_line);
				insert($$,$1);insert($$,$2);
			}
		 	| /* empty */{ $$ = NULL; }
			;
Stmt	: Exp SEMI{ 
			$$ = init("Stmt", "", TYPE_KEYWORD, @$.first_line);
			insert($$,$1);insert($$,$2);
		}
	 	| CompSt{
			$$ = init("Stmt", "", TYPE_KEYWORD, @$.first_line);
			insert($$,$1);
		}
		| RETURN Exp SEMI{
			$$ = init("Stmt", "", TYPE_KEYWORD, @$.first_line);
			insert($$,$1);insert($$,$2);insert($$,$3);
		}
		| IF LP Exp RP Stmt %prec LOWER_THAN_ELSE{
			$$ = init("Stmt", "", TYPE_KEYWORD, @$.first_line);
			insert($$,$1);insert($$,$2);insert($$,$3);
			insert($$,$4);insert($$,$5);
		}
		| IF LP Exp RP Stmt ELSE Stmt{
			$$ = init("Stmt", "", TYPE_KEYWORD, @$.first_line);
			insert($$,$1);insert($$,$2);insert($$,$3);
			insert($$,$4);insert($$,$5);insert($$,$6);insert($$,$7);
		}
		| WHILE LP Exp RP Stmt{
			$$ = init("Stmt", "", TYPE_KEYWORD, @$.first_line);
			insert($$,$1);insert($$,$2);insert($$,$3);
			insert($$,$4);insert($$,$5);
		}
		;

/* Local Definitions */
DefList	: Def DefList{
			$$ = init("DefList", "", TYPE_KEYWORD, @$.first_line);
			insert($$,$1);insert($$,$2);
		}
		| /* empty */{ $$=NULL; }
		;
Def	: Specifier DecList SEMI{
		$$ = init("Def", "", TYPE_KEYWORD, @$.first_line);
		insert($$,$1);insert($$,$2);insert($$,$3);
	}
	;
DecList	: Dec{
			$$ = init("DecList", "", TYPE_KEYWORD, @$.first_line);
			insert($$,$1);
		}
		| Dec COMMA DecList{
			$$ = init("DecList", "", TYPE_KEYWORD, @$.first_line);
			insert($$,$1);insert($$,$2);insert($$,$3);
		}
		;
Dec	: VarDec{
		$$ = init("Dec", "", TYPE_KEYWORD, @$.first_line);
		insert($$,$1);
	}
	| VarDec ASSIGNOP Exp{
		$$ = init("Dec", "", TYPE_KEYWORD, @$.first_line);
		insert($$,$1);insert($$,$2);insert($$,$3);
	}

/* Expressions */
Exp	: Exp ASSIGNOP Exp{
		$$ = init("Exp", "", TYPE_KEYWORD, @$.first_line);
		insert($$,$1);insert($$,$2);insert($$,$3);
	}
	| Exp AND Exp{
		$$ = init("Exp", "", TYPE_KEYWORD, @$.first_line);
		insert($$,$1);
	}
	| Exp OR Exp{
		$$ = init("Exp", "", TYPE_KEYWORD, @$.first_line);
		insert($$,$1);insert($$,$2);insert($$,$3);
	}
	| Exp RELOP Exp{
		$$ = init("Exp", "", TYPE_KEYWORD, @$.first_line);
		insert($$,$1);insert($$,$2);insert($$,$3);
	}
	| Exp PLUS Exp{
		$$ = init("Exp", "", TYPE_KEYWORD, @$.first_line);
		insert($$,$1);insert($$,$2);insert($$,$3);
	}
	| Exp MINUS Exp{
		$$ = init("Exp", "", TYPE_KEYWORD, @$.first_line);
		insert($$,$1);insert($$,$2);insert($$,$3);
	}
	| Exp STAR Exp{
		$$ = init("Exp", "", TYPE_KEYWORD, @$.first_line);
		insert($$,$1);insert($$,$2);insert($$,$3);
	}
	| Exp DIV Exp{
		$$ = init("Exp", "", TYPE_KEYWORD, @$.first_line);
		insert($$,$1);insert($$,$2);insert($$,$3);
	}
	| LP Exp RP{
		$$ = init("Exp", "", TYPE_KEYWORD, @$.first_line);
		insert($$,$1);insert($$,$2);insert($$,$3);
	}
	| MINUS Exp{
		$$ = init("Exp", "", TYPE_KEYWORD, @$.first_line);
		insert($$,$1);insert($$,$2);
	}
	| NOT Exp{
		$$ = init("Exp", "", TYPE_KEYWORD, @$.first_line);
		insert($$,$1);insert($$,$2);
	}
	| ID LP Args RP{
		$$ = init("Exp", "", TYPE_KEYWORD, @$.first_line);
		insert($$,$1);insert($$,$2);insert($$,$3);insert($$,$4);
	}
	| ID LP RP{
		$$ = init("Exp", "", TYPE_KEYWORD, @$.first_line);
		insert($$,$1);insert($$,$2);insert($$,$3);
	}
	| Exp LB Exp RB{
		$$ = init("Exp", "", TYPE_KEYWORD, @$.first_line);
		insert($$,$1);insert($$,$2);insert($$,$3);insert($$,$4);
	}
	| Exp DOT ID{
		$$ = init("Exp", "", TYPE_KEYWORD, @$.first_line);
		insert($$,$1);insert($$,$2);insert($$,$3);
	}
	| ID{
		$$ = init("Exp", "", TYPE_KEYWORD, @$.first_line);
		insert($$,$1);
	}
	| INT{
		$$ = init("Exp", "", TYPE_KEYWORD, @$.first_line);
		insert($$,$1);
	}
	| FLOAT{
		$$ = init("Exp", "", TYPE_KEYWORD, @$.first_line);
		insert($$,$1);
	}
	;
Args	: Exp COMMA Args{
			$$ = init("Args", "", TYPE_KEYWORD, @$.first_line);
			insert($$,$1);insert($$,$2);insert($$,$3);
		}
	 	| Exp{
			$$ = init("Args", "", TYPE_KEYWORD, @$.first_line);
			insert($$,$1);
		}
		;

/* Comments */

%%


void yyerror(char* msg){
	fprintf(stderr, "Error type B at Line %d:  %s\n",yylineno,msg);
}
