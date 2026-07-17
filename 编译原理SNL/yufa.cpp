#include"yufa.h"
#include "adv.h"
#include<vector>
struct TreeNode;
int LL1Table[110][110];
TreeNode* currentP;
TreeNode* saveP;
int error = 0;
int lineno = 0;
Dec* temp;  //为保存类型需要的临时变量
int space = 0;
/*纪录表达式中，未匹配的左括号数目*/
int  expflag = 0;
/*判断简单表达式处理结束，整个表达式是否处理结束标识*/
/*当是条件表达式时，取假值，简单表达式时，取真值*/
/*用于函数preocess84*/
int  getExpResult = 1;
/*仅用于数组变量，故初始化为假，遇到数组变量时，将其
  改变为真，以便在函数process84中，即算术表达式结束时，
  从语法树栈中弹出相应指针，将数组下标表达式的结构链入
  节点中*/
int  getExpResult2 = 0;

string tmlv_string[] =
{
	/* 簿记单词符号 */
	"ENDFILE1", "ERROR1",
	/* 保留字 */
	"PROGRAM", "PROCEDURE", "TYPE","VAR" , "IF",
	"THEN", "ELSE", "FI", "WHILE", "DO",
	"ENDWH", "BEGIN", "END", "READ", "WRITE",
	"ARRAY", "OF", "RECORD", "RETURN",

	"INTEGER", "CHAR1",
	/* 多字符单词符号 */
	"ID", "INTC", "CHARC",
	/*特殊符号 */
	"ASSIGN", "EQ", "LT", "PLUS", "MINUS",
	"TIMES", "OVER", "LPAREN", "RPAREN", "DOT",
	"COLON", "SEMI", "COMMA", "LMIDPAREN", "RMIDPAREN",
	"UNDERANGE"
};
//TokenType token;
/*所有终极符*/
typedef LexType tmlType;
/*所有非终极符*/
typedef enum
{
	Program, ProgramHead, ProgramName, DeclarePart,
	TypeDec, TypeDeclaration, TypeDecList, TypeDecMore,
	TypeId, TypeName, BaseType, StructureType,
	ArrayType, Low, Top, RecType,
	FieldDecList, FieldDecMore, IdList, IdMore,
	VarDec, VarDeclaration, VarDecList, VarDecMore,
	VarIdList, VarIdMore, ProcDec, ProcDeclaration,
	ProcDecMore, ProcName, ParamList, ParamDecList,
	ParamMore, Param, FormList, FidMore,
	ProcDecPart, ProcBody, ProgramBody, StmList,
	StmMore, Stm, AssCall, AssignmentRest,
	ConditionalStm, StmL, LoopStm, InputStm,
	InVar, OutputStm, ReturnStm, CallStmRest,
	ActParamList, ActParamMore, RelExp, OtherRelE,
	Exp, OtherTerm, Term, OtherFactor,
	Factor, Variable, VariMore, FieldVar,
	FieldVarMore, CmpOp, AddOp, MultOp

}  ntmlType;
string ntmlvtype_string[] = {
	"Program", "ProgramHead", "ProgramName","DeclarePart" ,
	"TypeDec", "TypeDeclaration", "TypeDecList", "TypeDecMore",
	"TypeId", "TypeName", "BaseType", "StructureType",
	"ArrayType", "Low", "Top", "RecType",
	"FieldDecList", "FieldDecMore", "IdList", "IdMore",
	"VarDec", "VarDeclaration", "VarDecList","VarDecMore",
	"VarIdList", "VarIdMore", "ProcDec", "ProcDeclaration",
	"ProcDecMore","ProcName" , "ParamList","ParamDecList" ,
	"ParamMore", "Param", "FormList", "FidMore",
	"ProcDecPart", "ProcBody", "ProgramBody", "StmList",
	"StmMore", "Stm", "AssCall", "AssignmentRest",
	"ConditionalStm", "StmL", "LoopStm", "InputStm",
	"InVar", "OutputStm", "ReturnStm", "CallStmRest",
	"ActParamList", "ActParamMore", "RelExp", "OtherRelE",
	"Exp", "OtherTerm", "Term", "OtherFactor",
	"Factor", "Variable", "VariMore", "FieldVar",
	"FieldVarMore", "CmpOp", "AddOp", "MultOp"
};
vector<TokenType>TokenList;
void getTokenList()
{
	TokenList.clear();
	int fp_num = 0;
	FILE* p = fopen("Tokenlist.txt", "rb");
	fseek(p, fp_num * sizeof(TokenType), 0);
	int b = sizeof(TokenType);
	int a = 1;
	while (1)
	{
		TokenType temp;
		a = fread(&temp, sizeof(TokenType), 1, p);
		if (a != 1)break;
		TokenList.push_back(temp);
		fp_num++;
	}
	fclose(p);
}
int string_toenum_ntmlv(string stri)//非终极符
{
	for (int i = 0; i < 68; i++)
	{
		if (stri == ntmlvtype_string[i])
			return i;
	}
	return -1;
}
int string_toenum_tmlv(string stri)//终极符
{
	for (int i = 0; i < 42; i++)
	{
		if (stri == tmlv_string[i])
			return i;
	}
	return -1;
}
vector<pair<int, string>>SymbolStack;//符号栈
vector<TreeNode**>SyntaxTreeStack;//语法树栈
vector<TreeNode*>OperatorStack;//操作符栈
vector<TreeNode*>OperandStack;//操作数栈
int token_pointer = 0;
void CreateLL1Table()
{
	/*初始化LL1表元素*/
	for (int i = 0; i < 110; i++)
		for (int j = 0; j < 110; j++)
			LL1Table[i][j] = 0;

	LL1Table[Program][PROGRAM] = 1;
	LL1Table[ProgramHead][PROGRAM] = 2;
	LL1Table[ProgramName][ID] = 3;
	LL1Table[DeclarePart][TYPE] = 4;
	LL1Table[DeclarePart][VAR] = 4;
	LL1Table[DeclarePart][PROCEDURE] = 4;
	LL1Table[DeclarePart][BEGIN] = 4;
	LL1Table[TypeDec][VAR] = 5;
	LL1Table[TypeDec][PROCEDURE] = 5;
	LL1Table[TypeDec][BEGIN] = 5;
	LL1Table[TypeDec][TYPE] = 6;
	LL1Table[TypeDeclaration][TYPE] = 7;
	LL1Table[TypeDecList][ID] = 8;
	LL1Table[TypeDecMore][VAR] = 9;
	LL1Table[TypeDecMore][PROCEDURE] = 9;
	LL1Table[TypeDecMore][BEGIN] = 9;
	LL1Table[TypeDecMore][ID] = 10;
	LL1Table[TypeId][ID] = 11;
	LL1Table[TypeName][INTEGER] = 12;
	LL1Table[TypeName][CHAR1] = 12;
	LL1Table[TypeName][ARRAY] = 13;
	LL1Table[TypeName][RECORD] = 13;
	LL1Table[TypeName][ID] = 14;
	LL1Table[BaseType][INTEGER] = 15;
	LL1Table[BaseType][CHAR1] = 16;
	LL1Table[StructureType][ARRAY] = 17;
	LL1Table[StructureType][RECORD] = 18;
	LL1Table[ArrayType][ARRAY] = 19;
	LL1Table[Low][INTC] = 20;
	LL1Table[Top][INTC] = 21;
	LL1Table[RecType][RECORD] = 22;
	LL1Table[FieldDecList][INTEGER] = 23;
	LL1Table[FieldDecList][CHAR1] = 23;
	LL1Table[FieldDecList][ARRAY] = 24;
	LL1Table[FieldDecMore][END] = 25;
	LL1Table[FieldDecMore][INTEGER] = 26;
	LL1Table[FieldDecMore][CHAR1] = 26;
	LL1Table[FieldDecMore][ARRAY] = 26;
	LL1Table[IdList][ID] = 27;
	LL1Table[IdMore][SEMI] = 28;
	LL1Table[IdMore][COMMA] = 29;
	LL1Table[VarDec][PROCEDURE] = 30;
	LL1Table[VarDec][BEGIN] = 30;
	LL1Table[VarDec][VAR] = 31;
	LL1Table[VarDeclaration][VAR] = 32;
	LL1Table[VarDecList][INTEGER] = 33;
	LL1Table[VarDecList][CHAR1] = 33;
	LL1Table[VarDecList][ARRAY] = 33;
	LL1Table[VarDecList][RECORD] = 33;
	LL1Table[VarDecList][ID] = 33;
	LL1Table[VarDecMore][PROCEDURE] = 34;
	LL1Table[VarDecMore][BEGIN] = 34;
	LL1Table[VarDecMore][INTEGER] = 35;
	LL1Table[VarDecMore][CHAR1] = 35;
	LL1Table[VarDecMore][ARRAY] = 35;
	LL1Table[VarDecMore][RECORD] = 35;
	LL1Table[VarDecMore][ID] = 35;
	LL1Table[VarIdList][ID] = 36;
	LL1Table[VarIdMore][SEMI] = 37;
	LL1Table[VarIdMore][COMMA] = 38;
	LL1Table[ProcDec][BEGIN] = 39;
	LL1Table[ProcDec][PROCEDURE] = 40;
	LL1Table[ProcDeclaration][PROCEDURE] = 41;
	LL1Table[ProcDecMore][BEGIN] = 42;
	LL1Table[ProcDecMore][PROCEDURE] = 43;
	LL1Table[ProcName][ID] = 44;
	LL1Table[ParamList][RPAREN] = 45;
	LL1Table[ParamList][INTEGER] = 46;
	LL1Table[ParamList][CHAR1] = 46;
	LL1Table[ParamList][ARRAY] = 46;
	LL1Table[ParamList][RECORD] = 46;
	LL1Table[ParamList][ID] = 46;
	LL1Table[ParamList][VAR] = 46;
	LL1Table[ParamDecList][INTEGER] = 47;
	LL1Table[ParamDecList][CHAR1] = 47;
	LL1Table[ParamDecList][ARRAY] = 47;
	LL1Table[ParamDecList][RECORD] = 47;
	LL1Table[ParamDecList][ID] = 47;
	LL1Table[ParamDecList][VAR] = 47;
	LL1Table[ParamMore][RPAREN] = 48;
	LL1Table[ParamMore][SEMI] = 49;
	LL1Table[Param][INTEGER] = 50;
	LL1Table[Param][CHAR1] = 50;
	LL1Table[Param][ARRAY] = 50;
	LL1Table[Param][RECORD] = 50;
	LL1Table[Param][ID] = 50;
	LL1Table[Param][VAR] = 51;
	LL1Table[FormList][ID] = 52;
	LL1Table[FidMore][SEMI] = 53;
	LL1Table[FidMore][RPAREN] = 53;
	LL1Table[FidMore][COMMA] = 54;
	LL1Table[ProcDecPart][TYPE] = 55;
	LL1Table[ProcDecPart][VAR] = 55;
	LL1Table[ProcDecPart][PROCEDURE] = 55;
	LL1Table[ProcDecPart][BEGIN] = 55;
	LL1Table[ProcBody][BEGIN] = 56;
	LL1Table[ProgramBody][BEGIN] = 57;
	LL1Table[StmList][ID] = 58;
	LL1Table[StmList][IF] = 58;
	LL1Table[StmList][WHILE] = 58;
	LL1Table[StmList][RETURN] = 58;
	LL1Table[StmList][READ] = 58;
	LL1Table[StmList][WRITE] = 58;
	LL1Table[StmList][ELSE] = 58;
	LL1Table[StmList][FI] = 58;
	LL1Table[StmList][ENDWH] = 58;
	LL1Table[StmList][END] = 58;
	LL1Table[StmMore][END] = 59;
	LL1Table[StmMore][ENDWH] = 59;
	LL1Table[StmMore][ELSE] = 59;
	LL1Table[StmMore][FI] = 59;
	LL1Table[StmMore][SEMI] = 60;
	LL1Table[Stm][IF] = 61;
	LL1Table[Stm][WHILE] = 62;
	LL1Table[Stm][READ] = 63;
	LL1Table[Stm][WRITE] = 64;
	LL1Table[Stm][RETURN] = 65;
	LL1Table[Stm][ID] = 66;
	LL1Table[AssCall][ASSIGN] = 67;
	LL1Table[AssCall][LMIDPAREN] = 67;
	LL1Table[AssCall][DOT] = 67;
	LL1Table[AssCall][LPAREN] = 68;
	LL1Table[AssignmentRest][ASSIGN] = 69;
	LL1Table[AssignmentRest][LMIDPAREN] = 69;
	LL1Table[AssignmentRest][DOT] = 69;
	LL1Table[ConditionalStm][IF] = 70;
	LL1Table[LoopStm][WHILE] = 71;
	LL1Table[InputStm][READ] = 72;
	LL1Table[InVar][ID] = 73;
	LL1Table[OutputStm][WRITE] = 74;
	LL1Table[ReturnStm][RETURN] = 75;
	LL1Table[CallStmRest][LPAREN] = 76;
	LL1Table[ActParamList][RPAREN] = 77;
	LL1Table[ActParamList][ID] = 78;
	LL1Table[ActParamList][INTC] = 78;
	LL1Table[ActParamList][CHARC] = 78;
	LL1Table[ActParamList][LPAREN] = 78;
	LL1Table[ActParamMore][RPAREN] = 79;
	LL1Table[ActParamMore][COMMA] = 80;
	LL1Table[RelExp][LPAREN] = 81;
	LL1Table[RelExp][INTC] = 81;
	LL1Table[RelExp][CHARC] = 81;
	LL1Table[RelExp][ID] = 81;
	LL1Table[OtherRelE][LT] = 82;
	LL1Table[OtherRelE][EQ] = 82;
	LL1Table[Exp][LPAREN] = 83;
	LL1Table[Exp][INTC] = 83;
	LL1Table[Exp][CHARC] = 83;
	LL1Table[Exp][ID] = 83;
	LL1Table[OtherTerm][LT] = 84;
	LL1Table[OtherTerm][EQ] = 84;
	LL1Table[OtherTerm][THEN] = 84;
	LL1Table[OtherTerm][DO] = 84;
	LL1Table[OtherTerm][RPAREN] = 84;
	LL1Table[OtherTerm][END] = 84;
	LL1Table[OtherTerm][SEMI] = 84;
	LL1Table[OtherTerm][COMMA] = 84;
	LL1Table[OtherTerm][ENDWH] = 84;
	LL1Table[OtherTerm][ELSE] = 84;
	LL1Table[OtherTerm][FI] = 84;
	LL1Table[OtherTerm][RMIDPAREN] = 84;
	LL1Table[OtherTerm][PLUS] = 85;
	LL1Table[OtherTerm][MINUS] = 85;
	LL1Table[Term][LPAREN] = 86;
	LL1Table[Term][INTC] = 86;
	LL1Table[Term][CHARC] = 86;
	LL1Table[Term][ID] = 86;
	LL1Table[OtherFactor][PLUS] = 87;
	LL1Table[OtherFactor][MINUS] = 87;
	LL1Table[OtherFactor][LT] = 87;
	LL1Table[OtherFactor][EQ] = 87;
	LL1Table[OtherFactor][THEN] = 87;
	LL1Table[OtherFactor][ELSE] = 87;
	LL1Table[OtherFactor][FI] = 87;
	LL1Table[OtherFactor][DO] = 87;
	LL1Table[OtherFactor][ENDWH] = 87;
	LL1Table[OtherFactor][RPAREN] = 87;
	LL1Table[OtherFactor][END] = 87;
	LL1Table[OtherFactor][SEMI] = 87;
	LL1Table[OtherFactor][COMMA] = 87;
	LL1Table[OtherFactor][RMIDPAREN] = 87;
	LL1Table[OtherFactor][TIMES] = 88;
	LL1Table[OtherFactor][OVER] = 88;
	LL1Table[Factor][LPAREN] = 89;
	LL1Table[Factor][INTC] = 90;
	LL1Table[Factor][ID] = 91;
	LL1Table[Variable][ID] = 92;
	LL1Table[VariMore][ASSIGN] = 93;
	LL1Table[VariMore][TIMES] = 93;
	LL1Table[VariMore][OVER] = 93;
	LL1Table[VariMore][PLUS] = 93;
	LL1Table[VariMore][MINUS] = 93;
	LL1Table[VariMore][LT] = 93;
	LL1Table[VariMore][EQ] = 93;
	LL1Table[VariMore][THEN] = 93;
	LL1Table[VariMore][ELSE] = 93;
	LL1Table[VariMore][FI] = 93;
	LL1Table[VariMore][DO] = 93;
	LL1Table[VariMore][ENDWH] = 93;
	LL1Table[VariMore][RPAREN] = 93;
	LL1Table[VariMore][END] = 93;
	LL1Table[VariMore][SEMI] = 93;
	LL1Table[VariMore][COMMA] = 93;
	LL1Table[VariMore][RMIDPAREN] = 93;
	LL1Table[VariMore][LMIDPAREN] = 94;
	LL1Table[VariMore][DOT] = 95;
	LL1Table[FieldVar][ID] = 96;
	LL1Table[FieldVarMore][ASSIGN] = 97;
	LL1Table[FieldVarMore][TIMES] = 97;
	LL1Table[FieldVarMore][OVER] = 97;
	LL1Table[FieldVarMore][PLUS] = 97;
	LL1Table[FieldVarMore][MINUS] = 97;
	LL1Table[FieldVarMore][LT] = 97;
	LL1Table[FieldVarMore][EQ] = 97;
	LL1Table[FieldVarMore][THEN] = 97;
	LL1Table[FieldVarMore][ELSE] = 97;
	LL1Table[FieldVarMore][FI] = 97;
	LL1Table[FieldVarMore][DO] = 97;
	LL1Table[FieldVarMore][ENDWH] = 97;
	LL1Table[FieldVarMore][RPAREN] = 97;
	LL1Table[FieldVarMore][END] = 97;
	LL1Table[FieldVarMore][SEMI] = 97;
	LL1Table[FieldVarMore][COMMA] = 97;
	LL1Table[FieldVarMore][LMIDPAREN] = 98;
	LL1Table[CmpOp][LT] = 99;
	LL1Table[CmpOp][EQ] = 100;
	LL1Table[AddOp][PLUS] = 101;
	LL1Table[AddOp][MINUS] = 102;
	LL1Table[MultOp][TIMES] = 103;
	LL1Table[MultOp][OVER] = 104;
	LL1Table[Stm][ELSE] = 105;
	LL1Table[Stm][FI] = 106;
	LL1Table[Stm][ENDWH] = 107;
	LL1Table[Stm][END] = 108;
	LL1Table[Factor][CHARC] = 109;
}
void predict(int num)
{
	switch (num)
	{
	case 1:       process1();	break;
	case 2:       process2();	break;
	case 3:       process3();	break;
	case 4:       process4();	break;
	case 5:	      process5();	break;
	case 6:	      process6();	break;
	case 7:	      process7();	break;
	case 8:	      process8();	break;
	case 9:	      process9();	break;
	case 10:      process10();	break;
	case 11:      process11();	break;
	case 12:	  process12();	break;
	case 13:	  process13();	break;
	case 14:	  process14();	break;
	case 15:	  process15();	break;
	case 16:	  process16();	break;
	case 17:	  process17();	break;
	case 18:	  process18();	break;
	case 19:	  process19();	break;
	case 20:	  process20();	break;
	case 21:	  process21();	break;
	case 22:	  process22();	break;
	case 23:	  process23();	break;
	case 24:	  process24();	break;
	case 25:	  process25();	break;
	case 26:	  process26();	break;
	case 27:	  process27();	break;
	case 28:	  process28();	break;
	case 29:	  process29();	break;
	case 30:	  process30();	break;
	case 31:	  process31();	break;
	case 32:	  process32();	break;
	case 33:	  process33();	break;
	case 34:	  process34();	break;
	case 35:	  process35();	break;
	case 36:	  process36();	break;
	case 37:	  process37();	break;
	case 38:	  process38();	break;
	case 39:	  process39();	break;
	case 40:	  process40();	break;
	case 41:	  process41();	break;
	case 42:	  process42();	break;
	case 43:	  process43();	break;
	case 44:	  process44();	break;
	case 45:	  process45();	break;
	case 46:      process46();	break;
	case 47:	  process47();	break;
	case 48:	  process48();	break;
	case 49:	  process49();	break;
	case 50:	  process50();	break;
	case 51:	  process51();	break;
	case 52:	  process52();	break;
	case 53:	  process53();	break;
	case 54:	  process54();	break;
	case 55:	  process55();	break;
	case 56:	  process56();	break;
	case 57:	  process57();  break;
	case 58:	  process58();	break;
	case 59:	  process59();	break;
	case 60:	  process60();	break;
	case 61:	  process61();	break;
	case 62:	  process62();	break;
	case 63:	  process63();	break;
	case 64:	  process64();	break;
	case 65:	  process65();	break;
	case 66:	  process66();	break;
	case 67:	  process67();	break;
	case 68:	  process68();	break;
	case 69:      process69();	break;
	case 70:      process70();	break;
	case 71:	  process71();	break;
	case 72:	  process72();	break;
	case 73:	  process73();	break;
	case 74:	  process74();	break;
	case 75:	  process75();	break;
	case 76:	  process76();	break;
	case 77:	  process77();	break;
	case 78:      process78();	break;
	case 79:      process79();	break;
	case 80:	  process80();	break;
	case 81:	  process81();	break;
	case 82:	  process82();	break;
	case 83:	  process83();	break;
	case 84:	  process84();	break;
	case 85:	  process85();	break;
	case 86:	  process86();	break;
	case 87:	  process87();	break;
	case 88:	  process88();	break;
	case 89:	  process89();	break;
	case 90:	  process90();	break;
	case 91:	  process91();	break;
	case 92:	  process92();	break;
	case 93:	  process93();	break;
	case 94:	  process94();	break;
	case 95:	  process95();	break;
	case 96:	  process96();	break;
	case 97:      process97();	break;
	case 98:      process98();	break;
	case 99:      process99();	break;
	case 100:     process100();	break;
	case 101:     process101();	break;
	case 102:     process102();	break;
	case 103:     process103();	break;
	case 104:     process104();	break;
	case 105:     process105();	break;
	case 106:     process106(); break;
	case 107:     process107(); break;
	case 108:     process108(); break;
	case 109:     process109(); break;
	case 0:
	default: {
		if (error == 0)
		{
			printf("%d  非期望单词错\n", lineno);
		}
		error = 1;
	}
	}
}
int Priosity(LexType op)
{
	if (op == END)return 0;
	else if (op == LT || op == EQ)return 1;
	else if (op == PLUS || op == MINUS)return 2;
	else if (op == TIMES || op == OVER)return 3;
	else return -1;
}
TreeNode* newNode(NodeKind kind)
{
	TreeNode* r = new TreeNode();
	for (int i = 0; i < 3; i++)
	{
		r->child[i] = NULL;
	}
	r->Sibling = NULL;
	r->nodekind = kind;
	r->lineno = lineno;
	r->idnum = 0;
	for (int i = 0; i < 10; i++)
	{
		strcpy(r->name[i], "\0");
		r->table[i] = NULL;
	}
	return r;
}

