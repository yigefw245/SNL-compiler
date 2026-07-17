#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include"yufa.h"
#include"adv.h"
// 语法树节点定义
struct TreeNode0 {
    enum NodeType {
        PROGRAM, PROCEDURE_DECL, VAR_DECL, TYPE_DECL,
        ASSIGN_STMT, IF_STMT, WHILE_STMT, READ_STMT,
        WRITE_STMT, RETURN_STMT, COMPOUND_STMT,
        EXPRESSION, IDENTIFIER, CONSTANT, OPERATOR,
        ARRAY_TYPE, RECORD_TYPE, FIELD_DECL, TYPE_DEF, ARRAY_RANGE, BASE_TYPE, TYPE_REF,
        VAR_LIST, INTEGER, CHAR1, PARAM_DECL, PARAM_LIST, INTC, CHARC
    } type;

    std::string value;        // 节点值（标识符名称、常量值等）
    LexType op;                // 运算符类型
    std::vector<TreeNode0*> children; // 子节点

    TreeNode0(NodeType t, const std::string& v = "")
        : type(t), value(v), op(ERROR1) {}

    ~TreeNode0() {
        for (auto child : children) delete child;
    }
    std::string typeToString() const {
        static const char* names[] = {
            "PROGRAM", "PROCEDURE_DECL", "VAR_DECL", "TYPE_DECL",
            "ASSIGN_STMT", "IF_STMT", "WHILE_STMT", "READ_STMT",
            "WRITE_STMT", "RETURN_STMT", "COMPOUND_STMT",
            "EXPRESSION", "IDENTIFIER", "CONSTANT", "OPERATOR",
            "ARRAY_TYPE", "RECORD_TYPE", "FIELD_DECL",
            "TYPE_DEF", "ARRAY_RANGE", "BASE_TYPE", "TYPE_REF",
            "VAR_LIST","INTEGER","CHAR1","PARAM_DECL","PARAM_LIST","INTC","CHARC"
        };
        return names[type];
    }

    // 添加节点信息摘要
    std::string summary() const {
        std::string info = typeToString();
        if (!value.empty()) info += " [" + value + "]";
        if (op != ERROR1) info += " <" + tokenToString(op) + ">";
        return info;
    }
private:
    // 辅助函数：LexType到字符串转换
    static std::string tokenToString(LexType t) {
        // 需要根据LexType定义实现具体转换逻辑
        return std::to_string(static_cast<int>(t));
    }
};

// 语法树打印类
class TreePrinter {
public:
    static void print(const TreeNode0* node,
        std::ostream& os = std::cout,
        int indent = 0) {
        printNode(node, os, indent, "", true);
    }

private:
    static void printNode(const TreeNode0* node,
        std::ostream& os,
        int indent,
        const std::string& prefix,
        bool isLast) {
        // 打印当前节点
        os << prefix;
        os << (isLast ? "└── " : "├── ");
        os << node->summary();
        if (!node->typeToString().empty()) os << " : " << node->typeToString();
        os << "\n";

        // 递归打印子节点
        const std::string newPrefix = prefix + (isLast ? "    " : "│   ");
        const size_t childCount = node->children.size();

        for (size_t i = 0; i < childCount; ++i) {
            const bool lastChild = (i == childCount - 1);
            printNode(node->children[i], os, indent + 1, newPrefix, lastChild);
        }
    }
};


class Parser {
private:
    std::vector<TokenType> tokens;
    size_t pos = 0;
    TokenType current;

    // 辅助函数
    void advance() {
        if (pos < tokens.size()) current = tokens[pos++];
    }

    bool match(LexType expected) {
        if (current.Lex == expected) {
            advance();
            return true;
        }
        return false;
    }

    void error(const std::string& msg) {
        std::cerr << "Syntax error at position " << current.lineshow - 1
            << ": " << msg << std::endl;
        exit(1);
    }

public:
    Parser(const std::vector<TokenType>& t) : tokens(t) {
        advance();
    }

