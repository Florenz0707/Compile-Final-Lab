/*!
 * @file AST.h
 * @brief 抽象语法树节点定义头文件
 * @version 1.0.0
 * @date 2025
 */

#ifndef SYSYC_AST_H
#define SYSYC_AST_H

#include <string>
#include <vector>
#include <memory>
#include <variant>

// 前向声明
class ASTNode;
class CompUnitNode;
class DeclNode;
class ConstDeclNode;
class VarDeclNode;
class ConstDefNode;
class VarDefNode;
class InitValNode;
class FuncDefNode;
class FuncFParamNode;
class BlockNode;
class BlockItemNode;
class StmtNode;
class ExpNode;
class CondNode;
class LValNode;
class PrimaryExpNode;
class UnaryExpNode;
class FuncRParamsNode;
class MulExpNode;
class AddExpNode;
class RelExpNode;
class EqExpNode;
class LAndExpNode;
class LOrExpNode;
class NumberNode;

/**
 * @brief 基本类型枚举
 */
enum class BType {
    INT,
    FLOAT,
    VOID
};

/**
 * @brief 一元运算符枚举
 */
enum class UnaryOp {
    PLUS,   // +
    MINUS,  // -
    NOT     // !
};

/**
 * @brief 二元运算符枚举
 */
enum class BinaryOp {
    ADD,    // +
    SUB,    // -
    MUL,    // *
    DIV,    // /
    MOD     // %
};

/**
 * @brief 关系运算符枚举
 */
enum class RelOp {
    LT,     // <
    GT,     // >
    LE,     // <=
    GE      // >=
};

/**
 * @brief 相等运算符枚举
 */
enum class EqOp {
    EQ,     // ==
    NE      // !=
};

/**
 * @brief AST节点基类
 */
class ASTNode {
public:
    virtual ~ASTNode() = default;
    std::string type;  // 节点类型名称

    ASTNode() : type("ASTNode") {}
    explicit ASTNode(const std::string& t) : type(t) {}
};

/**
 * @brief 编译单元节点 - Program -> compUnit
 */
class CompUnitNode : public ASTNode {
public:
    std::vector<std::shared_ptr<DeclNode>> decls;        // 全局声明
    std::vector<std::shared_ptr<FuncDefNode>> funcDefs;  // 函数定义

    CompUnitNode() : ASTNode("CompUnit") {}
};

/**
 * @brief 声明节点基类
 */
class DeclNode : public ASTNode {
public:
    bool isConst = false;

    DeclNode() : ASTNode("Decl") {}
    explicit DeclNode(const std::string& t) : ASTNode(t) {}
};

/**
 * @brief 常量声明节点 - constDecl -> 'const' bType constDef (',' constDef)* ';'
 */
class ConstDeclNode : public DeclNode {
public:
    BType bType;
    std::vector<std::shared_ptr<ConstDefNode>> constDefs;

    ConstDeclNode() : DeclNode("ConstDecl") { isConst = true; }
};

/**
 * @brief 变量声明节点 - varDecl -> bType varDef (',' varDef)* ';'
 */
class VarDeclNode : public DeclNode {
public:
    BType bType;
    std::vector<std::shared_ptr<VarDefNode>> varDefs;

    VarDeclNode() : DeclNode("VarDecl") { isConst = false; }
};

/**
 * @brief 常量定义节点 - constDef -> Ident '=' constInitVal
 */
class ConstDefNode : public ASTNode {
public:
    std::string ident;
    std::shared_ptr<ExpNode> initVal;  // constInitVal -> constExp -> addExp

    ConstDefNode() : ASTNode("ConstDef") {}
};

/**
 * @brief 变量定义节点 - varDef -> Ident | Ident '=' initVal
 */
class VarDefNode : public ASTNode {
public:
    std::string ident;
    std::shared_ptr<ExpNode> initVal;  // 可选，为nullptr表示没有初始化

    VarDefNode() : ASTNode("VarDef") {}
};

