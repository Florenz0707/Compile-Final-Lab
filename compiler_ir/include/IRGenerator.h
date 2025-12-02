#ifndef SYSYC_IRGENERATOR_H
#define SYSYC_IRGENERATOR_H

#include "AST.h"
#include "IRbuilder.h"
#include "Module.h"
#include "Function.h"
#include "BasicBlock.h"
#include "Type.h"
#include "Value.h"
#include "GlobalVariable.h"
#include <map>
#include <string>
#include <stack>

class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;
    
    // 访问各种节点
    virtual void visit(TypeNode* node) = 0;
    virtual void visit(NumberNode* node) = 0;
    virtual void visit(VarRefNode* node) = 0;
    virtual void visit(BinaryExprNode* node) = 0;
    virtual void visit(UnaryExprNode* node) = 0;
    virtual void visit(CallExprNode* node) = 0;
    virtual void visit(AssignStmtNode* node) = 0;
    virtual void visit(ExprStmtNode* node) = 0;
    virtual void visit(BlockNode* node) = 0;
    virtual void visit(IfStmtNode* node) = 0;
    virtual void visit(ReturnStmtNode* node) = 0;
    virtual void visit(VarDefNode* node) = 0;
    virtual void visit(VarDeclNode* node) = 0;
    virtual void visit(FuncParamNode* node) = 0;
    virtual void visit(FuncDefNode* node) = 0;
    virtual void visit(CompUnitNode* node) = 0;
};

/**
 * @brief IR 生成器类，使用 Visitor 模式遍历 AST 并生成 LLVM IR
 */
class IRGenerator : public ASTVisitor {
private:
    Module* module_;                                    // 模块
    IRBuilder* builder_;                                // IR 构造器
    Function* currentFunction_;                         // 当前函数
    
    // 符号表：变量名 -> Value*
    std::map<std::string, Value*> namedValues_;
    
    // 当前表达式的值（用于表达式求值）
    Value* currentValue_;
    
    // 当前类型（用于类型转换）
    Type* currentType_;
    
    // 是否在全局作用域
    bool isGlobalScope_;
    
    // 用于 if 语句的基本块管理
    struct IfContext {
        BasicBlock* mergeBB;
    };
    std::stack<IfContext> ifContextStack_;
    
public:
    /**
     * @brief 构造 IR 生成器
     * @param moduleName 模块名称
     */
    explicit IRGenerator(const std::string& moduleName);
    
    /**
     * @brief 析构函数
     */
    ~IRGenerator();
    
    /**
     * @brief 生成 IR 代码
     * @param root AST 根节点
     */
    void generate(CompUnitNode* root);
    
    /**
     * @brief 获取生成的模块
     */
    Module* getModule() { return module_; }
    
    /**
     * @brief 打印生成的 IR
     */
    std::string print();
    
    // ============ Visitor 接口实现 ============
    void visit(TypeNode* node) override;
    void visit(NumberNode* node) override;
    void visit(VarRefNode* node) override;
    void visit(BinaryExprNode* node) override;
    void visit(UnaryExprNode* node) override;
    void visit(CallExprNode* node) override;
    void visit(AssignStmtNode* node) override;
    void visit(ExprStmtNode* node) override;
    void visit(BlockNode* node) override;
    void visit(IfStmtNode* node) override;
    void visit(ReturnStmtNode* node) override;
    void visit(VarDefNode* node) override;
    void visit(VarDeclNode* node) override;
    void visit(FuncParamNode* node) override;
    void visit(FuncDefNode* node) override;
    void visit(CompUnitNode* node) override;
    
private:
    /**
     * @brief 将 AST 类型转换为 LLVM Type
     */
    Type* getTypeFromASTType(TypeNode::Type type);
    
    /**
     * @brief 创建运行时库函数声明
     */
    void createRuntimeFunctions();
    
    /**
     * @brief 进入新的作用域
     */
    void enterScope();
    
    /**
     * @brief 离开当前作用域
     */
    void leaveScope();
};

#endif // SYSYC_IRGENERATOR_H