    // 程序解析入口
    TreeNode0* parseProgram() {
        auto node = new TreeNode0(TreeNode0::PROGRAM);

        if (!match(PROGRAM)) error("Missing PROGRAM");
        if (current.Lex == ID) node->value = current.Sem;
        else error("Missing program name");
        advance();

        // 解析声明部分
        while (current.Lex == TYPE || current.Lex == VAR
            || current.Lex == PROCEDURE) {
            node->children.push_back(parseDeclaration());
        }

        if (!match(BEGIN)) error("Missing BEGIN");
        node->children.push_back(parseStatementList());
        if (!match(END)) error("Missing END");
        if (!match(DOT)) error("Missing .");

        return node;
    }

private:
    // 声明解析
    TreeNode0* parseDeclaration() {
        if (current.Lex == TYPE) return parseTypeDecl();
        if (current.Lex == VAR) return parseVarDecl();
        if (current.Lex == PROCEDURE) return parseProcDecl();
        error("Invalid declaration");
        return nullptr;
    }

    // 变量声明
    TreeNode0* parseVarDecl() {
        auto node = new TreeNode0(TreeNode0::VAR_DECL);
        match(VAR); // 跳过VAR
        do {
            auto varkind = parseType();
            node->children.push_back(varkind);//自己改的,原来在下面
            // 变量名列表
           // auto varList = new TreeNode(TreeNode::IDENTIFIER);
            while (current.Lex == ID) {
                varkind->children.push_back(
                    new TreeNode0(TreeNode0::IDENTIFIER, current.Sem));
                advance();
                if (!match(COMMA)) break;
            }

            match(COMMA);//跳过逗号
        } while (!match(SEMI));

        if (current.Lex == INTEGER || current.Lex == CHAR1 || current.Lex == ARRAY) {
            auto varkind = parseVarDecl();
            node->children.insert(node->children.end(), varkind->children.begin(), varkind->children.end());
        }
        return node;
    }

    // 类型解析
    TreeNode0* parseType() {
        if (current.Lex == ARRAY) {
            auto arrayNode = new TreeNode0(TreeNode0::ARRAY_TYPE);
            match(ARRAY); // 消费 ARRAY 关键字

            // 解析数组维度
            if (!match(LMIDPAREN)) {
                error("Missing '[' for array dimension");
                return arrayNode;
            }

            // 解析数组范围（下界..上界）
            do {
                auto rangeNode = parseArrayRange();
                arrayNode->children.push_back(rangeNode);

                // 支持多维数组
                if (!match(COMMA)) break;
            } while (true);

            if (!match(RMIDPAREN)) {
                error("Missing ']' for array dimension");
                return arrayNode;
            }

            // 解析数组元素类型
            if (!match(OF)) {
                error("Missing 'OF' in array declaration");
                return arrayNode;
            }

            // 解析元素类型（可以是基础类型、数组类型或记录类型）
            auto elementTypeNode = parseType();
            arrayNode->children.push_back(elementTypeNode);

            return arrayNode;
        }
        if (current.Lex == RECORD) {
            auto node = new TreeNode0(TreeNode0::RECORD_TYPE);
            match(RECORD);
            // 解析字段列表
            if (current.Lex == ID) {
                auto ID0 = new TreeNode0(TreeNode0::IDENTIFIER);
                match(ID);
                if (!match(BEGIN))error("Missing: begin");
                ID0->children.push_back(parseFieldDecl());
            }
            if (!match(END))error("Missing end");
            return node;
        }
        // 基本类型
        if (current.Lex == INTEGER) {
            auto node = new TreeNode0(TreeNode0::INTEGER, current.Sem);
            advance();
            return node;
        }
        if (current.Lex == CHAR1) {
            auto node = new TreeNode0(TreeNode0::CHAR1, current.Sem);
            advance();
            return node;
        }
        error("Invalid type");
        return nullptr;
    }

    // 语句列表解析
    TreeNode0* parseStatementList() {
        auto node = new TreeNode0(TreeNode0::COMPOUND_STMT);
        while (current.Lex != END && current.Lex != ENDWH
            && current.Lex != FI && current.Lex != ELSE) {
            node->children.push_back(parseStatement());
            if (!match(SEMI)) error("Missing ;");
        }
        return node;
    }