/**
 * @brief 初始化值节点 - initVal -> exp
 */
class InitValNode : public ASTNode {
public:
    std::shared_ptr<ExpNode> exp;

    InitValNode() : ASTNode("InitVal") {}
};

/**
 * @brief 函数定义节点 - funcDef -> funcType Ident '(' (funcFParams)? ')' block
 */
class FuncDefNode : public ASTNode {
public:
    BType returnType;
    std::string ident;
    std::vector<std::shared_ptr<FuncFParamNode>> params;
    std::shared_ptr<BlockNode> block;

    FuncDefNode() : ASTNode("FuncDef") {}
};

/**
 * @brief 函数形参节点 - funcFParam -> bType Ident
 */
class FuncFParamNode : public ASTNode {
public:
    BType bType;
    std::string ident;

    FuncFParamNode() : ASTNode("FuncFParam") {}
};

/**
 * @brief 代码块节点 - block -> '{' (blockItem)* '}'
 */
class BlockNode : public ASTNode {
public:
    std::vector<std::shared_ptr<BlockItemNode>> items;

    BlockNode() : ASTNode("Block") {}
};

/**
 * @brief 代码块项节点 - blockItem -> decl | stmt
 */
class BlockItemNode : public ASTNode {
public:
    std::shared_ptr<DeclNode> decl;  // 声明
    std::shared_ptr<StmtNode> stmt;  // 语句

    BlockItemNode() : ASTNode("BlockItem") {}
};

/**
 * @brief 语句类型枚举
 */
enum class StmtType {
    ASSIGN,     // lVal '=' exp ';'
    EXP,        // (exp)? ';'
    BLOCK,      // block
    IF,         // 'if' '(' cond ')' stmt ('else' stmt)?
    RETURN      // 'return' (exp)? ';'
};

/**
 * @brief 语句节点
 */
class StmtNode : public ASTNode {
public:
    StmtType stmtType;

    // ASSIGN类型
    std::shared_ptr<LValNode> lVal;
    std::shared_ptr<ExpNode> exp;

    // BLOCK类型
    std::shared_ptr<BlockNode> block;

    // IF类型
    std::shared_ptr<CondNode> cond;      // 原来写为CondNode
    std::shared_ptr<StmtNode> thenStmt;
    std::shared_ptr<StmtNode> elseStmt;  // 可选

    // RETURN类型 - 使用exp字段

    StmtNode() : ASTNode("Stmt"), stmtType(StmtType::EXP) {}
};

/**
 * @brief 表达式节点基类 - exp -> addExp
 */
class ExpNode : public ASTNode {
public:
    ExpNode() : ASTNode("Exp") {}
    explicit ExpNode(const std::string& t) : ASTNode(t) {}
};

/**
 * @brief 条件表达式节点 - cond -> lOrExp
 */
class CondNode : public ASTNode {
public:
    std::shared_ptr<LOrExpNode> lOrExp;

    CondNode() : ASTNode("Cond") {}
};

/**
 * @brief 左值节点 - lVal -> Ident
 */
class LValNode : public ExpNode {
public:
    std::string ident;

    LValNode() : ExpNode("LVal") {}
};

/**
 * @brief 数字节点 - number -> IntConst | floatConst
 */
class NumberNode : public ExpNode {
public:
    bool isFloat = false;
    int intVal = 0;
    float floatVal = 0.0f;

    NumberNode() : ExpNode("Number") {}
};

/**
 * @brief 基本表达式节点 - primaryExp -> '(' exp ')' | lVal | number
 */
class PrimaryExpNode : public ExpNode {
public:
    enum class PrimaryType {
        PAREN_EXP,  // '(' exp ')'
        LVAL,       // lVal
        NUMBER      // number
    };

    PrimaryType primaryType;
    std::shared_ptr<ExpNode> exp;       // PAREN_EXP
    std::shared_ptr<LValNode> lVal;     // LVAL
    std::shared_ptr<NumberNode> number; // NUMBER

