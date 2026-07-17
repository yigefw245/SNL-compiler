#pragma once
#include"yufa.h"//符号表啥的定义都在语法的头文件里
#include <iomanip>
typedef struct //标识符的属性结构定义，抄书上的
{
	struct typeIR* idtype;	//指向标识符的类型内部表示
	IdKind kind;			//标识符的类型
	union
	{
		struct
		{
			AccessKind access;   //判断是变参还是值参
			int level;
			int off;
			bool isParam;  //判断是参数还是普通变量
		}VarAttr;//变量标识符的属性
		struct
		{
			int level;     //该过程的层数
			ParamTable* param;   //参数表
			int mOff;	   //过程活动记录的大小
			int nOff;  	   //sp到display表的偏移量
			int procEntry; //过程的入口地址
			int codeEntry;//过程入口标号,用于中间代码生成
		}ProcAttr;//过程名标识符的属性
	}More;//标识符的不同类型有不同的属性
}AttributeIR;//标识符信息项
enum TypeKind { intTy, charTy, arrayTy, recordTy, boolTy };
typedef struct FieldChain
{
	char idname[10];
	struct typeIR* unitType;
	int offset;
	struct FieldChain* next;
}FieldChain;//域链

typedef struct symbtable
{
	char idname[10];
	AttributeIR attrIR;
	struct symbtable* next;
}SymbTable;//符号表
typedef struct typeIR//书上抄的
{
	int size;
	TypeKind kind;
	union
	{
		struct
		{
			struct typeIR* indexTy;
			struct typeIR* elemTy;
			int low;
			int up;
		}ArrayAttr;
		FieldChain* body;
	}More;
}TypeIR;//intTy,charTy,arrayTy,recordTy,boolTy五个类型的内部表示
//符号表管理
void CreateTable();//创建一个符号表
void DestroyTable();//删除一个符号表
bool Enter(char* Id, AttributeIR* AttribP, SymbTable** Entry);//登记标记符和属性到符号表
bool FindEntry(char* id, SymbTable** Entry);//符号表中查找标记符
bool FindField(char* Id, FieldChain* head, FieldChain** Entry);//域表中查找域名函数
void PrintSymbTable();//打印符号表

//语义分析
void Analyze(TreeNode* currentP);//主函数
void initialize();//初始化函数
TypeIR* TypeProcess(TreeNode* t, Dec deckind);//类型分析总函数
TypeIR* nameType(TreeNode* t);//自定义类型内部结构分析函数
TypeIR* arrayType(TreeNode* t);//数组类型内部表示处理函数
TypeIR* recordType(TreeNode* t);//记录类型内部表示处理函数


void TypeDecPart(TreeNode* t);//类型声明处理
void VarDecPart(TreeNode* t);//变量声明处理
void ProcDecPart(TreeNode* t);//过程声明处理
SymbTable* HeadProcess(TreeNode* t);//过程头处理
ParamTable* ParaDecList(TreeNode* t);//形参处理
void statement(TreeNode* t);//语句处理总函数
TypeIR* Expr(TreeNode* t, AccessKind* Ekind);//表达式处理
TypeIR* arrayVar(TreeNode* t);//数组变量处理函数
TypeIR* recordVar(TreeNode* t);//记录类型中的域变量分析处理

void assignstatemnet(TreeNode* t);//赋值语句处理
void callstatement(TreeNode* t);//函数调用语句处理
void ifstatment(TreeNode* t);//if语句处理
void whilestatement(TreeNode* t);//while循环语句处理
void readstatement(TreeNode* t);//read语句处理
void writestatement(TreeNode* t);//write语句处理
void elsestatement(TreeNode* t);
void fistatement(TreeNode* t);
void endwhstatement(TreeNode* t);
void endstatement(TreeNode* t);


void returnstatement(TreeNode* t);//return语句处理
bool Compat(TypeIR* tp1, TypeIR* tp2);//判断类型是否相容
TypeIR* NewTy(TypeKind  kind);//创建当前空类型内部表示
AttributeIR FindAttr(SymbTable* entry);//属性查询
