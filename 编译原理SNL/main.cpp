#include <iostream>
#include"adv.h"
#include"yuyi.h"
#include"pre.h"
#include"zhongjiandaima.h"
#include"mubiao.h"
#include"RD.h"
int main()
{
	cifa_fenxi();
    yufa_fenxi();
	cout << "RD语法树:" << endl;
	yufafenxi(TokenList);
	cout << "\n开始语义分析：\n";
	yuyi_fenxi();
	vector<Quadruple> quadruples=gencode(treeroot);
	cout << "\n\n\n\n";
	//cout << "中间代码优化:\n" << endl;
	//showyouhua();
	cout << "\n\n\n\n";
	cout << "MIPS目标代码:\n" << endl;
	mubiao(quadruples);
	return 0;
}