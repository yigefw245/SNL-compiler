#pragma once
#include<vector>
#include<fstream>
#include"yufa.h"
#include <iomanip>
#include"yuyi.h"
#include"zhongjiandaima.h"
const char* typeall[] =
{
	"ENDFILE1", "ERROR1",
	/* 保留字 */
	"PROGRAM", "PROCEDURE", "TYPE", "VAR", "IF",
	"THEN", "ELSE", "FI", "WHILE", "DO",
	"ENDWH", "BEGIN", "END", "READ", "WRITE",
	"ARRAY", "OF", "RECORD", "RETURN",
	//类型
	"INTEGER", "CHAR1",
	/* 多字符单词符号 */
	"ID", "INTC", "CHARC",
	/*特殊符号 */
	"ASSIGN", "EQ", "LT", "PLUS", "MINUS",
	"TIMES", "OVER", "LPAREN", "RPAREN", "DOT",
	"COLON", "SEMI", "COMMA", "LMIDPAREN", "RMIDPAREN",
	"UNDERANGE"
};

int CIFA();
extern vector<TokenType>TokenList;
void cifa_fenxi()
{
	cout << "即将开始词法分析：" << endl << endl;
	if (CIFA())
	{
		printf("词法分析完成：\n发生词法错误\n");
		exit(0);
	}
	printf("词法分析完成：\n无词法错误\n\n开始语法分析：\n");
	getTokenList();
	cout << "该代码一共有" << TokenList.size() << "个token，如下：" << endl;
	for (int i = 0; i < TokenList.size(); i++) {
		cout << "|" << setw(3) << TokenList[i].lineshow << " | "
			<< setw(10) << typeall[TokenList[i].Lex] << " | "
			<< setw(10) << TokenList[i].Sem << " | " << endl;
	}
	printf("\n");
}
TreeNode* treeroot;
TreeNode* yufa_fenxi()
{
	return treeroot = LL1();
}
void yuyi_fenxi()
{
	Analyze(treeroot);
}
void generateQuadruples(TreeNode* node, vector<Quadruple>& quadruples);
vector<Quadruple> gencode(TreeNode* treeroot)
{
	FILE* fp = fopen("gencode.txt", "w");
	vector<Quadruple> quadruples;
	TreeNode* node = treeroot;
	generateQuadruples(node,quadruples);
	cout << endl;
	cout << "中间代码:" << endl;
	for (int i = 0; i < quadruples.size(); i++) {
		fprintf(fp, "%s,%s,%s,%s\n", quadruples[i].op.c_str(), quadruples[i].arg1.c_str(), quadruples[i].arg2.c_str(), quadruples[i].result.c_str());
		cout <<"(" << quadruples[i].op << "," << quadruples[i].arg1 << "," << quadruples[i].arg2 << "," << quadruples[i].result<<")" << endl;
	}
	return quadruples;
}
void showyouhua() {
	TreeNode* node = treeroot;
	int i = 0;
	fstream myfile("genyouhua.txt", ios::in | ios::out);
	string line;
	if (myfile.fail()) {
		cerr << "error oprening file myname!" << endl;
		exit(-1);
	}
	while (getline(myfile, line))
		cout <<"(" << line <<")" << endl;
}