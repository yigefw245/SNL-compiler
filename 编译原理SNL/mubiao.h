#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <sstream>
#include"yufa.h"
#include"zhongjiandaima.h"
#include<stack>
using namespace std;

// MIPS代码生成器类
class MIPSGenerator {
private:
    vector<Quadruple> quads;  // 四元式列表
    map<string, int> varMap;  // 变量到内存偏移的映射
    int tempCount;            // 临时变量计数器
    int labelCount;           // 标签计数器
    int stackOffset;          // 栈偏移量
    map<string, int> arr;
    struct IfThenElse {
        string elseLabel;
        string fiLabel;
    };
    vector<IfThenElse> ifStack;
    struct FunctionInfo {
        string name;
        int frameSize;
        int level;
    };
    vector<FunctionInfo> functionStack;  // 函数调用栈
    // WHILE循环相关状态
    struct WhileLoop {
        string startLabel;
        string endLabel;
    };
    vector<WhileLoop> whileStack;  // WHILE循环栈

public:
    MIPSGenerator() : tempCount(0), labelCount(0), stackOffset(0) {}

    // 添加四元式
    void addQuadruple(const Quadruple& q) {
        quads.push_back(q);

        // 记录变量和临时变量
        if (isVariable(q.arg1)) allocateVariable(q.arg1,q);
        if (isVariable(q.arg2)) allocateVariable(q.arg2,q);
        if (isVariable(q.result)) allocateVariable(q.result,q);
    }

    // 生成MIPS代码
    void generateMIPS() {
        // 数据段声明
        cout << ".data" << endl;
        for (const auto& var : varMap) {
            if(arr[var.first]!=1)
            cout << var.first << ": .word 0" << endl;
            else {
                cout << var.first << ": .space 200" << endl;
            }
            
        }
        int ff = 1;
        // 代码段开始
        cout << "\n.text" << endl;
        cout << "j main" << endl;
        if(quads[0].op!="ENTRY")
        cout << "main:" << endl;
        // 生成每条四元式的代码
        stack<int>en;//函数，过程入口
        for (size_t i = 0; i < quads.size(); i++) {
            const auto& quad = quads[i];

            if (quad.op == "ENTRY") {
                en.push(1);
                handleEntry(quad);
            }
            else if(ff==0&&en.size()==0){
                cout << "main:" << endl;
                ff = 1;
                i--;
            }
            else if (quad.op == "WHILE") {
                handleWhile(quad, i);
            }
            else if (quad.op == "do") {
                handleDo(quad);
            }
            else if (quad.op == "ENDWH") {
                handleEndWhile();
            }
            else if (quad.op == "END") {
                handleEnd();
                // END之后的代码不应该执行，所以break
                break;
            }
            else if (quad.op == "ENDFUC") {
                handleEndFunc();
                ff = 0;
                en.pop();
            }
            else if (quad.op == "THEN") {
                handleThen(quad, i);
            }
            else if (quad.op == "ELSE") {
                handleElse();
            }
            else if (quad.op == "ENDIF") {
                handleFi();
            }
            else if (quad.op == "VarACT") {
                handleVarAct(quad);
            }
            else if (quad.op == "CALL") {
                handleCall(quad);
            }
            else {
                generateQuadrupleCode(quad);
            }
        }

        // 程序结束
        if (find_if(quads.begin(), quads.end(),
            [](const Quadruple& q) { return q.op == "END"; }) == quads.end()) {
            generateProgramExit();
        }
    }

private:
    void handleRead(const Quadruple& quad) {
        if (quad.result.empty()) {
            cerr << "错误: read操作缺少目标变量" << endl;
            return;
        }

        cout << "# 读取整数到 " << quad.result << endl;
        cout << "li $v0, 5" << endl;      // 系统调用5表示读取整数
        cout << "syscall" << endl;
        cout << "sw $v0, " << quad.result << " # 存储输入值" << endl;
    }
    void handleWrite(const Quadruple& quad) {
        if (quad.result.empty()) {
            cerr << "错误: write操作缺少要输出的变量" << endl;
            return;
        }

        cout << "# 输出变量 " << quad.result << " 的值" << endl;

        // 加载要输出的值到$a0
        if(arr[quad.result]==1){
            cout << "lw $t1, " << quad.result << endl;
            cout << "lw $a0,0($t1) "<< endl;
        }
        else {
            if(isNumber(quad.result))
                cout << "li $a0, " << quad.result << endl;
            else
            cout << "lw $a0, " << quad.result << endl;
        }
        // 系统调用1输出整数
        cout << "li $v0, 1" << endl;
        cout << "syscall" << endl;

        // 输出换行符(可选)
        cout << "# 输出换行符" << endl;
        cout << "li $a0, 10" << endl;  // ASCII码10是换行符
        cout << "li $v0, 11" << endl;  // 系统调用11输出字符
        cout << "syscall" << endl;
    }
    void handleThen(const Quadruple& quad, size_t& currentIndex) {
        IfThenElse ifContext;
        ifContext.elseLabel = newLabel("ELSE");
        ifContext.fiLabel = newLabel("FI");
        ifStack.push_back(ifContext);

        // THEN前面通常是一个条件跳转
        if (currentIndex > 0) {
            const auto& prevQuad = quads[currentIndex - 1];
            if (prevQuad.op == "<") {
                // 修改跳转目标为ELSE标签
                Quadruple modifiedQuad = prevQuad;
                modifiedQuad.result = ifContext.elseLabel;
                modifiedQuad.op = "j>=";
                modifiedQuad.arg1 = prevQuad.result;
                modifiedQuad.arg2 = "0";
                // 重新生成这个跳转指令
                generateQuadrupleCode(modifiedQuad);
            }
        }
    }
    // 处理ELSE四元式
    void handleElse() {
        if (ifStack.empty()) {
            cerr << "错误: ELSE没有对应的THEN" << endl;
            return;
        }

        IfThenElse& ifContext = ifStack.back();

        // 跳转到FI标签(跳过ELSE块)
        cout << "j " << ifContext.fiLabel << endl;

        // 生成ELSE标签
        cout << ifContext.elseLabel << ":" << endl;
    }

