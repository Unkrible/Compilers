#include"stdlib.h"
#include"stdio.h"
#include"string.h"
#include"InterCode.h"

InterCode code_head=NULL;
InterCode code_tail=NULL;

int temVarNo = 1;
int labelNo = 1;
int varNo = 1;
char *zeroStr;
char *oneStr;
char *neStr;

void insertCode(InterCode p){
	if(p==NULL)
		exit(-1);
	//assert(p!=NULL);
    p->pre=NULL;
	p->next=NULL;
	if(code_head==NULL){
		code_head = p;
		code_tail = p;
	}
	else{
		p->pre = code_tail;
		code_tail->next = p;
		code_tail = p;
	}
}

void deleteCode(InterCode p){
	if(p==NULL)
		exit(-1);
	//assert(p!=NULL);
	if(p==code_head){
		code_head = p->next;
	}
	else{
		p->pre->next = p->next;
	}

	if(p==code_tail){
		code_tail = p->pre;
	}
	else{
		p->next->pre = p->pre;
	}
}

void printOp(Operand p, FILE* fp){
	assert(p!=NULL);
	char str[50];
	memset(str, 0, sizeof(str));
	switch(p->kind){
		case TEMPVAR:
			sprintf(str, "t%d ", p->u.var_no);
			fputs(str, fp);
			break;
		case VARIABLE:
			assert(p->u.value!=NULL);
			sprintf(str, "%s ", p->u.value);
			fputs(str, fp);
			break;
		case CONSTANT:
			sprintf(str, "#%s ", p->u.value);
			fputs(str, fp);
			break;
		case VADDRESS:
			fputs("*", fp);
			assert(p->u.addr!=NULL);
			printOp(p->u.addr, fp);
			break;
		case LABEL:
			sprintf(str, "label%d ", p->u.var_no);
			fputs(str, fp);
			break;
		case FUNCTION_:
			sprintf(str, "%s ", p->u.value);
			fputs(str, fp);
			break;
		case TADDRESS:
			fputs("*t",fp);
			sprintf(str,"%d",p->u.addr->u.var_no);
			fputs(str,fp);
			break;
		default:
			assert(0);
	}
}

void printCode(char* fileName){
	FILE* fp = fopen(fileName, "w");
	if(fp==NULL){
		printf("ERROR: Can not open file \"%s\".",fileName);
		return;
	}
	InterCode p;
	for(p=code_head;p!=NULL;p=p->next){
		switch(p->kind){
			case ASSIGN_N:
				printOp(p->u.assign.left, fp);
				fputs(":= ", fp);
				printOp(p->u.assign.right, fp);
				break;
			case ADD_N:
				printOp(p->u.binop.result, fp);
				fputs(":= ", fp);
				printOp(p->u.binop.op1, fp);
				fputs("+ ", fp);
				printOp(p->u.binop.op2, fp);
				break;
			case SUB_N:
				printOp(p->u.binop.result, fp);
				fputs(":= ", fp);
				printOp(p->u.binop.op1, fp);
				fputs("- ", fp);
				printOp(p->u.binop.op2, fp);
				break;
			case MUL_N:
				printOp(p->u.binop.result, fp);
				fputs(":= ", fp);
				printOp(p->u.binop.op1, fp);
				fputs("* ", fp);
				printOp(p->u.binop.op2, fp);
				break;
			case DIV_N:
				printOp(p->u.binop.result, fp);
				fputs(":= ", fp);
				printOp(p->u.binop.op1, fp);
				fputs("/ ", fp);
				printOp(p->u.binop.op2, fp);
				break;
			case RETURN_N:
				fputs("RETURN ", fp);
				printOp(p->u.sinop.op, fp);
				break;
			case LABEL_N:
				fputs("LABEL ", fp);
				printOp(p->u.sinop.op, fp);
				fputs(": ", fp);
				break;
			case GOTO_N:
				fputs("GOTO ", fp);
				printOp(p->u.sinop.op, fp);
				break;
			case IFGOTO_N:
				fputs("IF ", fp);
				printOp(p->u.triop.x, fp);
				fputs(p->u.triop.op, fp);
				fputs(" ", fp);
				printOp(p->u.triop.y, fp);
				fputs("GOTO ", fp);
				printOp(p->u.triop.label, fp);
				break;
			case READ_N:
				fputs("READ ", fp);
				printOp(p->u.sinop.op, fp);
				break;
			case WRITE_N:
				fputs("WRITE ", fp);
				printOp(p->u.sinop.op, fp);
				break;
			case CALL_N:
				printOp(p->u.assign.left, fp);
				fputs(":= CALL ", fp);
				printOp(p->u.assign.right, fp);
				break;
			case ARG_N:
				fputs("ARG ", fp);
				printOp(p->u.sinop.op, fp);
				break;
			case FUNCTION_N:
				fputs("FUNCTION ", fp);
				printOp(p->u.sinop.op, fp);
				fputs(": ", fp);
				break;
			case PARAM_N:
				fputs("PARAM ", fp);
				printOp(p->u.sinop.op, fp);
				break;
			case DEC_N:
				fputs("DEC ", fp);
				printOp(p->u.dec.op, fp);
				char sizeStr[16];
				sprintf(sizeStr, "%d", p->u.dec.size);
				fputs(sizeStr, fp);
				break;
			case ADDRESS_N:
				printOp(p->u.assign.left, fp);
				fputs(":= &", fp);
				printOp(p->u.assign.right, fp);
				break;
			default:
				assert(0);
		}
		fputs("\n", fp);
	}
	fclose(fp);
}

