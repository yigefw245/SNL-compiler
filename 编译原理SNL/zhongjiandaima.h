#pragma once
#include<vector>
#include<unordered_map>
#include<stack>
#include"yufa.h"
#include<string>
struct Quadruple {
	string op;
	string arg1;
	string arg2;
	string result;

	Quadruple(string o, string a1, string a2, string r)
		: op(o), arg1(a1), arg2(a2), result(r) {}
};
//求数组目标代码
string biao(TreeNode* node, vector<Quadruple>& quadruples);
string arr(TreeNode* node, vector<Quadruple>& quadruples);
vector<Quadruple> gencode(TreeNode* treeroot);