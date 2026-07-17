#pragma once
#include"adv.h"

using namespace std;
typedef struct  paramTable //形参表的结构定义
{
	struct symbtable* entry; //指向该形参所在符号表中的地址入口
	struct paramTable* next;
}ParamTable;
typedef pair<int, string>token_single;
typedef struct
{
	int     lineshow;
	LexType Lex;
	char Sem[41];
} TokenType;
typedef  enum { typeKind, varKind, procKind }IdKind; //标识符的类型
typedef  enum { dir, indir }AccessKind; //变量的类别。dir表直接变量(值参)，indir表示间接变量(变参)
enum Stmt { IfK,ElseK,FiK, WhileK,EndwhK, AssignK, ReadK, WriteK, CallK, ReturnK,EndK };  //kind成员，为语法树节点语句类型  /*StmtKind*/
enum Expk { OpK, ConstK, VariK };    //kind成员  语法树节点表达式类型  /*StmtKind*/
enum Paramt { valparamtype, varparamtype };   //procAttr成员  参数类型  /*ParamType*/
enum Varkind { IdV, ArrayMembV, FieldMembV };//ExpAttr成员，变量的类别  /*VarKind*/
enum Type { Void, Integer, Boolean };   //ExpAttr成员，记录语法树节点的检查类别，取值Void,Integer,Boolean  /*ExpType*/
enum Dec { ArrayK, CharK, IntegerK, RecordK, IdK };  //kind成员 语法树节点声明类型 /*DecKind*/
enum NodeKind { ProK, PheadK, TypeK, VarK, ProcDecK, StmLK, DecK, StmtK, ExpK }; //记录语法树节点类型

typedef struct TreeNode
{
	TreeNode* child[3];//指向子语法树节点指针
	TreeNode* Sibling;//指向兄弟语法树节点指针
	int lineno;//记录源程序行号，为整数类型
	NodeKind nodekind;//记录语法树节点类型，取值prok,pheadk,typek,vark,procdeck,stmlk,deck,stmtk,expk
	union
	{
		Dec dec;//记录语法树节点的声明类型，当nodeKind=deck时有效，取值arrayk,chark,integerk,recordk,idk
		Stmt stmt;//记录语法树节点的语句类型，当nodekind=stmtk时有效，取值ifk,whitek,assignk,readk,writek,callk,returnk
		Expk exp;//记录语法树节点的语句类型，当nodekind=expk时有效，取值opk,constk,idk
	}kind;
	int idnum;//记录一个节点中的标志符的个数
	char name[10][10];//节点中的标志符的名字
	struct symbtable* table[10];//节点中的各个标志符在符号表中的入口
	char type_name[10];//记录类型名，当节点为声明节点且类型是由类型标志符表示时有效
	struct
	{
		struct
		{
			int low;//数组下界
			int up;//数组上界
			Dec childType;//数组的成员类型
		}ArrayAttr;//数组属性
		struct
		{
			Paramt paramt;//记录过程的参数类型，值为valparamtype或者是varparamtype表示过程的参数是值参还是变参
		}procAttr;//过程属性
		struct
		{
			LexType op;//记录运算符单词，取值lt,eq,plus,minus,times,over
			int val;//记录数值
			string valc;
			Varkind varkind;//记录变量类别，取值idv,arraymembv,fieldmembv分别表示变量是标志符变量、数组成员变量还是域成员变量
			Type type;//记录语法树节点的检查类型，取值void,integer,boolean
		}ExpAttr; //记录表达式属性
	}attr;//记录语法树节点其他属性
}TreeNode;
void CreateLL1Table();
TreeNode* praseLL();
void predict(int num);
int Priosity(LexType op);
void process1();
void process2();
void process3();
void process4();
void process5();
void process6();
void process7();
void process8();
void process9();
void process10();
void process11();
void process12();
void process13();
void process14();
void process15();
void process16();
void process17();
void process18();
void process19();
void process20();
void process21();
void process22();
void process23();
void process24();
void process25();
void process26();
void process27();
void process28();
void process29();
void process30();
void process31();
void process32();
void process33();
void process34();
void process35();
void process36();
void process37();
void process38();
void process39();
void process40();
void process41();
void process42();
void process43();
void process44();
void process45();
void process46();
void process47();
void process48();
void process49();
void process50();
void process51();
void process52();
void process53();
void process54();
void process55();
void process56();
void process57();
void process58();
void process59();
void process60();
void process61();
void process62();
void process63();
void process64();
void process65();
void process66();
void process67();
void process68();
void process69();
void process70();
void process71();
void process72();
void process73();
void process74();
void process75();
void process76();
void process77();
void process78();
void process79();
void process80();
void process81();
void process82();
void process83();
void process84();
void process85();
void process86();
void process87();
void process88();
void process89();
void process90();
void process91();
void process92();
void process93();
void process94();
void process95();
void process96();
void process97();
void process98();
void process99();
void process100();
void process101();
void process102();
void process103();
void process104();
void process105();
void process106();
void process107();
void process108();
void process109();
void showtree(TreeNode* tree);
TreeNode* LL1();
void getTokenList();