TreeNode* newStmtNode(Stmt kind) {   // 创建语句类型语法树节点函数	
	TreeNode* r = new TreeNode();
	for (int i = 0; i < 3; i++)
	{
		r->child[i] = NULL;
	}
	r->Sibling = NULL;
	r->nodekind = StmtK;
	r->kind.stmt = kind;
	r->lineno = lineno;
	r->idnum = 0;
	for (int i = 0; i < 10; i++)
	{
		strcpy(r->name[i], "\0");
		r->table[i] = NULL;
	}
	return r;
}
TreeNode* newExpNode(Expk expkind) {
	TreeNode* r = new TreeNode();
	for (int i = 0; i < 3; i++)
	{
		r->child[i] = NULL;
	}
	r->Sibling = NULL;
	r->nodekind = ExpK;
	r->kind.exp = expkind;
	r->lineno = lineno;
	r->attr.ExpAttr.varkind = IdV;
	r->attr.ExpAttr.type = Void;
	r->idnum = 0;
	for (int i = 0; i < 10; i++)
	{
		strcpy(r->name[i], "\0");
		r->table[i] = NULL;
	}
	return r;
}
bool match(LexType a, string b)//终极符匹配
{
	if (tmlv_string[a] == b)return true;
	return false;
}

void showtree(TreeNode* tree)
{
	space = space + 4;//前面的偏移

	while (tree != NULL)
	{
		for (int i = 0; i < space; i++)
		{
			printf(" ");
		}

		// 添加修饰符，使输出更像一棵树
		printf("|---");

		switch (tree->nodekind)
		{
		case ProK:
			printf("Prok ");
			break;
		case PheadK:
		{
			printf("PheadK  ");
			printf("%s  ", tree->name[0]);
		}
		break;
		case DecK:
		{
			printf("DecK  ");
			switch (tree->kind.dec)
			{
			case  ArrayK:
			{
				printf("ArrayK  ");
				printf("%d  ", tree->attr.ArrayAttr.low);
				printf("%d  ", tree->attr.ArrayAttr.up);
				if (tree->attr.ArrayAttr.childType == CharK)
					printf("Chark  ");
				else if (tree->attr.ArrayAttr.childType == IntegerK)
					printf("IntegerK  ");
			}; break;
			case  CharK:
				printf("CharK  "); break;
			case  IntegerK:
				printf("IntegerK  "); break;
			case  RecordK:
				printf("RecordK  "); break;
			case  IdK:
				printf("IdK  ");
				printf("%s  ", tree->type_name);
				break;
			default:
				printf("error1!");
				error = 1;
			};
			if (tree->idnum != 0)
				for (int i = 0; i <= (tree->idnum); i++)
				{
					printf("%s  ", tree->name[i]);
				}
			else
			{
				printf("wrong!no var!\n");
				error = 1;
			}
		} break;
		case TypeK:
		{
			printf("TypeK  "); break;
		}
		case VarK:
			printf("VarK  ");
			break;
		case ProcDecK:
			printf("%s  ", "ProcDecK");
			printf("%s  ", tree->name[0]);
			break;
		case StmLK:
			printf("StmLk  "); break;
		case StmtK:
		{
			printf("StmtK  ");
			switch (tree->kind.stmt)
			{
			case IfK:
				printf("If  "); break;
			case ElseK:
				printf("Else "); break;
			case FiK:
				printf("Fi "); break;
			case WhileK:
				printf("While  "); break;
			case EndwhK:
				printf("Endwh  "); break;
			case AssignK:
				printf("Assign  ");
				break;
			case ReadK:
				printf("Read  ");
				printf("%s  ", tree->name[0]);
				break;
			case WriteK:
				printf("Write  "); break;
			case CallK:
				printf("Call  ");
				printf("%s  ", tree->name[0]);
				break;
			case ReturnK:
				printf("Return  "); break;
			case EndK:
				printf("End  "); break;
			default:
				printf("error2!");
				error = 1;
			}
		}; break;
		case ExpK:
		{
			printf("ExpK  ");
			switch (tree->kind.exp)
			{
			case OpK:
				printf("Op  ");
				switch (tree->attr.ExpAttr.op)
				{
				case EQ:   printf("=  "); break;
				case LT:   printf("<  "); break;
				case PLUS: printf("+  "); break;
				case MINUS:printf("-  "); break;
				case TIMES:printf("*  "); break;
				case OVER: printf("/  "); break;
				default:
					printf("error3!");
					error = 1;
				}

				if (tree->attr.ExpAttr.varkind == ArrayMembV)
				{
					printf("ArrayMember  ");
					printf("%s  ", tree->name[0]);
				}
				break;
			case ConstK:
				printf("Const  ");
				switch (tree->attr.ExpAttr.varkind)
				{
				case IdV:
					printf("%s  ", tree->name[0]);
					printf("IdV  ");
					break;
				case FieldMembV:
					printf("%s  ", tree->name[0]);
					printf("FieldMembV  ");
					break;
				case ArrayMembV:
					printf("%s  ", tree->name[0]);
					printf("ArrayMembV  ");
					break;
				default:
					printf("ConstK type error!");
					error = 1;
				}
				printf("%d  ", tree->attr.ExpAttr.val);
				break;
			case VariK:
				switch (tree->attr.ExpAttr.varkind)
				{
				case IdV:
					printf("%s  ", tree->name[0]);
					printf("IdV  ");
					break;
				case FieldMembV:
					printf("%s  ", tree->name[0]);
					printf("FieldMembV  ");
					break;
				case ArrayMembV:
					printf("%s  ", tree->name[0]);
					printf("ArrayMembV  ");
					break;
				default:
					printf("var type error!");
					error = 1;
				}
				break;
			default:
				printf("error4!");
				error = 1;
			}
		}; break;
		default:
			printf("error5!");
			error = 1;
		}

		printf("\n");

		for (int i = 0; i < 3; i++)
		{
			showtree(tree->child[i]);
		}

		tree = tree->Sibling;
	}
	space = space - 4;
}