    PrimaryExpNode() : ExpNode("PrimaryExp"), primaryType(PrimaryType::NUMBER) {}
};

/**
 * @brief 一元表达式节点 - unaryExp -> primaryExp | Ident '(' funcRParams? ')' | unaryOp unaryExp
 */
class UnaryExpNode : public ExpNode {
public:
    enum class UnaryType {
        PRIMARY,    // primaryExp
        FUNC_CALL,  // Ident '(' funcRParams? ')'
        UNARY_OP    // unaryOp unaryExp
    };

    UnaryType unaryType;
    std::shared_ptr<PrimaryExpNode> primaryExp; // PRIMARY
    std::string funcName;                        // FUNC_CALL
    std::vector<std::shared_ptr<ExpNode>> args; // FUNC_CALL
    UnaryOp unaryOp;                            // UNARY_OP
    std::shared_ptr<UnaryExpNode> unaryExp;     // UNARY_OP

    UnaryExpNode() : ExpNode("UnaryExp"), unaryType(UnaryType::PRIMARY) {}
};

/**
 * @brief 乘法表达式节点 - mulExp -> unaryExp | mulExp ('*' | '/' | '%') unaryExp
 */
class MulExpNode : public ExpNode {
public:
    std::shared_ptr<MulExpNode> left;   // 左操作数（可选）
    BinaryOp op;                        // 运算符 (MUL, DIV, MOD)
    std::shared_ptr<UnaryExpNode> right; // 右操作数或单独的unaryExp

    MulExpNode() : ExpNode("MulExp"), op(BinaryOp::MUL) {}
};

/**
 * @brief 加法表达式节点 - addExp -> mulExp | addExp ('+' | '-') mulExp
 */
class AddExpNode : public ExpNode {
public:
    std::shared_ptr<AddExpNode> left;   // 左操作数（可选）
    BinaryOp op;                        // 运算符 (ADD, SUB)
    std::shared_ptr<MulExpNode> right;  // 右操作数或单独的mulExp

    AddExpNode() : ExpNode("AddExp"), op(BinaryOp::ADD) {}
};

/**
 * @brief 关系表达式节点 - relExp -> addExp | relExp ('<' | '>' | '<=' | '>=') addExp
 */
class RelExpNode : public ExpNode {
public:
    std::shared_ptr<RelExpNode> left;   // 左操作数（可选）
    RelOp op;                           // 关系运算符
    std::shared_ptr<AddExpNode> right;  // 右操作数或单独的addExp

    RelExpNode() : ExpNode("RelExp"), op(RelOp::LT) {}
};

/**
 * @brief 相等表达式节点 - eqExp -> relExp | eqExp ('==' | '!=') relExp
 */
class EqExpNode : public ExpNode {
public:
    std::shared_ptr<EqExpNode> left;    // 左操作数（可选）
    EqOp op;                            // 相等运算符
    std::shared_ptr<RelExpNode> right;  // 右操作数或单独的relExp

    EqExpNode() : ExpNode("EqExp"), op(EqOp::EQ) {}
};

/**
 * @brief 逻辑与表达式节点 - lAndExp -> eqExp | lAndExp '&&' eqExp
 */
class LAndExpNode : public ExpNode {
public:
    std::shared_ptr<LAndExpNode> left;  // 左操作数（可选）
    std::shared_ptr<EqExpNode> right;   // 右操作数或单独的eqExp

    LAndExpNode() : ExpNode("LAndExp") {}
};

/**
 * @brief 逻辑或表达式节点 - lOrExp -> lAndExp | lOrExp '||' lAndExp
 */
class LOrExpNode : public ExpNode {
public:
    std::shared_ptr<LOrExpNode> left;   // 左操作数（可选）
    std::shared_ptr<LAndExpNode> right; // 右操作数或单独的lAndExp

    LOrExpNode() : ExpNode("LOrExp") {}
};

#endif // SYSYC_AST_H
