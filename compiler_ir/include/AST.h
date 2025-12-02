#ifndef SYSYC_AST_H
#define SYSYC_AST_H

#include <string>
#include <vector>
#include <memory>

// 前向声明
class ASTVisitor;

// ============ 基础 AST 节点 ============
class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual void accept(ASTVisitor* visitor) = 0;
};

// ============ 类型节点 ============
class TypeNode : public ASTNode {
public:
    enum Type { INT, FLOAT, VOID };
    Type type;
    
    explicit TypeNode(Type t) : type(t) {}
    void accept(ASTVisitor* visitor) override;
};

// ============ 表达式节点 ============
class ExprNode : public ASTNode {
public:
    virtual ~ExprNode() = default;
};

// 数字字面量
class NumberNode : public ExprNode {
public:
    enum NumType { INT_NUM, FLOAT_NUM };
    NumType numType;
    union {
        int intVal;
        float floatVal;
    };
    
    explicit NumberNode(int val) : numType(INT_NUM), intVal(val) {}
    explicit NumberNode(float val) : numType(FLOAT_NUM), floatVal(val) {}
    void accept(ASTVisitor* visitor) override;
};

// 变量引用
class VarRefNode : public ExprNode {
public:
    std::string name;
    
    explicit VarRefNode(const std::string& n) : name(n) {}
    void accept(ASTVisitor* visitor) override;
};

// 二元运算表达式
class BinaryExprNode : public ExprNode {
public:
    enum OpType { 
        ADD, SUB, MUL, DIV, MOD,           // 算术运算
        LT, LE, GT, GE, EQ, NE,            // 比较运算
        AND, OR                             // 逻辑运算
    };
    
    OpType op;
    std::unique_ptr<ExprNode> left;
    std::unique_ptr<ExprNode> right;
    
    BinaryExprNode(OpType o, ExprNode* l, ExprNode* r) 
        : op(o), left(l), right(r) {}
    void accept(ASTVisitor* visitor) override;
};

// 一元运算表达式
class UnaryExprNode : public ExprNode {
public:
    enum OpType { PLUS, MINUS, NOT };
    
    OpType op;
    std::unique_ptr<ExprNode> operand;
    
    UnaryExprNode(OpType o, ExprNode* opd) 
        : op(o), operand(opd) {}
    void accept(ASTVisitor* visitor) override;
};

// 函数调用表达式
class CallExprNode : public ExprNode {
public:
    std::string funcName;
    std::vector<std::unique_ptr<ExprNode>> args;
    
    explicit CallExprNode(const std::string& name) : funcName(name) {}
    void accept(ASTVisitor* visitor) override;
};

// ============ 语句节点 ============
class StmtNode : public ASTNode {
public:
    virtual ~StmtNode() = default;
};

// 赋值语句
class AssignStmtNode : public StmtNode {
public:
    std::string varName;
    std::unique_ptr<ExprNode> expr;
    
    AssignStmtNode(const std::string& name, ExprNode* e) 
        : varName(name), expr(e) {}
    void accept(ASTVisitor* visitor) override;
};

// 表达式语句
class ExprStmtNode : public StmtNode {
public:
    std::unique_ptr<ExprNode> expr;
    
    explicit ExprStmtNode(ExprNode* e) : expr(e) {}
    void accept(ASTVisitor* visitor) override;
};

// 代码块
class BlockNode : public StmtNode {
public:
    std::vector<std::unique_ptr<ASTNode>> items;
    
    void accept(ASTVisitor* visitor) override;
};

// if 语句
class IfStmtNode : public StmtNode {
public:
    std::unique_ptr<ExprNode> condition;
    std::unique_ptr<StmtNode> thenStmt;
    std::unique_ptr<StmtNode> elseStmt;  // 可选
    
    IfStmtNode(ExprNode* cond, StmtNode* then, StmtNode* els = nullptr)
        : condition(cond), thenStmt(then), elseStmt(els) {}
    void accept(ASTVisitor* visitor) override;
};

// return 语句
class ReturnStmtNode : public StmtNode {
public:
    std::unique_ptr<ExprNode> expr;  // 可选
    
    explicit ReturnStmtNode(ExprNode* e = nullptr) : expr(e) {}
    void accept(ASTVisitor* visitor) override;
};

// ============ 声明节点 ============

// 变量定义
class VarDefNode : public ASTNode {
public:
    std::string name;
    std::unique_ptr<ExprNode> initVal;  // 可选
    
    VarDefNode(const std::string& n, ExprNode* init = nullptr)
        : name(n), initVal(init) {}
    void accept(ASTVisitor* visitor) override;
};

// 变量声明
class VarDeclNode : public ASTNode {
public:
    std::unique_ptr<TypeNode> type;
    std::vector<std::unique_ptr<VarDefNode>> varDefs;
    bool isConst;
    
    VarDeclNode(TypeNode* t, bool isC = false) 
        : type(t), isConst(isC) {}
    void accept(ASTVisitor* visitor) override;
};

// 函数参数
class FuncParamNode : public ASTNode {
public:
    std::unique_ptr<TypeNode> type;
    std::string name;
    
    FuncParamNode(TypeNode* t, const std::string& n) 
        : type(t), name(n) {}
    void accept(ASTVisitor* visitor) override;
};

// 函数定义
class FuncDefNode : public ASTNode {
public:
    std::unique_ptr<TypeNode> returnType;
    std::string name;
    std::vector<std::unique_ptr<FuncParamNode>> params;
    std::unique_ptr<BlockNode> body;
    
    FuncDefNode(TypeNode* retType, const std::string& n)
        : returnType(retType), name(n) {}
    void accept(ASTVisitor* visitor) override;
};

// ============ 编译单元 ============
class CompUnitNode : public ASTNode {
public:
    std::vector<std::unique_ptr<ASTNode>> items;  // 包含全局变量和函数定义
    
    void accept(ASTVisitor* visitor) override;
};

#endif // SYSYC_AST_H