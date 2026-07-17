#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <sstream>
#include"zhongjiandaima.h"
#include"mubiao.h"
using namespace std;

void mubiao(vector<Quadruple> quadruples) {
    MIPSGenerator generator;
    // 示例：计算 if (a + b * 2 < 10) then result = 1 else result = 0
    // 生成MIPS代码
    for (int i = 0; i < quadruples.size(); i++) {
        generator.addQuadruple(quadruples[i]);
    }

    generator.generateMIPS();
    return;
}