    // 语句解析
    TreeNode0* parseStatement() {
        switch (current.Lex) {
        case ID: return parseAssignStmt();
        case IF: return parseIfStmt();
        case WHILE: return parseWhileStmt();
        case READ: return parseReadStmt();
        case WRITE: return parseWriteStmt();
        case RETURN: return parseReturnStmt();
        case BEGIN: return parseCompoundStmt();
        default: error("Invalid statement");
        }
        return nullptr;
    }

    // 赋值或者函数语句
    TreeNode0* parseAssignStmt() {
        auto node = new TreeNode0(TreeNode0::ASSIGN_STMT);
        node->children.push_back(parseVariable());
        int f = 0;
        if (match(LPAREN))
            f = 1;
        if (f == 0 && !match(ASSIGN)) error("Missing := ");
        do {
            node->children.push_back(parseExpression());
        } while (match(COMMA));
        if (f == 1 && !match(RPAREN))error("Missing )");
        return node;
    }

    // 条件表达式
    TreeNode0* parseCondition() {
        auto node = parseExpression();
        if (current.Lex == LT || current.Lex == EQ) {
            auto opNode = new TreeNode0(TreeNode0::OPERATOR);
            opNode->op = current.Lex;
            advance();
            opNode->children.push_back(node);
            opNode->children.push_back(parseExpression());
            return opNode;
        }
        return node;
    }
    
    TreeNode0* parseExpression() {
        auto node = parseTerm();
        while (current.Lex == PLUS || current.Lex == MINUS||current.Lex==TIMES||current.Lex==OVER) {
            auto opNode = new TreeNode0(TreeNode0::OPERATOR);
            opNode->op = current.Lex;
            advance();
            opNode->children.push_back(node);
            opNode->children.push_back(parseTerm());
            node = opNode;
        }
        return node;
    }

    TreeNode0* parseTerm() {
        auto node = parseFactor();
        while (current.Lex == TIMES || current.Lex == OVER) {
            auto opNode = new TreeNode0(TreeNode0::OPERATOR);
            opNode->op = current.Lex;
            advance();
            opNode->children.push_back(node);
            opNode->children.push_back(parseFactor());
            node = opNode;
        }
        return node;
    }

    TreeNode0* parseFactor() {
        if (match(LPAREN)) {
            auto node = parseExpression();
            if (!match(RPAREN)) error("Missing )");
            return node;
        }
        if (current.Lex == INTC) {
            auto node = new TreeNode0(TreeNode0::INTC, current.Sem);
            advance();
            return node;
        }
        if (current.Lex == CHARC) {
            auto node = new TreeNode0(TreeNode0::CHARC, current.Sem);
            advance();
            return node;
        }
        return parseVariable();
    }

    TreeNode0* parseVariable() {
        if (current.Lex == ID) {
            auto node = new TreeNode0(TreeNode0::IDENTIFIER, current.Sem);
            advance();
            if (current.Lex == LMIDPAREN) {
                match(LMIDPAREN);
                auto arr = new TreeNode0(TreeNode0::ARRAY_RANGE);
                arr->children.push_back(parseCondition());
                if (!match(RMIDPAREN))error("Missing: ]");
                node->children.push_back(arr);
            }
            return node;
        }
        error("Expected variable");
        return nullptr;
    }

    TreeNode0* parseIfStmt() {
        auto node = new TreeNode0(TreeNode0::IF_STMT);
        match(IF); // 消费IF

        node->children.push_back(parseCondition()); // 条件表达式

        if (!match(THEN)) error("Missing THEN");
        node->children.push_back(parseStatementList()); // THEN分支

        if (match(ELSE)) { // 处理ELSE分支
            node->children.push_back(parseStatementList());
        }
        else {
            node->children.push_back(nullptr); // 空else分支
        }

        if (!match(FI)) error("Missing FI");
        return node;
    }

    // while语句解析
    TreeNode0* parseWhileStmt() {
        auto node = new TreeNode0(TreeNode0::WHILE_STMT);
        match(WHILE); // 消费WHILE

        node->children.push_back(parseCondition()); // 循环条件

        if (!match(DO)) error("Missing DO");
        node->children.push_back(parseStatementList()); // 循环体

        if (!match(ENDWH)) error("Missing ENDWH");
        return node;
    }

