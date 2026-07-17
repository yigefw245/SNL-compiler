#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include"RD.h"
// 使用之前定义的LexType和Token结构

// 使用示例
TreeNode0* yufafenxi(vector<TokenType> tokens) {
    // 假设已经通过词法分析得到tokens
    //std::vector<Token> tokens;
    //cifafenxi(tokens);
    Parser parser(tokens);
    TreeNode0* syntaxTree = parser.parseProgram();

    TreePrinter show;
    cout << "!" << endl;
    show.print(syntaxTree);
    return syntaxTree;
}