    // 处理FI四元式
    void handleFi() {
        if (ifStack.empty()) {
            cerr << "错误: FI没有对应的THEN" << endl;
            return;
        }

        IfThenElse ifContext = ifStack.back();
        ifStack.pop_back();

        // 生成FI标签
        cout << ifContext.fiLabel << ":" << endl;
    }

    void handleEntry(const Quadruple& quad) {
        FunctionInfo func;
        func.name = quad.arg1;
        func.frameSize = stoi(quad.arg2);//字符串变数字
        func.level = stoi(quad.result);
        functionStack.push_back(func);

        // 生成函数入口代码
        cout << quad.arg1 << ":" << endl;

        // 函数序言(prologue)
        cout << "# 函数序言 - " << quad.arg1 << endl;
        cout << "addi $sp, $sp, -" << func.frameSize << " # 分配栈空间" << endl;
        cout << "sw $ra, " << func.frameSize - 4 << "($sp) # 保存返回地址" << endl;
        cout << "sw $fp, " << func.frameSize - 8 << "($sp) # 保存帧指针" << endl;
        cout << "addi $fp, $sp, " << func.frameSize << " # 设置新帧指针" << endl;

        // 根据需要保存其他寄存器
        cout << "# 保存其他需要保存的寄存器" << endl;
        for (int i = 0; i < 8; i++) {  // 保存$s0-$s7
            cout << "sw $s" << i << ", " << func.frameSize - 12 - (i * 4) << "($sp)" << endl;
        }
    }
    void handleEnd() {
        generateProgramExit();
    }
    void handleEndFunc() {
        if (functionStack.empty()) {
            cerr << "错误: ENDFUC没有对应的ENTRY" << endl;
            return;
        }

        FunctionInfo func = functionStack.back();
        functionStack.pop_back();

        cout << "\n# 函数收尾 - " << func.name << endl;

        // 恢复保存的寄存器
        cout << "# 恢复保存的寄存器" << endl;
        for (int i = 0; i < 8; i++) {  // 恢复$s0-$s7
            cout << "lw $s" << i << ", " << func.frameSize - 12 - (i * 4) << "($sp)" << endl;
        }

        // 恢复帧指针和返回地址
        cout << "lw $fp, " << func.frameSize - 8 << "($sp) # 恢复帧指针" << endl;
        cout << "lw $ra, " << func.frameSize - 4 << "($sp) # 恢复返回地址" << endl;

        // 释放栈空间
        cout << "addi $sp, $sp, " << func.frameSize << " # 释放栈空间" << endl;

        // 返回调用者
        cout << "jr $ra # 返回调用者" << endl;
    }
    // 生成程序退出代码
    void generateProgramExit() {
        cout << "\n# 程序结束" << endl;
        cout << "li $v0, 10" << endl;
        cout << "syscall" << endl;
    }
    // 处理WHILE四元式 (WHILE循环开始)
    void handleWhile(const Quadruple& quad, size_t& currentIndex) {
        // 创建新的WHILE循环上下文
        WhileLoop loop;
        loop.startLabel = newLabel("WHILE_START");
        loop.endLabel = newLabel("WHILE_END");
        whileStack.push_back(loop);

        // 生成开始标签
        cout << loop.startLabel << ":" << endl;

        // WHERE后面通常跟着一个条件跳转四元式
        // 例如: (j<, a, b, ENDWH)
        while (currentIndex + 1 < quads.size()) {
            const auto& nextQuad = quads[currentIndex + 1];
            if (nextQuad.op=="<") {
                // 修改跳转目标为ENDWH标签
                Quadruple modifiedQuad = nextQuad;
                modifiedQuad.result = loop.endLabel;
                
                if (modifiedQuad.op == "<") {
                    modifiedQuad.op = "j>=";
                    generateQuadrupleCode(modifiedQuad);
                    currentIndex++;
                    break;
                }
            }
            generateQuadrupleCode(nextQuad);
            currentIndex++;
        }
    }

