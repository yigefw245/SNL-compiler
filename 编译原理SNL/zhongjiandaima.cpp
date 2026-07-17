#include"zhongjiandaima.h"
#include"yuyi.h"
#include<stack>
vector<string>t;
stack<int>en;//函数和过程栈
int temp = 0;
vector<string>offset;
int tempoff = 0;
//数组
string arr(TreeNode* node, vector<Quadruple>& quadruples) {
	string op;
	string arg1;
	string arg2;
	string result;
	op = "[+]";
	arg1 = node->name[0];
	arg2 = biao(node->child[0],quadruples);
	result = "t" + to_string(temp);
	t.push_back(result);
	temp++;
	quadruples.push_back(Quadruple(op, arg1, arg2, result));
	return result;
}
//表达式运算
string biao(TreeNode* node, vector<Quadruple>& quadruples) {
	if (node == NULL)
		return " ";
	vector<string>shizi;
	LexType op0 = node->attr.ExpAttr.op;
	string op;
	string arg1;
	string arg2;
	string result;
	if (node->kind.exp == OpK) {
		if (op0 == LexType::PLUS)
			op = "+";
		if (op0 == LexType::MINUS)
			op = "-";
		if (op0 == LexType::TIMES)
			op = "*";
		if (op0 == LexType::OVER)
			op = "/";
		if (op0 == LexType::LT)
			op = "<";
		if (op0 == LexType::EQ)
			op = "=";
		arg1=biao(node->child[0], quadruples);
		arg2 = biao(node->child[1], quadruples);
		result = "t" + to_string(temp);//临时变量值编号
		t.push_back(result);
		temp++;
		quadruples.push_back(Quadruple(op, arg1, arg2, result));
		return result;
	}
	else {
		if(node->kind.exp==ConstK)
			if(node->attr.ExpAttr.valc=="")
		result =to_string(node->attr.ExpAttr.val);
			else {
				result = "\'";
				result+=node->attr.ExpAttr.valc;
				result += "\'";
			}
		if (node->kind.exp == VariK) {
			if(node->attr.ExpAttr.varkind==IdV)
			result = node->name[0];
			if (node->attr.ExpAttr.varkind == ArrayMembV)
				result = arr(node,quadruples);
		}
		return result;
	}
	return " ";
}
string result0;
string fun=" ";
void generateQuadruples(TreeNode* node, vector<Quadruple>& quadruples) {
	if (node == NULL) return;
	string arg1;
	string arg2;
	string result;
	
		if (node->nodekind==StmtK&&node->kind.stmt == AssignK) {
			string op = "=";
			result = biao(node->child[0],quadruples);
			arg2 = " ";
			arg1 = biao(node->child[1], quadruples);  
			quadruples.push_back(Quadruple(op, arg1,  arg2, result));
		}
		if (node->nodekind == StmtK && node->kind.stmt == IfK) {
			string op = "THEN";
			arg1 = biao(node->child[0],quadruples);
			arg2 = " ";
			result = " ";
			quadruples.push_back(Quadruple(op, arg1, arg2, result));
		}
		if (node->nodekind == StmtK && node->kind.stmt == ElseK) {
			string op = "ELSE";
			arg1 = " ";
			arg2 = " ";
			result = " ";
			quadruples.push_back(Quadruple(op, arg1, arg2, result));
		}
		if (node->nodekind == StmtK && node->kind.stmt == FiK) {
			string op = "ENDIF";
			arg1 = " ";
			arg2 = " ";
			result = " ";
			quadruples.push_back(Quadruple(op, arg1, arg2, result));
		}
		if (node->nodekind == StmtK && node->kind.stmt == WhileK) {
			string op = "WHILE";
			arg1 = " ";
			arg2 = " ";                                                                                 
			result = " ";
			quadruples.push_back(Quadruple(op, arg1, arg2, result));
			string flag = biao(node->child[0], quadruples);
			quadruples.push_back(Quadruple("do", flag, arg2, result));
		}
		if (node->nodekind == StmtK && node->kind.stmt == EndwhK) {
			
			string op = "ENDWH";
			arg1 = " ";
			arg2 = " ";
			result = " ";
			quadruples.push_back(Quadruple(op, arg1, arg2, result));
		}
		if (node->nodekind == StmtK && node->kind.stmt == ReadK) {
			string op = "READ";
			arg1 = " ";
			arg2 = " ";
			result = node->name[0];
			quadruples.push_back(Quadruple(op, arg1, arg2, result));
		}
		if (node->nodekind == StmtK && node->kind.stmt == WriteK) {
			string op = "WRITE";
			arg1 = " ";
			arg2 = " ";
			result = biao(node->child[0],quadruples);
			quadruples.push_back(Quadruple(op, arg1, arg2, result));
		}
		if (node->nodekind == StmtK && node->kind.stmt == CallK){
			string op = "CALL";
			arg1 = node->child[0]->name[0];
			arg2 = "true";
			for (int i = 1; i < 3; i++) {
				string tt=biao(node->child[i], quadruples);
				if (tt != " ") {
					string op0 = "VarACT";
					string arg01 = tt;
					string arg02;
					int k = i - 1;
					arg02 += to_string(k);		//offset
					string result0;		//size
					result0 = "1";	
					quadruples.push_back(Quadruple(op0, arg01, arg02, result0));
				}
			}   
			string result = "t" + to_string(temp);
			t.push_back(result);
			temp++;
			quadruples.push_back(Quadruple(op, arg1, arg2, result));
			tempoff++;
		}
		if (node->nodekind == ProcDecK) {
			string name = node->name[0];
			string Label = "Label" +name;
			string op = "ENTRY";
			en.push(1);
			int size = 0;
			for (int i = 0; i < 3; i++) {
				if (node->child[i]->nodekind == DecK) {
					for (int j = 0; j < 10; j++) {
						if (!(strcmp(node->child[i]->name[j],"")==0)) {
							size++;
						}
					}
				}
			}
			string size0 = to_string(size);
			int l= node->table[0]->attrIR.More.ProcAttr.level;
			string level = to_string(l);
			quadruples.push_back(Quadruple(op, Label, size0, level));
		}
		if (node->nodekind == StmtK && node->kind.stmt == EndK) {
			string op = "END";
			if (en.size()!=0)
				op = "ENDFUC";
			arg1 = " ";
			arg2 = " ";
			result = " ";
			quadruples.push_back(Quadruple(op, arg1, arg2, result));
			if (en.size() != 0) {
				en.pop();
			}
		}
		
	
	// 递归处理子节点
	for (int i = 0; i < 3; i++) {
		generateQuadruples(node->child[i], quadruples);
	}
	generateQuadruples(node->Sibling, quadruples);
}