TreeNode* praseLL()
{
	CreateLL1Table();
	int n = TokenList.size();
	SymbolStack.push_back({ 1,"Program" });
	TreeNode* rootpointer = newNode(ProK);
	SyntaxTreeStack.push_back(&(rootpointer->child[2]));
	SyntaxTreeStack.push_back(&(rootpointer->child[1]));
	SyntaxTreeStack.push_back(&(rootpointer->child[0]));
	while (!empty(SymbolStack) && !error)
	{
		lineno = TokenList[token_pointer].lineshow;
		token_single symbol_temp = SymbolStack.back();
		SymbolStack.pop_back();
		if (symbol_temp.first == 2)//判断是否是终极符
		{
			if (match(TokenList[token_pointer].Lex, symbol_temp.second))
			{
				token_pointer++;
			}
			else//终极符不匹配
			{
				printf("%d 终极符不匹配\n", TokenList[token_pointer].lineshow);
				error = 1;
				return nullptr;
			}
		}
		else//不是终极符
		{
			int x = string_toenum_ntmlv(symbol_temp.second);
			int pnum = LL1Table[x][TokenList[token_pointer].Lex];
			predict(pnum);
		}

	}
	if (token_pointer == n - 1 && error == 0)
	{
		printf("语法分析成功\n");
		return rootpointer;
	}
	else
	{
		printf("语法分析失败\n");
		return nullptr;
	}
}
void process1()
{
	SymbolStack.push_back({ 2, "DOT" });
	SymbolStack.push_back({ 1, "ProgramBody" });
	SymbolStack.push_back({ 1, "DeclarePart" });
	SymbolStack.push_back({ 1, "ProgramHead" });
}
void process2()
{
	SymbolStack.push_back({ 1, "ProgramName" });
	SymbolStack.push_back({ 2, "PROGRAM" });
	currentP = newNode(PheadK);
	TreeNode** t = SyntaxTreeStack.back();
	SyntaxTreeStack.pop_back();
	(*t) = currentP;
}
void process3()
{
	SymbolStack.push_back({ 2,"ID" });
	strcpy(currentP->name[0], TokenList[token_pointer].Sem);
	currentP->idnum++;
}
void process4()
{
	SymbolStack.push_back({ 1,"ProcDec" });
	SymbolStack.push_back({ 1,"VarDec" });
	SymbolStack.push_back({ 1,"TypeDec" });
}
void process5()
{

}
void process6()
{
	SymbolStack.push_back({ 1,"TypeDeclaration" });
}
void process7()
{
	SymbolStack.push_back({ 1,"TypeDecList" });
	SymbolStack.push_back({ 2,"TYPE" });
	currentP = newNode(TypeK);
	TreeNode** t = SyntaxTreeStack.back();
	SyntaxTreeStack.pop_back();
	(*t) = currentP;
	SyntaxTreeStack.push_back(&(currentP->Sibling));
	SyntaxTreeStack.push_back(&(currentP->child[0]));
}
void process8()
{
	SymbolStack.push_back({ 1,"TypeDecMore" });
	SymbolStack.push_back({ 2,"SEMI" });
	SymbolStack.push_back({ 1,"TypeName" });
	SymbolStack.push_back({ 2,"EQ" });
	SymbolStack.push_back({ 1,"TypeId" });
	currentP = newNode(DecK);
	TreeNode** t = SyntaxTreeStack.back();
	SyntaxTreeStack.pop_back();
	(*t) = currentP;
	SyntaxTreeStack.push_back(&(currentP->Sibling));
}
void process9()
{
	SyntaxTreeStack.pop_back();
}
void process10()
{
	SymbolStack.push_back({ 1,"TypeDecList" });
}
void process11()
{
	SymbolStack.push_back({ 2,"ID" });
	strcpy(currentP->name[0], TokenList[token_pointer].Sem);
	currentP->idnum++;
}
void process12()
{
	SymbolStack.push_back({ 1,"BaseType" });
	temp = &(currentP->kind.dec);
}
void process13()
{
	SymbolStack.push_back({ 1,"StructureType" });
}
void process14()
{
	SymbolStack.push_back({ 2,"ID" });
	currentP->kind.dec = IdK;
	//currentP->idnum++;
	strcpy(currentP->type_name, TokenList[token_pointer].Sem);
}
void process15()
{
	SymbolStack.push_back({ 2,"INTEGER" });
	(*temp) = IntegerK;
}
void process16()
{
	SymbolStack.push_back({ 2,"CHAR1" });
	(*temp) = CharK;
}
void process17()
{
	SymbolStack.push_back({ 1,"ArrayType" });
}
void process18()
{
	SymbolStack.push_back({ 1,"RecType" });
}
void process19()
{
	SymbolStack.push_back({ 1,"BaseType" });
	SymbolStack.push_back({ 2,"OF" });
	SymbolStack.push_back({ 2,"RMIDPAREN" });
	SymbolStack.push_back({ 1,"Top" });
	SymbolStack.push_back({ 2,"UNDERANGE" });
	SymbolStack.push_back({ 1,"Low" });
	SymbolStack.push_back({ 2,"LMIDPAREN" });
	SymbolStack.push_back({ 2,"ARRAY" });
	currentP->kind.dec = ArrayK;
	temp = &(currentP->attr.ArrayAttr.childType);
}
void process20()
{
	SymbolStack.push_back({ 2,"INTC" });
	(*currentP).attr.ArrayAttr.low = stoi(TokenList[token_pointer].Sem);
}
void process21()
{
	SymbolStack.push_back({ 2,"INTC" });
	(*currentP).attr.ArrayAttr.up = stoi(TokenList[token_pointer].Sem);
}
void process22()
{
	SymbolStack.push_back({ 2,"END" });
	SymbolStack.push_back({ 1,"FieldDecList" });
	SymbolStack.push_back({ 2,"RECORD" });
	currentP->kind.dec = RecordK;
	saveP = currentP;
	SyntaxTreeStack.push_back(&(currentP->child[0]));
}
void process23()
{
	SymbolStack.push_back({ 1,"FieldDecMore" });
	SymbolStack.push_back({ 2,"SEMI" });
	SymbolStack.push_back({ 1,"IdList" });
	SymbolStack.push_back({ 1,"BaseType" });
	currentP = newNode(DecK); /*生成记录类型的下一个域，不添任何信息*/
	TreeNode** t = SyntaxTreeStack.back();
	SyntaxTreeStack.pop_back();
	(*t) = currentP;
	temp = (&(currentP->kind.dec));
	SyntaxTreeStack.push_back(&((*currentP).Sibling));

}
void process24()
{
	SymbolStack.push_back({ 1,"FieldDecMore" });
	SymbolStack.push_back({ 2,"SEMI" });
	SymbolStack.push_back({ 1,"IdList" });
	SymbolStack.push_back({ 1,"ArrayType" });
	currentP = newNode(DecK);
	TreeNode** t = SyntaxTreeStack.back();
	SyntaxTreeStack.pop_back();
	(*t) = currentP;
	SyntaxTreeStack.push_back(&(currentP->Sibling));
}
void process25()
{
	SyntaxTreeStack.pop_back();
	currentP = saveP;

}
void process26()
{
	SymbolStack.push_back({ 1,"FieldDecList" });
}
void process27()
{
	SymbolStack.push_back({ 1,"IdMore" });
	SymbolStack.push_back({ 2,"ID" });
	strcpy(currentP->name[currentP->idnum], TokenList[token_pointer].Sem);
	currentP->idnum++;
}
void process28()
{

}
void process29()
{
	SymbolStack.push_back({ 1,"IdList" });
	SymbolStack.push_back({ 2,"COMMA" });
}
void process30()
{

}
void process31()
{
	SymbolStack.push_back({ 1,"VarDeclaration" });
}
void process32()
{
	SymbolStack.push_back({ 1,"VarDecList" });
	SymbolStack.push_back({ 2,"VAR" });
	currentP = newNode(VarK);
	TreeNode** t = SyntaxTreeStack.back();
	SyntaxTreeStack.pop_back();
	(*t) = currentP;
	SyntaxTreeStack.push_back(&(currentP->Sibling));
	SyntaxTreeStack.push_back(&(currentP->child[0]));
}
void process33()
{
	SymbolStack.push_back({ 1,"VarDecMore" });
	SymbolStack.push_back({ 2,"SEMI" });
	SymbolStack.push_back({ 1,"VarIdList" });
	SymbolStack.push_back({ 1,"TypeName" });//TypeDef
	currentP = newNode(DecK);
	TreeNode** t = SyntaxTreeStack.back();
	SyntaxTreeStack.pop_back();
	(*t) = currentP;
	SyntaxTreeStack.push_back(&(currentP->Sibling));

}
void process34()
{
	SyntaxTreeStack.pop_back();
}
void process35()
{
	SymbolStack.push_back({ 1,"VarDecList" });
}
void process36()
{
	SymbolStack.push_back({ 1,"VarIdMore" });//IdMore
	SymbolStack.push_back({ 2,"ID" });
	strcpy(currentP->name[currentP->idnum], TokenList[token_pointer].Sem);
	currentP->idnum++;
}
void process37()
{

}
void process38()
{
	SymbolStack.push_back({ 1,"VarIdList" });
	SymbolStack.push_back({ 2,"COMMA" });

}
void process39()
{

}
void process40()
{
	SymbolStack.push_back({ 1,"ProcDeclaration" });
}
void process41()
{
	SymbolStack.push_back({ 1,"ProcDecMore" });
	SymbolStack.push_back({ 1,"ProcBody" });
	SymbolStack.push_back({ 1,"ProcDecPart" });
	SymbolStack.push_back({ 2,"SEMI" });
	SymbolStack.push_back({ 2,"RPAREN" });
	SymbolStack.push_back({ 1,"ParamList" });
	SymbolStack.push_back({ 2,"LPAREN" });
	SymbolStack.push_back({ 1,"ProcName" });
	SymbolStack.push_back({ 2,"PROCEDURE" });
	currentP = newNode(ProcDecK);
	TreeNode** t = SyntaxTreeStack.back();
	SyntaxTreeStack.pop_back();
	(*t) = currentP;
	SyntaxTreeStack.push_back(&(currentP->Sibling));
	SyntaxTreeStack.push_back(&(currentP->child[2]));
	SyntaxTreeStack.push_back(&(currentP->child[1]));
	SyntaxTreeStack.push_back(&(currentP->child[0]));

}
void process42()
{

}
void process43()
{
	SymbolStack.push_back({ 1,"ProcDeclaration" });
}
void process44()
{
	SymbolStack.push_back({ 2,"ID" });
	strcpy(currentP->name[0], TokenList[token_pointer].Sem);
	currentP->idnum++;
}
void process45()
{
	SyntaxTreeStack.pop_back();
}
void process46()
{
	SymbolStack.push_back({ 1,"ParamDecList" });
}
void process47()
{
	SymbolStack.push_back({ 1,"ParamMore" });
	SymbolStack.push_back({ 1,"Param" });
}
void process48()
{
	SyntaxTreeStack.pop_back();

}
void process49()
{
	SymbolStack.push_back({ 1,"ParamDecList" });
	SymbolStack.push_back({ 2,"SEMI" });
}
void process50()
{
	SymbolStack.push_back({ 1,"FormList" });
	SymbolStack.push_back({ 1,"TypeName" });
	currentP = newNode(DecK);
	currentP->attr.procAttr.paramt = valparamtype;
	TreeNode** t = SyntaxTreeStack.back();
	SyntaxTreeStack.pop_back();
	(*t) = currentP;
	SyntaxTreeStack.push_back(&(currentP->Sibling));

}
void process51()
{
	SymbolStack.push_back({ 1,"FormList" });
	SymbolStack.push_back({ 1,"TypeName" });
	SymbolStack.push_back({ 2,"VAR" });
	currentP = newNode(DecK);
	currentP->attr.procAttr.paramt = varparamtype;
	TreeNode** t = SyntaxTreeStack.back();
	SyntaxTreeStack.pop_back();
	(*t) = currentP;
	SyntaxTreeStack.push_back(&(currentP->Sibling));
}
void process52()
{
	SymbolStack.push_back({ 1,"FidMore" });
	SymbolStack.push_back({ 2,"ID" });
	strcpy(currentP->name[currentP->idnum], TokenList[token_pointer].Sem);
	currentP->idnum++;
}
void process53()
{

}
void process54()
{
	SymbolStack.push_back({ 1,"FormList" });
	SymbolStack.push_back({ 2,"COMMA" });
}
void process55()
{
	SymbolStack.push_back({ 1,"DeclarePart" });
}
void process56()
{
	SymbolStack.push_back({ 1,"ProgramBody" });
}
void process57()
{
	SymbolStack.push_back({ 2,"END" });
	SymbolStack.push_back({ 1,"StmList" });
	SymbolStack.push_back({ 2,"BEGIN" });
	SyntaxTreeStack.pop_back();
	currentP = newNode(StmLK);
	TreeNode** t = SyntaxTreeStack.back();
	SyntaxTreeStack.pop_back();
	(*t) = currentP;
	SyntaxTreeStack.push_back(&(currentP->child[0]));
}
void process58()
{
	SymbolStack.push_back({ 1,"StmMore" });
	SymbolStack.push_back({ 1,"Stm" });
}
void process59()
{
	SyntaxTreeStack.pop_back();
}
void process60()
{
	SymbolStack.push_back({ 1,"StmList" });
	SymbolStack.push_back({ 2,"SEMI" });
}
void process61()
{
	SymbolStack.push_back({ 1,"ConditionalStm" });
	currentP = newStmtNode(IfK);
	TreeNode** t = SyntaxTreeStack.back();
	SyntaxTreeStack.pop_back();
	(*t) = currentP;
	SyntaxTreeStack.push_back(&(currentP->Sibling));
}
void process62()
{
	SymbolStack.push_back({ 1,"LoopStm" });
	currentP = newStmtNode(WhileK);
	TreeNode** t = SyntaxTreeStack.back();
	SyntaxTreeStack.pop_back();
	(*t) = currentP;
	SyntaxTreeStack.push_back(&(currentP->Sibling));

}
void process63()
{
	SymbolStack.push_back({ 1,"InputStm" });
	currentP = newStmtNode(ReadK);
	TreeNode** t = SyntaxTreeStack.back();
	SyntaxTreeStack.pop_back();
	(*t) = currentP;
	SyntaxTreeStack.push_back(&(currentP->Sibling));
}
void process64()
{
	SymbolStack.push_back({ 1,"OutputStm" });
	currentP = newStmtNode(WriteK);
	TreeNode** t = SyntaxTreeStack.back();
	SyntaxTreeStack.pop_back();
	(*t) = currentP;
	SyntaxTreeStack.push_back(&(currentP->Sibling));
}
void process65()
{
	SymbolStack.push_back({ 1,"ReturnStm" });
	currentP = newStmtNode(ReturnK);
	TreeNode** t = SyntaxTreeStack.back();
	SyntaxTreeStack.pop_back();
	(*t) = currentP;
	SyntaxTreeStack.push_back(&(currentP->Sibling));
}
void process66()
{
	SymbolStack.push_back({ 1,"AssCall" });
	SymbolStack.push_back({ 2,"ID" });
	currentP = newStmtNode(AssignK);

	/*赋值语句左部变量节点*/
	TreeNode* t = newExpNode(VariK);
	strcpy(t->name[0], TokenList[token_pointer].Sem);
	t->idnum++;
	currentP->child[0] = t;

	TreeNode** t1 = SyntaxTreeStack.back();
	SyntaxTreeStack.pop_back();
	(*t1) = currentP;
	SyntaxTreeStack.push_back(&(currentP->Sibling));
}
void process67()
{
	SymbolStack.push_back({ 1,"AssignmentRest" });
	currentP->kind.stmt = AssignK;
}
void process68()
{
	SymbolStack.push_back({ 1,"CallStmRest" });
	currentP->child[0]->attr.ExpAttr.varkind = IdV;

	currentP->kind.stmt = CallK;
}
void process69()
{
	SymbolStack.push_back({ 1,"Exp" });
	SymbolStack.push_back({ 2,"ASSIGN" });
	SymbolStack.push_back({ 1,"VariMore" });
	SyntaxTreeStack.push_back(&(currentP->child[1]));
	currentP = currentP->child[0];
	TreeNode* t = newExpNode(OpK);
	t->attr.ExpAttr.op = END;
	OperatorStack.push_back(t);
}
void process70()
{
	SymbolStack.push_back({ 2,"FI" });
	SymbolStack.push_back({ 1,"StmList" });
	SymbolStack.push_back({ 2,"ELSE" });
	SymbolStack.push_back({ 1,"StmList" });
	SymbolStack.push_back({ 2,"THEN" });
	SymbolStack.push_back({ 1,"RelExp" });
	SymbolStack.push_back({ 2,"IF" });
	SyntaxTreeStack.push_back(&(currentP->child[2]));
	SyntaxTreeStack.push_back(&(currentP->child[1]));
	SyntaxTreeStack.push_back(&(currentP->child[0]));
}
void process71()
{
	SymbolStack.push_back({ 2,"ENDWH" });
	SymbolStack.push_back({ 1,"StmList" });
	SymbolStack.push_back({ 2,"DO" });
	SymbolStack.push_back({ 1,"RelExp" });
	SymbolStack.push_back({ 2,"WHILE" });
	SyntaxTreeStack.push_back(&(currentP->child[1]));
	SyntaxTreeStack.push_back(&(currentP->child[0]));
}
void process72()
{
	SymbolStack.push_back({ 2,"RPAREN" });
	SymbolStack.push_back({ 1,"InVar" });
	SymbolStack.push_back({ 2,"LPAREN" });
	SymbolStack.push_back({ 2,"READ" });
}
void process73()
{
	SymbolStack.push_back({ 2,"ID" });
	strcpy(currentP->name[0], TokenList[token_pointer].Sem);
	currentP->idnum++;
}
void process74()
{
	SymbolStack.push_back({ 2,"RPAREN" });
	SymbolStack.push_back({ 1,"Exp" });
	SymbolStack.push_back({ 2,"LPAREN" });
	SymbolStack.push_back({ 2,"WRITE" });
	SyntaxTreeStack.push_back(&(currentP->child[0]));
	TreeNode* t = newExpNode(OpK);
	t->attr.ExpAttr.op = END;
	OperatorStack.push_back(t);
}
void process75()
{
	SymbolStack.push_back({ 2,"RETURN" });
}
void process76()
{
	SymbolStack.push_back({ 2,"RPAREN" });
	SymbolStack.push_back({ 1,"ActParamList" });
	SymbolStack.push_back({ 2,"LPAREN" });
	SyntaxTreeStack.push_back(&(currentP->child[1]));
}
void process77()
{
	SyntaxTreeStack.pop_back();
}
void process78()
{
	SymbolStack.push_back({ 1,"ActParamMore" });
	SymbolStack.push_back({ 1,"Exp" });
	TreeNode* t = newExpNode(OpK);
	t->attr.ExpAttr.op = END;
	OperatorStack.push_back(t);
}
void process79()
{

}
void process80()
{
	SymbolStack.push_back({ 1,"ActParamList" });
	SymbolStack.push_back({ 2,"COMMA" });
	SyntaxTreeStack.push_back(&(currentP->Sibling));
}
void process81()
{
	SymbolStack.push_back({ 1,"OtherRelE" });
	SymbolStack.push_back({ 1,"Exp" });
	TreeNode* t = newExpNode(OpK);
	t->attr.ExpAttr.op = END;
	OperatorStack.push_back(t);
	getExpResult = 0;
}
void process82()
{
	SymbolStack.push_back({ 1,"Exp" });
	SymbolStack.push_back({ 1,"CmpOp" });
	TreeNode* currentP = newExpNode(OpK);
	currentP->attr.ExpAttr.op = TokenList[token_pointer].Lex;
	LexType sTop = OperandStack.back()->attr.ExpAttr.op;
	while (Priosity(sTop) >= Priosity(TokenList[token_pointer].Lex))
		/*如果操作符栈顶运算符的优先级高于或等于当前读到的操作符*/
	{
		TreeNode* t = OperatorStack.back();
		OperatorStack.pop_back();
		TreeNode* Rnum = OperandStack.back();
		OperandStack.pop_back();
		TreeNode* Lnum = OperandStack.back();
		OperandStack.pop_back();
		t->child[1] = Rnum;
		t->child[0] = Lnum;
		OperandStack.push_back(t);
		sTop = OperandStack.back()->attr.ExpAttr.op;
	}
	OperatorStack.push_back(currentP);
	getExpResult = 1;
}
void process83()
{
	SymbolStack.push_back({ 1,"OtherTerm" });
	SymbolStack.push_back({ 1,"Term" });
}
void process84()
{
	if ((TokenList[token_pointer].Lex == RPAREN) && (expflag != 0))
		//说明当前右括号是表达式中的一部分
	{

		while (OperatorStack.back()->attr.ExpAttr.op != LPAREN)
		{
			TreeNode* t = OperatorStack.back();
			OperatorStack.pop_back();
			TreeNode* Rnum = OperandStack.back();
			OperandStack.pop_back();
			TreeNode* Lnum = OperandStack.back();
			OperandStack.pop_back();

			t->child[1] = Rnum;
			t->child[0] = Lnum;
			OperandStack.push_back(t);
		}
		OperatorStack.pop_back(); //弹出左括号
		expflag--;
	}
	else
	{
		if ((getExpResult) || (getExpResult2))
		{
			while (OperatorStack.back()->attr.ExpAttr.op != END)
			{
				TreeNode* t = OperatorStack.back();
				OperatorStack.pop_back();
				TreeNode* Rnum = OperandStack.back();
				OperandStack.pop_back();
				TreeNode* Lnum = OperandStack.back();
				OperandStack.pop_back();

				t->child[1] = Rnum;
				t->child[0] = Lnum;
				OperandStack.push_back(t);
			}
			OperatorStack.pop_back();//弹出栈底标志
			currentP = OperandStack.back();
			OperandStack.pop_back();

			TreeNode** t = SyntaxTreeStack.back();
			SyntaxTreeStack.pop_back();
			(*t) = currentP;

			/*处理完数组变量，标志恢复初始值假，
			  遇到下一个数组下标表达式时，再将其设置为真值*/
			if (getExpResult2 == 1)
				getExpResult2 = 0;
		}
	}
}
void process85()
{

	SymbolStack.push_back({ 1,"Exp" });
	SymbolStack.push_back({ 1,"AddOp" });
	TreeNode* currentP = newExpNode(OpK);
	currentP->attr.ExpAttr.op = TokenList[token_pointer].Lex;
	LexType  sTop = OperatorStack.back()->attr.ExpAttr.op;
	while (Priosity(sTop) >= Priosity(TokenList[token_pointer].Lex))
		/*如果操作符栈顶运算符的优先级高于或等于当前读到的操作符*/
	{
		TreeNode* t = OperatorStack.back();
		OperatorStack.pop_back();
		TreeNode* Rnum = OperandStack.back();
		OperandStack.pop_back();
		TreeNode* Lnum = OperandStack.back();
		OperandStack.pop_back();

		t->child[1] = Rnum;
		t->child[0] = Lnum;
		OperandStack.push_back(t);
		sTop = OperatorStack.back()->attr.ExpAttr.op;
	}
	OperatorStack.push_back(currentP);
}
void process86()
{
	SymbolStack.push_back({ 1,"OtherFactor" });
	SymbolStack.push_back({ 1,"Factor" });
}
void process87()
{

}
void process88()
{
	SymbolStack.push_back({ 1,"Term" });
	SymbolStack.push_back({ 1,"MultOp" });
	TreeNode* currentP = newExpNode(OpK);
	currentP->attr.ExpAttr.op = TokenList[token_pointer].Lex;

	LexType  sTop = OperatorStack.back()->attr.ExpAttr.op;
	while (Priosity(sTop) >= Priosity(TokenList[token_pointer].Lex))
	{
		TreeNode* t = OperatorStack.back();
		OperatorStack.pop_back();
		TreeNode* Rnum = OperandStack.back();
		OperandStack.pop_back();
		TreeNode* Lnum = OperandStack.back();
		OperandStack.pop_back();
		t->child[1] = Rnum;
		t->child[0] = Lnum;
		OperandStack.push_back(t);

		sTop = OperatorStack.back()->attr.ExpAttr.op;
	}
	OperatorStack.push_back(currentP);
}
void process89()
{
	SymbolStack.push_back({ 2,"RPAREN" });
	SymbolStack.push_back({ 1,"Exp" });
	SymbolStack.push_back({ 2,"LPAREN" });
	TreeNode* t = newExpNode(OpK);
	t->attr.ExpAttr.op = TokenList[token_pointer].Lex;
	OperatorStack.push_back(t);
	expflag++;
}
void process90()
{
	SymbolStack.push_back({ 2,"INTC" });
	TreeNode* t = newExpNode(ConstK);
	//currentP = newExpNode(ConstK);
	t->attr.ExpAttr.val = stoi(TokenList[token_pointer].Sem);
	OperandStack.push_back(t);
}
void process91()
{
	SymbolStack.push_back({ 1,"Variable" });
}
void process92()
{
	SymbolStack.push_back({ 1,"VariMore" });
	SymbolStack.push_back({ 2,"ID" });
	currentP = newExpNode(VariK);
	strcpy(currentP->name[0], TokenList[token_pointer].Sem);
	currentP->idnum++;
	OperandStack.push_back(currentP);
}
void process93()
{
	currentP->attr.ExpAttr.varkind = IdV;
}
void process94()
{
	SymbolStack.push_back({ 2,"RMIDPAREN" });
	SymbolStack.push_back({ 1,"Exp" });
	SymbolStack.push_back({ 2,"LMIDPAREN" });
	currentP->attr.ExpAttr.varkind = ArrayMembV;
	SyntaxTreeStack.push_back(&(currentP->child[0]));
	TreeNode* t = newExpNode(OpK);
	t->attr.ExpAttr.op = END;
	OperatorStack.push_back(t);
	getExpResult2 = 1;
}
void process95()
{
	SymbolStack.push_back({ 1,"FieldVar" });
	SymbolStack.push_back({ 2,"DOT" });
	currentP->attr.ExpAttr.varkind = FieldMembV;
	SyntaxTreeStack.push_back(&(currentP->child[0]));
}
void process96()
{
	SymbolStack.push_back({ 1,"FieldVarMore" });
	SymbolStack.push_back({ 2,"ID" });
	currentP = newExpNode(VariK);
	strcpy(currentP->name[0], TokenList[token_pointer].Sem);
	currentP->idnum++;
	TreeNode** t = SyntaxTreeStack.back();
	SyntaxTreeStack.pop_back();
	(*t) = currentP;

}
void process97()
{
	currentP->attr.ExpAttr.varkind = IdV;
}
void process98()
{
	SymbolStack.push_back({ 2,"RMIDPAREN" });
	SymbolStack.push_back({ 1,"Exp" });
	SymbolStack.push_back({ 2,"LMIDPAREN" });
	currentP->attr.ExpAttr.varkind = ArrayMembV;
	SyntaxTreeStack.push_back(&(currentP->child[0]));
	TreeNode* t = newExpNode(OpK);
	t->attr.ExpAttr.op = END;
	OperatorStack.push_back(t);
	getExpResult2 = 1;
}
void process99()
{
	SymbolStack.push_back({ 2,"LT" });
}
void process100()
{
	SymbolStack.push_back({ 2,"EQ" });
}
void process101()
{
	SymbolStack.push_back({ 2,"PLUS" });
}
void process102()
{
	SymbolStack.push_back({ 2,"MINUS" });
}
void process103()
{
	SymbolStack.push_back({ 2,"TIMES" });
}
void process104()
{
	SymbolStack.push_back({ 2,"OVER" });
}
void process105()
{
	currentP = newStmtNode(ElseK);
	TreeNode** t = SyntaxTreeStack.back();
	SyntaxTreeStack.pop_back();
	(*t) = currentP;
	
	SyntaxTreeStack.push_back(&(currentP->Sibling));
}
void process106()
{
	currentP = newStmtNode(FiK);
	TreeNode** t = SyntaxTreeStack.back();
	SyntaxTreeStack.pop_back();
	(*t) = currentP;

	SyntaxTreeStack.push_back(&(currentP->Sibling));
}
void process107()
{
	currentP = newStmtNode(EndwhK);
	TreeNode** t = SyntaxTreeStack.back();
	SyntaxTreeStack.pop_back();
	(*t) = currentP;

	SyntaxTreeStack.push_back(&(currentP->Sibling));
}
void process108()
{
	currentP = newStmtNode(EndK);
	TreeNode** t = SyntaxTreeStack.back();
	SyntaxTreeStack.pop_back();
	(*t) = currentP;

	SyntaxTreeStack.push_back(&(currentP->Sibling));
}
void process109()
{
	SymbolStack.push_back({ 2,"CHARC" });
	TreeNode* t = newExpNode(ConstK);
	//currentP = newExpNode(ConstK);
	t->attr.ExpAttr.valc =TokenList[token_pointer].Sem;
	OperandStack.push_back(t);
}
TreeNode* LL1() {
	TreeNode* root = praseLL();
	if (root != nullptr)
	{
		printf("打印语法树：\n");
		showtree(root);
	}

	if (error == 0)
	{
		printf("\n没有发生语法错误\n");
	}
	else {
		exit(0);
	}
	return root;
}