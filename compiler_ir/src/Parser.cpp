/*!
 * @file Parser.cpp
 * @brief 递归下降语法分析器实现
 * @version 2.0.0
 * @date 2024
 * 
 * 实现递归下降分析器，构建AST
 */

#include "Parser.h"
#include <stdexcept>

// ==================== 构造函数 ====================

Parser::Parser() : tokenIndex(0), stepCount(0), hasError(false) {
}

// ==================== Token操作 ====================

Token Parser::currentToken() const {
    if (tokenIndex < tokens.size()) {
        return tokens[tokenIndex];
    }
    return Token(TokenType::END_OF_FILE, "$", 0, 0);
}

Token Parser::peek(int offset) const {
    size_t idx = tokenIndex + offset;
    if (idx < tokens.size()) {
        return tokens[idx];
    }
    return Token(TokenType::END_OF_FILE, "$", 0, 0);
}

void Parser::advance() {
    if (tokenIndex < tokens.size()) {
        tokenIndex++;
    }
}

bool Parser::match(TokenType type) {
    if (currentToken().type == type) {
        stepCount++;
        parseLog << stepCount << "\t" << currentToken().value << "\tmove" << std::endl;
        advance();
        return true;
    }
    return false;
}

bool Parser::expect(TokenType type, const std::string& msg) {
    if (!match(type)) {
        error(msg + ", got '" + currentToken().value + "'");
        return false;
    }
    return true;
}

void Parser::error(const std::string& msg) {
    hasError = true;
    Token tok = currentToken();
    parseLog << stepCount << "\terror: " << msg << " at line " << tok.line << std::endl;
    std::cerr << "Syntax error at line " << tok.line << ", column " << tok.column 
              << ": " << msg << std::endl;
}

bool Parser::isType(TokenType type) {
    return type == TokenType::KW_INT || type == TokenType::KW_FLOAT || type == TokenType::KW_VOID;
}

// ==================== 主解析函数 ====================

bool Parser::parse(const std::vector<Token>& tokenList) {
    tokens = tokenList;
    tokenIndex = 0;
    stepCount = 0;
    hasError = false;
    parseLog.str("");
    astRoot = nullptr;
    
    // 检查词法错误
    for (const auto& tok : tokens) {
        if (tok.type == TokenType::ERROR) {
            error("lexical error: illegal character '" + tok.value + "'");
            return false;
        }
    }
    
    astRoot = parseCompUnit();
    
    if (!hasError && currentToken().type != TokenType::END_OF_FILE) {
        error("unexpected token after program end");
    }
    
    if (!hasError) {
        parseLog << stepCount + 1 << "\t$#$\taccept" << std::endl;
    }
    
    return !hasError;
}

std::shared_ptr<CompUnitNode> Parser::getAST() const {
    return astRoot;
}

// ==================== 语法分析函数 ====================

std::shared_ptr<CompUnitNode> Parser::parseCompUnit() {
    auto compUnit = std::make_shared<CompUnitNode>();
    
    while (currentToken().type != TokenType::END_OF_FILE && !hasError) {
        // 判断是声明还是函数定义
        // 声明: BType Ident ;  或  BType Ident = ...  或  const BType Ident = ...
        // 函数: BType/void Ident ( ...
        
        if (currentToken().type == TokenType::KW_CONST) {
            // 常量声明
            auto decl = parseConstDecl();
            if (decl) compUnit->decls.push_back(decl);
        } else if (isType(currentToken().type)) {
            // 检查是函数还是变量声明
            // 向前看: Type Ident ( -> 函数
            //         Type Ident ; -> 变量
            //         Type Ident = -> 变量
            if (peek(1).type == TokenType::IDN && peek(2).type == TokenType::SE_LPAREN) {
                // 函数定义
                auto funcDef = parseFuncDef();
                if (funcDef) compUnit->funcDefs.push_back(funcDef);
            } else {
                // 变量声明
                auto decl = parseVarDecl();
                if (decl) compUnit->decls.push_back(decl);
            }
        } else {
            error("expected declaration or function definition");
            advance(); // 跳过错误token
        }
    }
    
    return compUnit;
}