    // 处理DO四元式 (WHILE循环体开始)
    void handleDo(const Quadruple& quad) {
        if (whileStack.empty()) {
            cerr << "错误: DO没有对应的WHERE" << endl;
            return;
        }
        // DO本身不生成代码，只是标记循环体开始
    }

    // 处理ENDWH四元式 (WHILE循环结束)
    void handleEndWhile() {
        if (whileStack.empty()) {
            cerr << "错误: ENDWH没有对应的WHERE" << endl;
            return;
        }

        WhileLoop loop = whileStack.back();
        whileStack.pop_back();

        // 跳回循环开始
        cout << "j " << loop.startLabel << endl;

        // 生成结束标签
        cout << loop.endLabel << ":" << endl;
    }
    //传参
    void handleVarAct(const Quadruple& quad) {
        if (quad.arg1.empty() || quad.arg2.empty()) {
            cerr << "错误: VarACT缺少必要参数" << endl;
            return;
        }

        string param = quad.arg1;
        int offset = stoi(quad.arg2);
        // int size = stoi(quad.result); // 通常固定为4字节

        cout << "# 传递参数 " << param << " 到偏移量 " << offset << endl;

        // 加载参数值到临时寄存器
        if (isNumber(param)) {
            cout << "li $t0, " << param << endl;
        }
        else {
            cout << "lw $t0, " << param << endl;
        }

        // 存储参数到栈帧
        cout << "sw $t0, " << offset << "($sp)" << endl;
    }
    //函数调用
    void handleCall(const Quadruple& quad) {
        if (quad.arg1.empty()) {
            cerr << "错误: CALL缺少函数名" << endl;
            return;
        }

        string funcName = "Label" + quad.arg1;
        bool hasReturn = (quad.arg2 == "true");
        string returnVar = quad.result;

        cout << "# 调用函数 " << funcName << endl;

        // 保存调用者保存寄存器 ($t0-$t9)
        cout << "addi $sp, $sp, -36" << endl;
        for (int i = 0; i <= 9; i++) {
            cout << "sw $t" << i << ", " << (i * 4) << "($sp)" << endl;
        }

        // 调用函数
        cout << "jal " << funcName << endl;

        // 恢复调用者保存寄存器
        for (int i = 0; i <= 9; i++) {
            cout << "lw $t" << i << ", " << (i * 4) << "($sp)" << endl;
        }
        cout << "addi $sp, $sp, 36" << endl;

        // 如果有返回值，存储到指定变量
        if (hasReturn && !returnVar.empty()) {
            cout << "sw $v0, " << returnVar << " # 存储返回值" << endl;
        }
    }
    // 生成新的唯一标签
    string newLabel(const string& prefix) {
        return prefix + "_" + to_string(labelCount++);
    }

