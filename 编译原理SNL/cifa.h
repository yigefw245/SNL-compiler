#pragma once
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include <cstring>
#include"adv.h"
using namespace std;

/*保留字数量常量MAXRESERVED为21 */
#define MAXRESERVED 21

/*源代码行的输入缓冲区长度为256 */
#define BUFLEN 256

char lineBuf[BUFLEN];//lineBuf为当前输入代码行缓冲区
int linepos;//linepos为在代码缓冲区LineBuf中的当前字符位置,初始为0
int bufsize;// bufsize为当前缓冲器中所存字串大小

FILE* source;
FILE* listing; 
int EOF_flag;//EOF_flag当为文件尾时,改变函数ungetNextChar功能

int Error; 
int lineno1;  //行号

#define MAXTOKENLEN 40

char tokenString[MAXTOKENLEN + 1];//tokenString用于保存标识符和保留字单词的词元,长度41

/*定义token的长度*/
#define TOKENLEN sizeof (Token1)//后面写文件要用
/*定义链表节点的长度*/
#define CHAINNODELEN sizeof (ChainNodeType) 


typedef struct tokenType
{
	int     lineshow;   //记录该单词在源程序中的行数 
	LexType Lex;       //记录该单词的词法信息 
	char    Sem[MAXTOKENLEN + 1];   //记录该单词的语义信息 
}Token1;

typedef struct node
{
	Token1   Token;      //单词
	struct node* nextToken; //指向下一个单词的指针
}ChainNodeType;

//词法分析器确定性有限自动机DFA的状态类型
typedef enum
{
	START, INASSIGN, INRANGE, INCOMMENT, INNUM, INID, INCHAR, DONE
	/*START 开始状态; INASSIGN 赋值状态; INRANGE 下标范围状态;*/
	/* INNUM 数字状态; INID 标识符状态; DONE 完成状态;*/
	/* INCHAR 字符状态;INCOMMENT 注释状态; */
}StateType;
int CIFA();