std::shared_ptr<ConstDeclNode> Parser::parseConstDecl() {
    auto decl = std::make_shared<ConstDeclNode>();
    
    if (!expect(TokenType::KW_CONST, "expected 'const'")) return nullptr;
    
    // BType
    if (currentToken().type == TokenType::KW_INT) {
        decl->bType = BType::INT;
        advance();
    } else if (currentToken().type == TokenType::KW_FLOAT) {
        decl->bType = BType::FLOAT;
        advance();
    } else {
        error("expected type specifier");
        return nullptr;
    }
    
    // ConstDef
    auto constDef = parseConstDef();
    if (constDef) decl->constDefs.push_back(constDef);
    
    // 多个ConstDef
    while (currentToken().type == TokenType::SE_COMMA && !hasError) {
        advance();
        auto def = parseConstDef();
        if (def) decl->constDefs.push_back(def);
    }
    
    if (!expect(TokenType::SE_SEMI, "expected ';'")) return nullptr;
    
    parseLog << stepCount << "\tConstDecl\treduction" << std::endl;
    return decl;
}

std::shared_ptr<ConstDefNode> Parser::parseConstDef() {
    auto def = std::make_shared<ConstDefNode>();
    
    if (currentToken().type != TokenType::IDN) {
        error("expected identifier");
        return nullptr;
    }
    def->ident = currentToken().value;
    advance();
    
    if (!expect(TokenType::OP_ASSIGN, "expected '='")) return nullptr;
    
    def->initVal = parseExp();
    
    parseLog << stepCount << "\tConstDef\treduction" << std::endl;
    return def;
}

std::shared_ptr<VarDeclNode> Parser::parseVarDecl() {
    auto decl = std::make_shared<VarDeclNode>();
    
    // BType
    if (currentToken().type == TokenType::KW_INT) {
        decl->bType = BType::INT;
        advance();
    } else if (currentToken().type == TokenType::KW_FLOAT) {
        decl->bType = BType::FLOAT;
        advance();
    } else {
        error("expected type specifier");
        return nullptr;
    }
    
    // VarDef
    auto varDef = parseVarDef();
    if (varDef) decl->varDefs.push_back(varDef);
    
    // 多个VarDef
    while (currentToken().type == TokenType::SE_COMMA && !hasError) {
        advance();
        auto def = parseVarDef();
        if (def) decl->varDefs.push_back(def);
    }
    
    if (!expect(TokenType::SE_SEMI, "expected ';'")) return nullptr;
    
    parseLog << stepCount << "\tVarDecl\treduction" << std::endl;
    return decl;
}

std::shared_ptr<VarDefNode> Parser::parseVarDef() {
    auto def = std::make_shared<VarDefNode>();
    
    if (currentToken().type != TokenType::IDN) {
        error("expected identifier");
        return nullptr;
    }
    def->ident = currentToken().value;
    advance();
    
    // 可选的初始化
    if (currentToken().type == TokenType::OP_ASSIGN) {
        advance();
        def->initVal = parseExp();
    }
    
    parseLog << stepCount << "\tVarDef\treduction" << std::endl;
    return def;
}

std::shared_ptr<FuncDefNode> Parser::parseFuncDef() {
    auto func = std::make_shared<FuncDefNode>();
    
    // 返回类型
    if (currentToken().type == TokenType::KW_INT) {
        func->returnType = BType::INT;
        advance();
    } else if (currentToken().type == TokenType::KW_FLOAT) {
        func->returnType = BType::FLOAT;
        advance();
    } else if (currentToken().type == TokenType::KW_VOID) {
        func->returnType = BType::VOID;
        advance();
    } else {
        error("expected return type");
        return nullptr;
    }
    
    // 函数名
    if (currentToken().type != TokenType::IDN) {
        error("expected function name");
        return nullptr;
    }
    func->ident = currentToken().value;
    advance();
    
    if (!expect(TokenType::SE_LPAREN, "expected '('")) return nullptr;
    
    // 参数列表
    if (currentToken().type != TokenType::SE_RPAREN) {
        parseFuncFParams(func->params);
    }
    
    if (!expect(TokenType::SE_RPAREN, "expected ')'")) return nullptr;
    
    // 函数体
    func->block = parseBlock();
    
    parseLog << stepCount << "\tFuncDef\treduction" << std::endl;
    return func;
}

void Parser::parseFuncFParams(std::vector<std::shared_ptr<FuncFParamNode>>& params) {
    auto param = parseFuncFParam();
    if (param) params.push_back(param);
    
    while (currentToken().type == TokenType::SE_COMMA && !hasError) {
        advance();
        auto p = parseFuncFParam();
        if (p) params.push_back(p);
    }
}