    // read语句解析
    TreeNode0* parseReadStmt() {
        auto node = new TreeNode0(TreeNode0::READ_STMT);
        match(READ); // 消费READ

        if (!match(LPAREN)) error("Missing ( after READ");

        // 读取变量列表
        while (current.Lex == ID) {
            node->children.push_back(parseVariable());
            if (!match(COMMA)) break;
        }

        if (!match(RPAREN)) error("Missing ) after READ");
        return node;
    }

    // write语句解析
    TreeNode0* parseWriteStmt() {
        auto node = new TreeNode0(TreeNode0::WRITE_STMT);
        match(WRITE); // 消费WRITE

        if (!match(LPAREN)) error("Missing ( after WRITE");

        // 读取表达式列表
        do {
            node->children.push_back(parseExpression());
        } while (match(COMMA));

        if (!match(RPAREN)) error("Missing ) after WRITE");
        return node;
    }

    // return语句解析
    TreeNode0* parseReturnStmt() {
        auto node = new TreeNode0(TreeNode0::RETURN_STMT);
        match(RETURN); // RETURN

        if (current.Lex != SEMI) { // 允许空return
            node->children.push_back(parseExpression());
        }
        return node;
    }

    // 复合语句解析
    TreeNode0* parseCompoundStmt() {
        auto node = new TreeNode0(TreeNode0::COMPOUND_STMT);
        match(BEGIN); // 消费BEGIN

        node->children.push_back(parseStatementList());

        if (!match(END)) error("Missing END");
        return node;
    }

    // 字段声明解析
    TreeNode0* parseFieldDecl() {
        auto node = new TreeNode0(TreeNode0::FIELD_DECL);
        do {
            // 字段标识符列表
            auto kind = parseType();
            while (current.Lex == ID) {
                kind->children.push_back(
                    new TreeNode0(TreeNode0::IDENTIFIER, current.Sem));
                advance();
                if (!match(COMMA)) break;
            }
            if (!match(SEMI))error("Missing: ;");
            node->children.push_back(kind);
            //node->children.push_back(parseType());
        } while (current.Lex == INTEGER || current.Lex == CHAR1 || current.Lex == ARRAY);
        return node;
    }

    // 过程声明解析
    TreeNode0* parseProcDecl() {
        auto node = new TreeNode0(TreeNode0::PROCEDURE_DECL);
        match(PROCEDURE); // 消费PROCEDURE

        // 过程名
        if (current.Lex == ID) {
            node->value = current.Sem; // 过程名
            advance();
        }
        else {
            error("Missing procedure name");
        }

        // 参数列表（可选）
        if (match(LPAREN)) {
            auto paramList = new TreeNode0(TreeNode0::PARAM_LIST);

            do {
                // 解析单个参数
                auto param = parseParamDecl();
                paramList->children.push_back(param);

                // 继续解析更多参数
            } while (match(SEMI));

            if (!match(RPAREN)) error("Missing ) in parameter list");
            node->children.push_back(paramList);
        }

        if (!match(SEMI)) error("Missing ; after procedure declaration");

        // 过程体
        do {
            node->children.push_back(parseDeclaration()); // 局部声明
        } while (current.Lex == VAR);
        if (!match(BEGIN)) error("Missing BEGIN in procedure");
        node->children.push_back(parseStatementList()); // 过程语句
        if (!match(END)) error("Missing END in procedure");

        return node;
    }

    // 解析单个参数声明
    TreeNode0* parseParamDecl() {
        auto paramNode = new TreeNode0(TreeNode0::PARAM_DECL);

        // 参数传递方式（值传递或引用传递）
        if (match(VAR)) {
            paramNode->value = "var"; // 标记为引用传递
        }
        else {
            paramNode->value = "val"; // 默认值传递
        }

        // 参数名列表
        auto idList = new TreeNode0(TreeNode0::VAR_LIST);
        auto varkind = parseType();
        do {
            if (current.Lex != ID) error("Expected parameter name");
            varkind->children.push_back(
                new TreeNode0(TreeNode0::IDENTIFIER, current.Sem));
            advance();
        } while (match(COMMA));
        idList->children.push_back(varkind);

        // 参数类型


        paramNode->children.push_back(idList);
        return paramNode;
    }