    // 判断是否是变量（临时变量或用户变量）
    bool isVariable(const string& s) {
        return !s.empty() && s != "_" && !isdigit(s[0])&&s[0]!='\'';
    }

    // 为变量分配栈空间
    void allocateVariable(const string& var, Quadruple q) {
        string var0 = var;
        if (varMap.find(var) == varMap.end()&&var0.substr(0,5)!="Label") {
            if(!(q.op=="[+]")) {
                arr[var] = 0;
            varMap[var] = stackOffset;
            stackOffset += 4;  // 每个变量占4字节
            }
            else {
                varMap[var] = stackOffset;
                arr[var] = 1;
                stackOffset += 200;
            }
        }
    }

    // 生成单个四元式的MIPS代码
    void generateQuadrupleCode(const Quadruple& quad) {
        if (quad.op == "=") {
            // 赋值操作
            if (isNumber(quad.arg1)) {
                cout << "li $t0, " << quad.arg1 << endl;
                if (arr[quad.result] == 1) {
                    cout << "lw $t1," << quad.result << endl;
                    cout << "sw $t0,0($t1)" << endl;
                    return;
                }
                cout << "sw $t0, " << quad.result << endl;
            }
            else {
                if (arr[quad.result] == 1) {
                    cout << "lw $t0, " << quad.arg1 << endl;
                    if (arr[quad.arg1] == 1) {
                        cout << "lw $t0,0($t0)" << endl;
                    }
                    cout << "lw $t1," << quad.result << endl;
                    cout << "sw $t0,0($t1)" << endl;
                    return;
                }
                else {
                    cout << "lw $t0, " << quad.arg1 << endl;
                    if (arr[quad.arg1] == 1) {
                        cout << "lw $t0,0($t0)" << endl;
                    }
                    cout << "sw $t0, " << quad.result << endl;
                }
            }
        }
        else if (quad.op == "+") {
            loadOperands(quad.arg1, quad.arg2, "$t0", "$t1");
            cout << "add $t2, $t0, $t1" << endl;
            cout << "sw $t2, " << quad.result << endl;
        }
        else if (quad.op == "[+]") {
            loadOperands0(quad.arg1, quad.arg2, "$t0", "$t1");
            cout << "sll $t2,$t1,2" << endl;
            cout << "add $t2, $t0, $t2" << endl;
            cout << "sw $t2, " << quad.result << endl;
        }
        else if (quad.op == "-") {
            loadOperands(quad.arg1, quad.arg2, "$t0", "$t1");
            cout << "sub $t2, $t0, $t1" << endl;
            cout << "sw $t2, " << quad.result << endl;
        }
        else if (quad.op == "*") {
            loadOperands(quad.arg1, quad.arg2, "$t0", "$t1");
            cout << "mul $t2, $t0, $t1" << endl;
            cout << "sw $t2, " << quad.result << endl;
        }
        else if (quad.op == "/") {
            loadOperands(quad.arg1, quad.arg2, "$t0", "$t1");
            cout << "div $t0, $t1" << endl;
            cout << "mflo $t2" << endl;
            cout << "sw $t2, " << quad.result << endl;
        }
        else if (quad.op == "READ") {
            handleRead(quad);
        }
        else if (quad.op == "WRITE") {
            handleWrite(quad);
        }
        else if (quad.op == "j") {
            // 无条件跳转
            cout << "j " << quad.result << endl;
        }
        else if (quad.op.substr(0, 1) == "j") {
            // 条件跳转 (j<, j>, j<=, j>=, j==, j!=)
            string cond = quad.op.substr(1);
            loadOperands(quad.arg1, quad.arg2, "$t0", "$t1");

            if (cond == "<") cout << "blt $t0, $t1, " << quad.result << endl;
            else if (cond == ">") cout << "bgt $t0, $t1, " << quad.result << endl;
            else if (cond == "<=") cout << "ble $t0, $t1, " << quad.result << endl;
            else if (cond == ">=") cout << "bge $t0, $t1, " << quad.result << endl;
            else if (cond == "==") cout << "beq $t0, $t1, " << quad.result << endl;
            else if (cond == "!=") cout << "bne $t0, $t1, " << quad.result << endl;
        }
        else if (quad.op == "<") {
            loadOperands(quad.arg1, quad.arg2, "$t1", "$t2");
           // cout << "lw $t1," << quad.arg1 << endl;
           // cout << "lw $t2," << quad.arg2 << endl;
            cout << "sub $t0,$t1,$t2"<< endl;
            cout << "sw $t0," << quad.result << endl;
        }
        else if (quad.op.find(":") != string::npos) {
            // 标签
            cout << quad.op << ":" << endl;
        }
        else {
            cerr << "未知操作符: " << quad.op << endl;
        }
    }