std::shared_ptr<FuncFParamNode> Parser::parseFuncFParam() {
    auto param = std::make_shared<FuncFParamNode>();
    
    if (currentToken().type == TokenType::KW_INT) {
        param->bType = BType::INT;
        advance();
    } else if (currentToken().type == TokenType::KW_FLOAT) {
        param->bType = BType::FLOAT;
        advance();
    } else {
        error("expected parameter type");
        return nullptr;
    }
    
    if (currentToken().type != TokenType::IDN) {
        error("expected parameter name");
        return nullptr;
    }
    param->ident = currentToken().value;
    advance();
    
    return param;
}

std::shared_ptr<BlockNode> Parser::parseBlock() {
    auto block = std::make_shared<BlockNode>();
    
    if (!expect(TokenType::SE_LBRACE, "expected '{'")) return nullptr;
    
    while (currentToken().type != TokenType::SE_RBRACE && 
           currentToken().type != TokenType::END_OF_FILE && !hasError) {
        auto item = parseBlockItem();
        if (item) block->items.push_back(item);
    }
    
    if (!expect(TokenType::SE_RBRACE, "expected '}'")) return nullptr;
    
    parseLog << stepCount << "\tBlock\treduction" << std::endl;
    return block;
}

std::shared_ptr<BlockItemNode> Parser::parseBlockItem() {
    auto item = std::make_shared<BlockItemNode>();
    
    if (currentToken().type == TokenType::KW_CONST) {
        item->decl = parseConstDecl();
    } else if (currentToken().type == TokenType::KW_INT || 
               currentToken().type == TokenType::KW_FLOAT) {
        item->decl = parseVarDecl();
    } else {
        item->stmt = parseStmt();
    }
    
    return item;
}

std::shared_ptr<StmtNode> Parser::parseStmt() {
    auto stmt = std::make_shared<StmtNode>();
    
    if (currentToken().type == TokenType::SE_LBRACE) {
        // Block
        stmt->stmtType = StmtType::BLOCK;
        stmt->block = parseBlock();
    } else if (currentToken().type == TokenType::KW_IF) {
        // if语句
        stmt->stmtType = StmtType::IF;
        advance();
        if (!expect(TokenType::SE_LPAREN, "expected '('")) return nullptr;
        stmt->cond = parseCond();
        if (!expect(TokenType::SE_RPAREN, "expected ')'")) return nullptr;
        stmt->thenStmt = parseStmt();
        if (currentToken().type == TokenType::KW_ELSE) {
            advance();
            stmt->elseStmt = parseStmt();
        }
    } else if (currentToken().type == TokenType::KW_RETURN) {
        // return语句
        stmt->stmtType = StmtType::RETURN;
        advance();
        if (currentToken().type != TokenType::SE_SEMI) {
            stmt->exp = parseExp();
        }
        if (!expect(TokenType::SE_SEMI, "expected ';'")) return nullptr;
    } else if (currentToken().type == TokenType::SE_SEMI) {
        // 空语句
        stmt->stmtType = StmtType::EXP;
        advance();
    } else {
        // 表达式语句或赋值语句
        // 需要先解析表达式，然后看是否有 =
        auto exp = parseExp();
        
        if (currentToken().type == TokenType::OP_ASSIGN) {
            // 赋值语句: LVal = Exp
            stmt->stmtType = StmtType::ASSIGN;
            // 从exp中提取LVal
            if (auto primaryExp = std::dynamic_pointer_cast<PrimaryExpNode>(exp)) {
                stmt->lVal = primaryExp->lVal;
            } else if (auto unaryExp = std::dynamic_pointer_cast<UnaryExpNode>(exp)) {
                if (unaryExp->primaryExp && unaryExp->primaryExp->lVal) {
                    stmt->lVal = unaryExp->primaryExp->lVal;
                }
            } else if (auto mulExp = std::dynamic_pointer_cast<MulExpNode>(exp)) {
                if (mulExp->right && mulExp->right->primaryExp && mulExp->right->primaryExp->lVal) {
                    stmt->lVal = mulExp->right->primaryExp->lVal;
                }
            } else if (auto addExp = std::dynamic_pointer_cast<AddExpNode>(exp)) {
                if (addExp->right && addExp->right->right && 
                    addExp->right->right->primaryExp && addExp->right->right->primaryExp->lVal) {
                    stmt->lVal = addExp->right->right->primaryExp->lVal;
                }
            }
            
            if (!stmt->lVal) {
                // 创建LVal
                stmt->lVal = std::make_shared<LValNode>();
                // 尝试从表达式中获取identifier
            }
            
            advance(); // skip =
            stmt->exp = parseExp();
        } else {
            // 表达式语句
            stmt->stmtType = StmtType::EXP;
            stmt->exp = exp;
        }
        
        if (!expect(TokenType::SE_SEMI, "expected ';'")) return nullptr;
    }
    
    parseLog << stepCount << "\tStmt\treduction" << std::endl;
    return stmt;
}

