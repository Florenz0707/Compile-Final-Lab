/*!
 * @file IRGenerator.h
 * @brief IR生成器头文件
 * @version 1.0.0
 * @date 2024
 */

#ifndef SYSYC_IRGENERATOR_H
#define SYSYC_IRGENERATOR_H

#include <string>
#include <memory>
#include <iostream>

#include "AST.h"
#include "SymbolTable.h"
#include "Module.h"
#include "Function.h"
#include "BasicBlock.h"
#include "Instruction.h"
#include "IRbuilder.h"
#include "GlobalVariable.h"
#include "Constant.h"
#include "Type.h"

/**
 * @brief IR生成器类 - 访问AST节点并生成LLVM IR
 */
class IRGenerator {
private:
    Module* module;              // 当前模块
    IRBuilder* builder;          // IR构建器
    SymbolTable symbolTable;     // 符号表
    
    Function* currentFunction;   // 当前函数
    BasicBlock* currentBB;       // 当前基本块
    
    // 临时存储常量值（用于常量折叠）
    int tmpIntVal;
    float tmpFloatVal;
    bool tmpIsFloat;
    bool isConstExpr;            // 是否在计算常量表达式
    
    // 用于短路求值的基本块
    BasicBlock* trueBB;          // 条件为真时的目标基本块
    BasicBlock* falseBB;         // 条件为假时的目标基本块

public:
    /**
     * @brief 构造函数
     * @param sourceFileName 源文件名
     */
    explicit IRGenerator(const std::string& sourceFileName);
    
    /**
     * @brief 析构函数
     */
    ~IRGenerator();
    
    /**
     * @brief 获取生成的Module
     * @return Module* 模块指针
     */
    Module* getModule() const { return module; }
    
    /**
     * @brief 生成IR（入口函数）
     * @param ast AST根节点
     */
    void generate(std::shared_ptr<CompUnitNode> ast);
    
    /**
     * @brief 打印生成的IR
     * @return std::string IR字符串
     */
    std::string print();
    
    // ==================== Visitor函数 ====================
    
    /**
     * @brief 访问编译单元节点
     */
    void visitCompUnit(std::shared_ptr<CompUnitNode> node);
    
    /**
     * @brief 访问声明节点
     */
    void visitDecl(std::shared_ptr<DeclNode> node);
    
    /**
     * @brief 访问常量声明节点
     */
    void visitConstDecl(std::shared_ptr<ConstDeclNode> node);
    
    /**
     * @brief 访问变量声明节点
     */
    void visitVarDecl(std::shared_ptr<VarDeclNode> node);
    
    /**
     * @brief 访问常量定义节点
     */
    void visitConstDef(std::shared_ptr<ConstDefNode> node, BType bType);
    
    /**
     * @brief 访问变量定义节点
     */
    void visitVarDef(std::shared_ptr<VarDefNode> node, BType bType);
    
    /**
     * @brief 访问函数定义节点
     */
    void visitFuncDef(std::shared_ptr<FuncDefNode> node);
    
    /**
     * @brief 访问代码块节点
     */
    void visitBlock(std::shared_ptr<BlockNode> node);
    
    /**
     * @brief 访问代码块项节点
     */
    void visitBlockItem(std::shared_ptr<BlockItemNode> node);
    
    /**
     * @brief 访问语句节点
     */
    void visitStmt(std::shared_ptr<StmtNode> node);
    
    /**
     * @brief 访问赋值语句
     */
    void visitAssignStmt(std::shared_ptr<StmtNode> node);
    
    /**
     * @brief 访问表达式语句
     */
    void visitExpStmt(std::shared_ptr<StmtNode> node);
    
    /**
     * @brief 访问块语句
     */
    void visitBlockStmt(std::shared_ptr<StmtNode> node);
    
    /**
     * @brief 访问if语句
     */
    void visitIfStmt(std::shared_ptr<StmtNode> node);
    
    /**
     * @brief 访问return语句
     */
    void visitReturnStmt(std::shared_ptr<StmtNode> node);
    
    /**
     * @brief 访问表达式节点（通用入口）
     * @return Value* 表达式的值
     */
    Value* visitExp(std::shared_ptr<ExpNode> node);
    
    /**
     * @brief 访问条件表达式节点
     * @return Value* 条件表达式的值
     */
    Value* visitCond(std::shared_ptr<CondNode> node);
    
    /**
     * @brief 访问左值节点
     * @param load 是否加载值（true返回值，false返回地址）
     * @return Value* 左值的值或地址
     */
    Value* visitLVal(std::shared_ptr<LValNode> node, bool load = true);
    
    /**
     * @brief 访问数字节点
     * @return Value* 数字常量值
     */
    Value* visitNumber(std::shared_ptr<NumberNode> node);
    
    /**
     * @brief 访问基本表达式节点
     * @return Value* 表达式的值
     */
    Value* visitPrimaryExp(std::shared_ptr<PrimaryExpNode> node);
    
    /**
     * @brief 访问一元表达式节点
     * @return Value* 表达式的值
     */
    Value* visitUnaryExp(std::shared_ptr<UnaryExpNode> node);
    
    /**
     * @brief 访问乘法表达式节点
     * @return Value* 表达式的值
     */
    Value* visitMulExp(std::shared_ptr<MulExpNode> node);
    
    /**
     * @brief 访问加法表达式节点
     * @return Value* 表达式的值
     */
    Value* visitAddExp(std::shared_ptr<AddExpNode> node);
    
    /**
     * @brief 访问关系表达式节点
     * @return Value* 比较结果（i1类型）
     */
    Value* visitRelExp(std::shared_ptr<RelExpNode> node);
    
    /**
     * @brief 访问相等表达式节点
     * @return Value* 比较结果（i1类型）
     */
    Value* visitEqExp(std::shared_ptr<EqExpNode> node);
    
    /**
     * @brief 访问逻辑与表达式节点
     * @return Value* 逻辑与结果（i1类型）
     */
    Value* visitLAndExp(std::shared_ptr<LAndExpNode> node);
    
    /**
     * @brief 访问逻辑或表达式节点
     * @return Value* 逻辑或结果（i1类型）
     */
    Value* visitLOrExp(std::shared_ptr<LOrExpNode> node);
    
private:
    // ==================== 辅助函数 ====================
    
    /**
     * @brief 将BType转换为LLVM Type
     */
    Type* bTypeToLLVMType(BType bType);
    
    /**
     * @brief 创建全局变量
     */
    GlobalVariable* createGlobalVariable(const std::string& name, Type* type, 
                                         bool isConst, Constant* init);
    
    /**
     * @brief 创建局部变量（alloca指令）
     */
    AllocaInst* createLocalVariable(const std::string& name, Type* type);
    
    /**
     * @brief 声明运行时库函数
     */
    void declareRuntimeFunctions();
    
    /**
     * @brief 将值转换为i32类型（用于条件判断）
     */
    Value* ensureInt32(Value* val);
    
    /**
     * @brief 将值转换为i1类型（用于条件判断）
     */
    Value* ensureInt1(Value* val);
    
    /**
     * @brief 计算常量表达式的整数值
     */
    int evalConstInt(std::shared_ptr<ExpNode> node);
    
    /**
     * @brief 计算常量表达式的浮点值
     */
    float evalConstFloat(std::shared_ptr<ExpNode> node);
};

#endif // SYSYC_IRGENERATOR_H