void optimizeGoto()
{
	InterCode c;
	for(c=code_head;c!=NULL;c=c->next)
	{
		if(c->kind==IFGOTO_N)
		{
			InterCode c1=c;
			InterCode c2=c->next;
			if(c2==NULL)
				continue;
			InterCode c3=c2->next;
			if(c3==NULL)
				continue;
			if(c2->kind==GOTO_N&&c3->kind==LABEL_N&&c1->u.triop.label==c3->u.sinop.op)
			{
				c1->u.triop.label=c2->u.sinop.op;
				deleteCode(c2);
				if(strcmp(c1->u.triop.op,"==")==0)
				{
					char* newop = malloc(4);
					memset(newop, 0, 4);
					strcpy(newop,"!=");
					c1->u.triop.op=newop;
				}
				else if(strcmp(c1->u.triop.op,"!=")==0)
				{
					char* newop = malloc(4);
					memset(newop, 0, 4);
					strcpy(newop,"==");
					c1->u.triop.op=newop;
				}
				else if(strcmp(c1->u.triop.op,">=")==0)
				{
					char* newop = malloc(4);
					memset(newop, 0, 4);
					strcpy(newop,"<");
					c1->u.triop.op=newop;
				}
				else if(strcmp(c1->u.triop.op,"<")==0)
				{
					char* newop = malloc(4);
					memset(newop, 0, 4);
					strcpy(newop,">=");
					c1->u.triop.op=newop;
				}
				else if(strcmp(c1->u.triop.op,">")==0)
				{
					char* newop = malloc(4);
					memset(newop, 0, 4);
					strcpy(newop,"<=");
					c1->u.triop.op=newop;
				}
				else if(strcmp(c1->u.triop.op,"<=")==0)
				{
					char* newop = malloc(4);
					memset(newop, 0, 4);
					strcpy(newop,">");
					c1->u.triop.op=newop;
				}
			}
		}
		else if(c->kind==GOTO_N)
		{
			InterCode c1=c;
			InterCode c2=c->next;
			if(c2==NULL)
				continue;
			if(c2->kind==LABEL_N&&c1->u.sinop.op==c2->u.sinop.op)
				deleteCode(c1);
		}
	}
}

void deleteLabel()
{
	Label_No head=malloc(sizeof(struct Label_No_));
	head->no=-1;
	head->next=NULL;
	Label_No tail=head;
	InterCode c=code_head;

	while(c!=NULL)
	{
		if(c->kind==GOTO_N)
		{
			Label_No temp=malloc(sizeof(struct Label_No_));
			temp->no=c->u.sinop.op->u.var_no;
			temp->next=NULL;
			tail->next=temp;
			tail=temp;
		}
		else if(c->kind==IFGOTO_N)
		{
			Label_No temp=malloc(sizeof(struct Label_No_));
			temp->no=c->u.triop.label->u.var_no;
			temp->next=NULL;
			tail->next=temp;
			tail=temp;
		}
		c=c->next;
	}

	c=code_head;
	while(c!=NULL)
	{
		if(c->kind==LABEL_N)
		{
			int no=c->u.sinop.op->u.var_no;
			Label_No ln=head;
			while(ln!=NULL)
			{
				if(ln->no==no)break;
				ln=ln->next;
			}
			InterCode c1=c;
			c=c->next;
			if(ln==NULL)
				deleteCode(c1);
		}
		else
			c=c->next;
	}

	tail=NULL;
	while(head!=NULL)
	{
		Label_No ln=head;
		head=head->next;
		free(ln);
	}
}

void figureCon()
{
	InterCode p;
	for(p=code_head;p!=NULL;p=p->next)
	{
		if(p->kind==ADD_N||p->kind==SUB_N||p->kind==MUL_N||p->kind==DIV_N)
		{
			Operand op1 = p->u.binop.op1;
			Operand op2 = p->u.binop.op2;
			Operand result = p->u.binop.result;

			if(result->kind==TEMPVAR&&op1->kind==CONSTANT&&op2->kind==CONSTANT)
			{
				int o1=atoi(op1->u.value);
				int o2=atoi(op2->u.value);
				int r=0;
				switch(p->kind){
					case ADD_N:r=o1+o2;break;
					case SUB_N:r=o1-o2;break;
					case MUL_N:r=o1*o2;break;
					case DIV_N:r=o1/o2;break;
					default:break;
				}
				result->kind=CONSTANT;
				sprintf(result->u.value,"%d",r);
				deleteCode(p);
				continue;
			}
		}
	}
}