std::shared_ptr<CondNode> Parser::parseCond() {
    auto cond = std::make_shared<CondNode>();
    cond->lOrExp = parseLOrExp();
    return cond;
}

std::shared_ptr<AddExpNode> Parser::parseExp() {
    return parseAddExp();
}

std::shared_ptr<LValNode> Parser::parseLVal() {
    auto lVal = std::make_shared<LValNode>();
    
    if (currentToken().type != TokenType::IDN) {
        error("expected identifier");
        return nullptr;
    }
    lVal->ident = currentToken().value;
    advance();
    
    return lVal;
}

std::shared_ptr<PrimaryExpNode> Parser::parsePrimaryExp() {
    auto primary = std::make_shared<PrimaryExpNode>();
    
    if (currentToken().type == TokenType::SE_LPAREN) {
        // ( Exp )
        advance();
        primary->primaryType = PrimaryExpNode::PrimaryType::PAREN_EXP;
        primary->exp = parseExp();
        if (!expect(TokenType::SE_RPAREN, "expected ')'")) return nullptr;
    } else if (currentToken().type == TokenType::INT) {
        // 整数
        primary->primaryType = PrimaryExpNode::PrimaryType::NUMBER;
        primary->number = std::make_shared<NumberNode>();
        primary->number->isFloat = false;
        primary->number->intVal = std::stoi(currentToken().value);
        advance();
    } else if (currentToken().type == TokenType::FLOAT) {
        // 浮点数
        primary->primaryType = PrimaryExpNode::PrimaryType::NUMBER;
        primary->number = std::make_shared<NumberNode>();
        primary->number->isFloat = true;
        primary->number->floatVal = std::stof(currentToken().value);
        advance();
    } else if (currentToken().type == TokenType::IDN) {
        // LVal
        primary->primaryType = PrimaryExpNode::PrimaryType::LVAL;
        primary->lVal = parseLVal();
    } else {
        error("expected expression");
        return nullptr;
    }
    
    return primary;
}

std::shared_ptr<UnaryExpNode> Parser::parseUnaryExp() {
    auto unary = std::make_shared<UnaryExpNode>();
    
    if (currentToken().type == TokenType::OP_PLUS ||
        currentToken().type == TokenType::OP_MINUS ||
        currentToken().type == TokenType::OP_NOT) {
        // UnaryOp UnaryExp
        unary->unaryType = UnaryExpNode::UnaryType::UNARY_OP;
        if (currentToken().type == TokenType::OP_PLUS) {
            unary->unaryOp = UnaryOp::PLUS;
        } else if (currentToken().type == TokenType::OP_MINUS) {
            unary->unaryOp = UnaryOp::MINUS;
        } else {
            unary->unaryOp = UnaryOp::NOT;
        }
        advance();
        unary->unaryExp = parseUnaryExp();
    } else if (currentToken().type == TokenType::IDN && peek(1).type == TokenType::SE_LPAREN) {
        // 函数调用
        unary->unaryType = UnaryExpNode::UnaryType::FUNC_CALL;
        unary->funcName = currentToken().value;
        advance(); // skip function name
        advance(); // skip (
        
        // 参数列表
        if (currentToken().type != TokenType::SE_RPAREN) {
            auto arg = parseExp();
            if (arg) unary->args.push_back(arg);
            
            while (currentToken().type == TokenType::SE_COMMA && !hasError) {
                advance();
                auto a = parseExp();
                if (a) unary->args.push_back(a);
            }
        }
        
        if (!expect(TokenType::SE_RPAREN, "expected ')'")) return nullptr;
    } else {
        // PrimaryExp
        unary->unaryType = UnaryExpNode::UnaryType::PRIMARY;
        unary->primaryExp = parsePrimaryExp();
    }
    
    return unary;
}