    TreeNode0* parseTypeDecl() {
        auto typeDeclNode = new TreeNode0(TreeNode0::TYPE_DECL);
        match(TYPE); // 消费TYPE关键字

        do {
            if(!(current.Lex==ID))error("Expected ID identifier");
            // 创建类型定义节点
            auto typeDefNode = new TreeNode0(TreeNode0::IDENTIFIER, current.Sem);
            advance();
            if (!match(EQ)) error("Missing = in type declaration");
            // 解析单个类型定义
            if (!(current.Lex == INTEGER || current.Lex == CHAR1 || current.Lex == ARRAY)) error("Expected type identifier");
            // 解析类型描述
            typeDefNode->children.push_back(parseTypeDef());
            //advance(); // 消费类型名称

            if (!match(SEMI)) error("Missing ; after type declaration");

            typeDeclNode->children.push_back(typeDefNode);

        } while (current.Lex == INTEGER || current.Lex == CHAR1 || current.Lex == ARRAY); // 继续解析直到没有新的类型标识符

        return typeDeclNode;
    }

    TreeNode0* parseTypeDef() {
        if (current.Lex == ARRAY) {
            return parseArrayType();
        }
        if (current.Lex == RECORD) {
            return parseRecordType();
        }
        return parseBaseType();
    }

    TreeNode0* parseArrayType() {
        auto arrayNode = new TreeNode0(TreeNode0::ARRAY_TYPE);
        match(ARRAY); // 消费ARRAY

        // 解析数组维度
        if (!match(LMIDPAREN)) error("Missing [ for array dimension");
        arrayNode->children.push_back(parseArrayRange());

        // 支持多维数组
        while (match(COMMA)) {
            arrayNode->children.push_back(parseArrayRange());
        }

        if (!match(RMIDPAREN)) error("Missing ] for array dimension");
        if (!match(OF)) error("Missing OF in array declaration");

        // 解析元素类型
        arrayNode->children.push_back(parseType());
        return arrayNode;
    }

    TreeNode0* parseArrayRange() {
        auto rangeNode = new TreeNode0(TreeNode0::ARRAY_RANGE);

        // 解析下界
        if (current.Lex == INTC) {
            rangeNode->children.push_back(
                new TreeNode0(TreeNode0::CONSTANT, current.Sem));
            advance();
        }
        else if (current.Lex == ID) {
            rangeNode->children.push_back(parseVariable());
        }
        else {
            error("Invalid array index lower bound");
        }

        if (!match(UNDERANGE)) error("Missing .. in array range");

        // 解析上界
        if (current.Lex == INTC) {
            rangeNode->children.push_back(
                new TreeNode0(TreeNode0::CONSTANT, current.Sem));
            advance();
        }
        else if (current.Lex == ID) {
            rangeNode->children.push_back(parseVariable());
        }
        else {
            error("Invalid array index upper bound");
        }

        return rangeNode;
    }

    TreeNode0* parseRecordType() {
        auto recordNode = new TreeNode0(TreeNode0::RECORD_TYPE);
        match(RECORD); // 消费RECORD

        // 解析字段列表
        while (current.Lex == ID) {
            recordNode->children.push_back(parseFieldDecl());
        }

        if (!match(END)) error("Missing END for record type");
        return recordNode;
    }

    TreeNode0* parseBaseType() {
        // 处理基础类型和类型别名
        if (current.Lex == INTEGER || current.Lex == CHAR1) {
            auto node = new TreeNode0(TreeNode0::VAR_DECL, current.Sem);
            advance();
            return node;
        }
        if (current.Lex == ID) { // 类型别名
            auto node = new TreeNode0(TreeNode0::IDENTIFIER, current.Sem);
            advance();
            return node;
        }
        error("Invalid base type");
        return nullptr;
    }
    
};
TreeNode0* yufafenxi(vector<TokenType> tokens);