    // 加载操作数到寄存器
    void loadOperands(const string& arg1, const string& arg2,const string& reg1, const string& reg2) {
        if (isNumber(arg1)) {
            cout << "li " << reg1 << ", " << arg1 << endl;
        }
        else {
            cout << "lw " << reg1 << ", " << arg1 << endl;
            if (arr[arg1] == 1) {
                cout << "lw " << reg1 << ",0(" << reg1 << ")" << endl;
            }
        }

        if (isNumber(arg2)) {
            cout << "li " << reg2 << ", " << arg2 << endl;
        }
        else {
            cout << "lw " << reg2 << ", " << arg2 << endl;
            if (arr[arg2] == 1) {
                cout << "lw " << reg2 << ",0(" << reg2 << ")" << endl;
            }
        }
    }
    void loadOperands0(const string& arg1, const string& arg2,const string& reg1, const string& reg2) {
        if (isNumber(arg1)) {
            cout << "li " << reg1 << ", " << arg1 << endl;
        }
        else {
            cout << "la " << reg1 << ", " << arg1 << endl;
        }

        if (isNumber(arg2)) {
            cout << "li " << reg2 << ", " << arg2 << endl;
        }
        else {
            cout << "lw " << reg2 << ", " << arg2 << endl;
        }
    }

    // 判断字符串是否是数字
    bool isNumber(const string& s) {
        if (s.empty()) return false;
        if (s == "_") return false;

        size_t i = 0;
        if (s[0] == '-') {
            if (s.length() == 1) return false;
            i = 1;
        }

        for (; i < s.length(); i++) {
            if (!isdigit(s[i])) {
                return false;
            }
        }
        return true;
    }
};

void mubiao(vector<Quadruple> quadruples);