std::shared_ptr<MulExpNode> Parser::parseMulExp() {
    auto left = std::make_shared<MulExpNode>();
    left->left = nullptr;
    left->right = parseUnaryExp();
    
    while ((currentToken().type == TokenType::OP_MUL ||
            currentToken().type == TokenType::OP_DIV ||
            currentToken().type == TokenType::OP_MOD) && !hasError) {
        auto newNode = std::make_shared<MulExpNode>();
        newNode->left = left;
        if (currentToken().type == TokenType::OP_MUL) {
            newNode->op = BinaryOp::MUL;
        } else if (currentToken().type == TokenType::OP_DIV) {
            newNode->op = BinaryOp::DIV;
        } else {
            newNode->op = BinaryOp::MOD;
        }
        advance();
        newNode->right = parseUnaryExp();
        left = newNode;
    }
    
    return left;
}

std::shared_ptr<AddExpNode> Parser::parseAddExp() {
    auto left = std::make_shared<AddExpNode>();
    left->left = nullptr;
    left->right = parseMulExp();
    
    while ((currentToken().type == TokenType::OP_PLUS ||
            currentToken().type == TokenType::OP_MINUS) && !hasError) {
        auto newNode = std::make_shared<AddExpNode>();
        newNode->left = left;
        if (currentToken().type == TokenType::OP_PLUS) {
            newNode->op = BinaryOp::ADD;
        } else {
            newNode->op = BinaryOp::SUB;
        }
        advance();
        newNode->right = parseMulExp();
        left = newNode;
    }
    
    return left;
}

std::shared_ptr<RelExpNode> Parser::parseRelExp() {
    auto left = std::make_shared<RelExpNode>();
    left->left = nullptr;
    left->right = parseAddExp();
    
    while ((currentToken().type == TokenType::OP_LT ||
            currentToken().type == TokenType::OP_GT ||
            currentToken().type == TokenType::OP_LE ||
            currentToken().type == TokenType::OP_GE) && !hasError) {
        auto newNode = std::make_shared<RelExpNode>();
        newNode->left = left;
        if (currentToken().type == TokenType::OP_LT) {
            newNode->op = RelOp::LT;
        } else if (currentToken().type == TokenType::OP_GT) {
            newNode->op = RelOp::GT;
        } else if (currentToken().type == TokenType::OP_LE) {
            newNode->op = RelOp::LE;
        } else {
            newNode->op = RelOp::GE;
        }
        advance();
        newNode->right = parseAddExp();
        left = newNode;
    }
    
    return left;
}

std::shared_ptr<EqExpNode> Parser::parseEqExp() {
    auto left = std::make_shared<EqExpNode>();
    left->left = nullptr;
    left->right = parseRelExp();
    
    while ((currentToken().type == TokenType::OP_EQ ||
            currentToken().type == TokenType::OP_NE) && !hasError) {
        auto newNode = std::make_shared<EqExpNode>();
        newNode->left = left;
        if (currentToken().type == TokenType::OP_EQ) {
            newNode->op = EqOp::EQ;
        } else {
            newNode->op = EqOp::NE;
        }
        advance();
        newNode->right = parseRelExp();
        left = newNode;
    }
    
    return left;
}

std::shared_ptr<LAndExpNode> Parser::parseLAndExp() {
    auto left = std::make_shared<LAndExpNode>();
    left->left = nullptr;
    left->right = parseEqExp();
    
    while (currentToken().type == TokenType::OP_AND && !hasError) {
        auto newNode = std::make_shared<LAndExpNode>();
        newNode->left = left;
        advance();
        newNode->right = parseEqExp();
        left = newNode;
    }
    
    return left;
}

std::shared_ptr<LOrExpNode> Parser::parseLOrExp() {
    auto left = std::make_shared<LOrExpNode>();
    left->left = nullptr;
    left->right = parseLAndExp();
    
    while (currentToken().type == TokenType::OP_OR && !hasError) {
        auto newNode = std::make_shared<LOrExpNode>();
        newNode->left = left;
        advance();
        newNode->right = parseLAndExp();
        left = newNode;
    }
    
    return left;
}

// ==================== 打印函数 ====================

void Parser::printGrammar() const {
    std::cout << "=== C-- Grammar (Recursive Descent) ===" << std::endl;
}

void Parser::printFirstSets() const {
    std::cout << "=== Recursive Descent Parser (no explicit FIRST sets) ===" << std::endl;
}

void Parser::printFollowSets() const {
    std::cout << "=== Recursive Descent Parser (no explicit FOLLOW sets) ===" << std::endl;
}
