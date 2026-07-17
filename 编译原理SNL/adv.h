#pragma once
#include<fstream>
#include"string"
#include<iostream>
using namespace std;
typedef enum
{
	/* 簿记单词符号 */
	ENDFILE1, ERROR1,
	/* 保留字 */
	PROGRAM, PROCEDURE, TYPE, VAR, IF,
	THEN, ELSE, FI, WHILE, DO,
	ENDWH, BEGIN, END, READ, WRITE,
	ARRAY, OF, RECORD, RETURN,

	INTEGER, CHAR1,
	/* 多字符单词符号 */
	ID, INTC, CHARC,
	/*特殊符号 */
	ASSIGN, EQ,LT, PLUS, MINUS,
	TIMES, OVER, LPAREN, RPAREN, DOT,
	COLON, SEMI, COMMA, LMIDPAREN, RMIDPAREN,
	UNDERANGE
} LexType;

//保留字查找表：
typedef struct
{
	const char* str;
	LexType tok;
} ReservedTable;

//初始化保留字查找表：
static const ReservedTable reservedWords[21]
= {
{"program",PROGRAM},{"type",TYPE},{"var",VAR},
{"procedure",PROCEDURE},{"begin",BEGIN},{"end",END},
{"array",ARRAY},{"of",OF},{"record",RECORD},{"if",IF},{"then",THEN},
{"else",ELSE},{"fi",FI},{"while",WHILE},{"do",DO},{"endwh",ENDWH},
{"read",READ},{"write",WRITE},{"return",RETURN},
{"integer",INTEGER},{"char",CHAR1}
};
typedef struct
{
	int Lineshow;
	LexType Lex;
	char* Sem;
}Token;
#pragma once
