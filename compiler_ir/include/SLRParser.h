/*!
 * @file SLRParser.h
 * @brief SLR Parser with AST generation (adapted for compiler_ir)
 * @version 1.0.0
 * @date 2024
 */

#ifndef SYSYC_SLRPARSER_H
#define SYSYC_SLRPARSER_H

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <set>
#include <stack>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <unordered_set>
#include "Lexer.h"
#include "AST.h"

// Forward declarations
class SLRParser;

// Production structure
struct Production {
    int id;
    std::string lhs;
    std::vector<std::string> rhs;
};

// LR(0) Item structure
struct Item {
    int prodId;
    int dotPos;
    bool operator<(const Item& other) const {
        if (prodId != other.prodId) return prodId < other.prodId;
        return dotPos < other.dotPos;
    }
    bool operator==(const Item& other) const {
        return prodId == other.prodId && dotPos == other.dotPos;
    }
};

// Action types
enum ActionType { ACC, SHIFT, REDUCE, ERR };

struct Action {
    ActionType type;
    int target;
};

// Semantic value - can hold AST node pointers
struct SemanticValue {
    std::string terminal;  // For terminals
    
    // AST nodes
    std::shared_ptr<CompUnitNode> compUnit;
    std::shared_ptr<DeclNode> decl;
    std::shared_ptr<ConstDeclNode> constDecl;
    std::shared_ptr<VarDeclNode> varDecl;
    std::shared_ptr<ConstDefNode> constDef;
    std::shared_ptr<VarDefNode> varDef;
    std::shared_ptr<FuncDefNode> funcDef;
    std::shared_ptr<FuncFParamNode> funcFParam;
    std::shared_ptr<BlockNode> block;
    std::shared_ptr<BlockItemNode> blockItem;
    std::shared_ptr<StmtNode> stmt;
    std::shared_ptr<LValNode> lVal;
    std::shared_ptr<AddExpNode> exp;
    std::shared_ptr<CondNode> cond;
    std::shared_ptr<PrimaryExpNode> primaryExp;
    std::shared_ptr<UnaryExpNode> unaryExp;
    std::shared_ptr<MulExpNode> mulExp;
    std::shared_ptr<AddExpNode> addExp;
    std::shared_ptr<RelExpNode> relExp;
    std::shared_ptr<EqExpNode> eqExp;
    std::shared_ptr<LAndExpNode> lAndExp;
    std::shared_ptr<LOrExpNode> lOrExp;
    std::shared_ptr<NumberNode> number;
    
    BType bType;
    UnaryOp unaryOp;
    
    // Lists
    std::vector<std::shared_ptr<ConstDefNode>> constDefList;
    std::vector<std::shared_ptr<VarDefNode>> varDefList;
    std::vector<std::shared_ptr<FuncFParamNode>> funcFParams;
    std::vector<std::shared_ptr<BlockItemNode>> blockItemList;
    std::vector<std::shared_ptr<ExpNode>> funcRParams;
    
    SemanticValue() : bType(BType::INT), unaryOp(UnaryOp::PLUS) {}
};

class SLRParser {
private:
    std::vector<Production> grammar;
    std::map<std::string, std::set<std::string>> first;
    std::map<std::string, std::set<std::string>> follow;
    std::vector<std::set<Item>> canonicalCollection;
    std::map<std::pair<int, std::string>, Action> actionTable;
    std::map<std::pair<int, std::string>, int> gotoTable;
    std::set<std::string> terminals;
    std::set<std::string> nonTerminals;
    
    std::shared_ptr<CompUnitNode> astRoot;
    bool hasError;
    std::stringstream parseLog;
    int logStep;

public:
    SLRParser() : hasError(false), logStep(1) {
        initGrammar();
        computeFirst();
        computeFollow();
        buildCollection();
        buildTable();
    }
    
    bool parse(const std::vector<Token>& tokens);
    std::shared_ptr<CompUnitNode> getAST() const { return astRoot; }
    std::string getParseLog() const { return parseLog.str(); }
    void saveParseLog(const std::string& filepath) const;
    
private:
    void initGrammar();
    void computeFirst();
    void computeFollow();
    void buildCollection();
    void buildTable();
    
    std::set<Item> closure(std::set<Item> I);
    std::set<Item> gotoState(std::set<Item> I, std::string X);
    std::string getTokenSymbol(const Token& t);
    
    // Semantic actions for AST construction
    SemanticValue reduce(int prodId, std::vector<SemanticValue>& values);
    bool shouldLogSymbol(const std::string& symbol) const;
};

#endif // SYSYC_SLRPARSER_H
