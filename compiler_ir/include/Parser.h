/*!
 * @file Parser.h
 * @brief 递归下降语法分析器头文件
 * @version 2.0.0
 * @date 2024
 */

#ifndef SYSYC_PARSER_H
#define SYSYC_PARSER_H

#include <string>
#include <vector>
#include <memory>
#include <sstream>
#include <iostream>
#include "Lexer.h"
#include "AST.h"

/**
 * @brief 递归下降语法分析器类
 */
class Parser {
private:
    std::vector<Token> tokens;
    size_t tokenIndex;
    std::stringstream parseLog;
    int stepCount;
    bool hasError;
    std::shared_ptr<CompUnitNode> astRoot;
    
public:
    Parser();
    
    bool parse(const std::vector<Token>& tokenList);
    std::string getParseLog() const { return parseLog.str(); }
    std::shared_ptr<CompUnitNode> getAST() const;
    
    void printGrammar() const;
    void printFirstSets() const;
    void printFollowSets() const;
    
private:
    Token currentToken() const;
    Token peek(int offset) const;
    void advance();
    bool match(TokenType type);
    bool expect(TokenType type, const std::string& msg);
    void error(const std::string& msg);
    bool isType(TokenType type);
    
    // 语法分析函数
    std::shared_ptr<CompUnitNode> parseCompUnit();
    std::shared_ptr<ConstDeclNode> parseConstDecl();
    std::shared_ptr<ConstDefNode> parseConstDef();
    std::shared_ptr<VarDeclNode> parseVarDecl();
    std::shared_ptr<VarDefNode> parseVarDef();
    std::shared_ptr<FuncDefNode> parseFuncDef();
    void parseFuncFParams(std::vector<std::shared_ptr<FuncFParamNode>>& params);
    std::shared_ptr<FuncFParamNode> parseFuncFParam();
    std::shared_ptr<BlockNode> parseBlock();
    std::shared_ptr<BlockItemNode> parseBlockItem();
    std::shared_ptr<StmtNode> parseStmt();
    std::shared_ptr<CondNode> parseCond();
    std::shared_ptr<AddExpNode> parseExp();
    std::shared_ptr<LValNode> parseLVal();
    std::shared_ptr<PrimaryExpNode> parsePrimaryExp();
    std::shared_ptr<UnaryExpNode> parseUnaryExp();
    std::shared_ptr<MulExpNode> parseMulExp();
    std::shared_ptr<AddExpNode> parseAddExp();
    std::shared_ptr<RelExpNode> parseRelExp();
    std::shared_ptr<EqExpNode> parseEqExp();
    std::shared_ptr<LAndExpNode> parseLAndExp();
    std::shared_ptr<LOrExpNode> parseLOrExp();
};

#endif // SYSYC_PARSER_H
