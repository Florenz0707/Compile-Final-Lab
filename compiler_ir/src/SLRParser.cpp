/*!
 * @file SLRParser.cpp
 * @brief SLR Parser implementation with AST generation
 * @version 1.0.0
 * @date 2024
 */

#include "SLRParser.h"
#include <cctype>

// Helper function to convert token to grammar symbol
std::string SLRParser::getTokenSymbol(const Token& t) {
    switch (t.type) {
        case TokenType::IDN: return "Ident";
        case TokenType::INT: return "IntConst";
        case TokenType::FLOAT: return "floatConst";
        case TokenType::KW_INT: return "int";
        case TokenType::KW_FLOAT: return "float";
        case TokenType::KW_VOID: return "void";
        case TokenType::KW_CONST: return "const";
        case TokenType::KW_RETURN: return "return";
        case TokenType::KW_IF: return "if";
        case TokenType::KW_ELSE: return "else";
        case TokenType::OP_PLUS: return "+";
        case TokenType::OP_MINUS: return "-";
        case TokenType::OP_MUL: return "*";
        case TokenType::OP_DIV: return "/";
        case TokenType::OP_MOD: return "%";
        case TokenType::OP_ASSIGN: return "=";
        case TokenType::OP_EQ: return "==";
        case TokenType::OP_NE: return "!=";
        case TokenType::OP_LT: return "<";
        case TokenType::OP_GT: return ">";
        case TokenType::OP_LE: return "<=";
        case TokenType::OP_GE: return ">=";
        case TokenType::OP_AND: return "&&";
        case TokenType::OP_OR: return "||";
        case TokenType::OP_NOT: return "!";
        case TokenType::SE_LPAREN: return "(";
        case TokenType::SE_RPAREN: return ")";
        case TokenType::SE_LBRACE: return "{";
        case TokenType::SE_RBRACE: return "}";
        case TokenType::SE_SEMI: return ";";
        case TokenType::SE_COMMA: return ",";
        case TokenType::END_OF_FILE: return "$";
        default: return "UNKNOWN";
    }
}

void SLRParser::initGrammar() {
    int id = 1;
    
    grammar.push_back({id++, "S'", {"Program"}});
    grammar.push_back({id++, "Program", {"compUnit"}});
    grammar.push_back({id++, "compUnit", {"compUnit", "element"}});
    grammar.push_back({id++, "compUnit", {"element"}});
    grammar.push_back({id++, "element", {"decl"}});
    grammar.push_back({id++, "element", {"funcDef"}});
    
    grammar.push_back({id++, "decl", {"constDecl"}});
    grammar.push_back({id++, "decl", {"varDecl"}});
    
    grammar.push_back({id++, "constDecl", {"const", "bType", "constDefList", ";"}});
    grammar.push_back({id++, "constDefList", {"constDefList", ",", "constDef"}});
    grammar.push_back({id++, "constDefList", {"constDef"}});
    
    grammar.push_back({id++, "bType", {"int"}});
    grammar.push_back({id++, "bType", {"float"}});
    
    grammar.push_back({id++, "constDef", {"Ident", "=", "constInitVal"}});
    grammar.push_back({id++, "constInitVal", {"constExp"}});
    
    grammar.push_back({id++, "varDecl", {"bType", "varDefList", ";"}});
    grammar.push_back({id++, "varDefList", {"varDefList", ",", "varDef"}});
    grammar.push_back({id++, "varDefList", {"varDef"}});
    
    grammar.push_back({id++, "varDef", {"Ident"}});
    grammar.push_back({id++, "varDef", {"Ident", "=", "initVal"}});
    grammar.push_back({id++, "initVal", {"exp"}});
    
    grammar.push_back({id++, "funcDef", {"funcType", "Ident", "(", ")", "block"}});
    grammar.push_back({id++, "funcDef", {"bType", "Ident", "(", ")", "block"}});
    grammar.push_back({id++, "funcDef", {"funcType", "Ident", "(", "funcFParams", ")", "block"}});
    grammar.push_back({id++, "funcDef", {"bType", "Ident", "(", "funcFParams", ")", "block"}});
    
    grammar.push_back({id++, "funcType", {"void"}});
    grammar.push_back({id++, "funcFParams", {"funcFParams", ",", "funcFParam"}});
    grammar.push_back({id++, "funcFParams", {"funcFParam"}});
    grammar.push_back({id++, "funcFParam", {"bType", "Ident"}});
    
    grammar.push_back({id++, "block", {"{", "blockItemList", "}"}});
    grammar.push_back({id++, "block", {"{", "}"}});
    grammar.push_back({id++, "blockItemList", {"blockItemList", "blockItem"}});
    grammar.push_back({id++, "blockItemList", {"blockItem"}});
    grammar.push_back({id++, "blockItem", {"decl"}});
    grammar.push_back({id++, "blockItem", {"stmt"}});
    
    grammar.push_back({id++, "stmt", {"lVal", "=", "exp", ";"}});
    grammar.push_back({id++, "stmt", {"exp", ";"}});
    grammar.push_back({id++, "stmt", {";"}});
    grammar.push_back({id++, "stmt", {"block"}});
    grammar.push_back({id++, "stmt", {"if", "(", "cond", ")", "stmt", "ElsePart"}});
    grammar.push_back({id++, "stmt", {"return", "exp", ";"}});
    grammar.push_back({id++, "stmt", {"return", ";"}});
    
    grammar.push_back({id++, "ElsePart", {"else", "stmt"}});
    grammar.push_back({id++, "ElsePart", {"epsilon"}});
    
    grammar.push_back({id++, "lVal", {"Ident"}});
    grammar.push_back({id++, "exp", {"lOrExp"}});
    grammar.push_back({id++, "lOrExp", {"lAndExp"}});
    grammar.push_back({id++, "lOrExp", {"lOrExp", "||", "lAndExp"}});
    grammar.push_back({id++, "lAndExp", {"eqExp"}});
    grammar.push_back({id++, "lAndExp", {"lAndExp", "&&", "eqExp"}});
    grammar.push_back({id++, "eqExp", {"relExp"}});
    grammar.push_back({id++, "eqExp", {"eqExp", "==", "relExp"}});
    grammar.push_back({id++, "eqExp", {"eqExp", "!=", "relExp"}});
    grammar.push_back({id++, "relExp", {"addExp"}});
    grammar.push_back({id++, "relExp", {"relExp", "<", "addExp"}});
    grammar.push_back({id++, "relExp", {"relExp", ">", "addExp"}});
    grammar.push_back({id++, "relExp", {"relExp", "<=", "addExp"}});
    grammar.push_back({id++, "relExp", {"relExp", ">=", "addExp"}});
    grammar.push_back({id++, "addExp", {"mulExp"}});
    grammar.push_back({id++, "addExp", {"addExp", "+", "mulExp"}});
    grammar.push_back({id++, "addExp", {"addExp", "-", "mulExp"}});
    grammar.push_back({id++, "mulExp", {"unaryExp"}});
    grammar.push_back({id++, "mulExp", {"mulExp", "*", "unaryExp"}});
    grammar.push_back({id++, "mulExp", {"mulExp", "/", "unaryExp"}});
    grammar.push_back({id++, "mulExp", {"mulExp", "%", "unaryExp"}});
    grammar.push_back({id++, "unaryExp", {"primaryExp"}});
    grammar.push_back({id++, "unaryExp", {"unaryOp", "unaryExp"}});
    grammar.push_back({id++, "unaryExp", {"Ident", "(", ")"}});
    grammar.push_back({id++, "unaryExp", {"Ident", "(", "funcRParams", ")"}});
    grammar.push_back({id++, "primaryExp", {"(", "exp", ")"}});
    grammar.push_back({id++, "primaryExp", {"lVal"}});
    grammar.push_back({id++, "primaryExp", {"number"}});
    grammar.push_back({id++, "number", {"IntConst"}});
    grammar.push_back({id++, "number", {"floatConst"}});
    grammar.push_back({id++, "unaryOp", {"+"}});
    grammar.push_back({id++, "unaryOp", {"-"}});
    grammar.push_back({id++, "unaryOp", {"!"}});
    grammar.push_back({id++, "funcRParams", {"exp", ",", "funcRParams"}});
    grammar.push_back({id++, "funcRParams", {"exp"}});
    grammar.push_back({id++, "constExp", {"addExp"}});
    grammar.push_back({id++, "cond", {"lOrExp"}});
    
    for (auto& p : grammar) {
        nonTerminals.insert(p.lhs);
        for (auto& s : p.rhs) if (s != "epsilon") terminals.insert(s);
    }
    for (auto& nt : nonTerminals) terminals.erase(nt);
    terminals.insert("$");
}

void SLRParser::computeFirst() {
    for (auto& t : terminals) first[t].insert(t);
    bool changed = true;
    while (changed) {
        changed = false;
        for (auto& p : grammar) {
            int before = first[p.lhs].size();
            if (p.rhs[0] == "epsilon") first[p.lhs].insert("epsilon");
            else {
                for (size_t i = 0; i < p.rhs.size(); i++) {
                    std::string Y = p.rhs[i];
                    bool hasEpsilon = false;
                    for (auto& f : first[Y]) {
                        if (f != "epsilon") first[p.lhs].insert(f);
                        else hasEpsilon = true;
                    }
                    if (!hasEpsilon) break;
                    if (i == p.rhs.size() - 1) first[p.lhs].insert("epsilon");
                }
            }
            if (first[p.lhs].size() > (size_t)before) changed = true;
        }
    }
}

void SLRParser::computeFollow() {
    follow["Program"].insert("$");
    bool changed = true;
    while (changed) {
        changed = false;
        for (auto& p : grammar) {
            for (size_t i = 0; i < p.rhs.size(); i++) {
                std::string B = p.rhs[i];
                if (nonTerminals.find(B) == nonTerminals.end()) continue;
                int before = follow[B].size();
                
                if (i + 1 < p.rhs.size()) {
                    std::string beta = p.rhs[i+1];
                    bool betaHasEpsilon = false;
                    for(auto &f : first[beta]) {
                        if(f != "epsilon") follow[B].insert(f);
                        else betaHasEpsilon = true;
                    }
                    if (betaHasEpsilon) {
                        for(auto &f : follow[p.lhs]) follow[B].insert(f);
                    }
                } else {
                    for(auto &f : follow[p.lhs]) follow[B].insert(f);
                }
                if (follow[B].size() > (size_t)before) changed = true;
            }
        }
    }
}

std::set<Item> SLRParser::closure(std::set<Item> I) {
    bool changed = true;
    while (changed) {
        changed = false;
        std::set<Item> temp = I;
        for (auto& item : I) {
            if (item.dotPos < (int)grammar[item.prodId - 1].rhs.size()) {
                std::string B = grammar[item.prodId - 1].rhs[item.dotPos];
                if (nonTerminals.count(B)) {
                    for (auto& p : grammar) {
                        if (p.lhs == B) {
                            Item newItem = {p.id, 0};
                            if (p.rhs[0] == "epsilon") newItem.dotPos = 1;
                            if (temp.find(newItem) == temp.end()) {
                                temp.insert(newItem);
                                changed = true;
                            }
                        }
                    }
                }
            }
        }
        I = temp;
    }
    return I;
}

std::set<Item> SLRParser::gotoState(std::set<Item> I, std::string X) {
    std::set<Item> J;
    for (auto& item : I) {
        if (item.dotPos < (int)grammar[item.prodId - 1].rhs.size()) {
            if (grammar[item.prodId - 1].rhs[item.dotPos] == X) {
                J.insert({item.prodId, item.dotPos + 1});
            }
        }
    }
    return closure(J);
}

void SLRParser::buildCollection() {
    canonicalCollection.push_back(closure({{1, 0}}));
    bool changed = true;
    while (changed) {
        changed = false;
        for (size_t i = 0; i < canonicalCollection.size(); i++) {
            std::set<std::string> nextSymbols;
            for (auto& item : canonicalCollection[i]) {
                if (item.dotPos < (int)grammar[item.prodId - 1].rhs.size()) {
                    std::string s = grammar[item.prodId - 1].rhs[item.dotPos];
                    if (s != "epsilon") nextSymbols.insert(s);
                }
            }
            for (auto& X : nextSymbols) {
                std::set<Item> nextI = gotoState(canonicalCollection[i], X);
                if (nextI.empty()) continue;
                
                int target = -1;
                for (size_t k = 0; k < canonicalCollection.size(); k++) {
                    if (canonicalCollection[k] == nextI) { target = k; break; }
                }
                if (target == -1) {
                    canonicalCollection.push_back(nextI);
                    target = canonicalCollection.size() - 1;
                    changed = true;
                }
                if (nonTerminals.count(X)) gotoTable[{i, X}] = target;
            }
        }
    }
}

void SLRParser::buildTable() {
    for (size_t i = 0; i < canonicalCollection.size(); i++) {
        for (auto& item : canonicalCollection[i]) {
            if (item.dotPos < (int)grammar[item.prodId - 1].rhs.size()) {
                std::string a = grammar[item.prodId - 1].rhs[item.dotPos];
                if (terminals.count(a)) {
                    std::set<Item> nextI = gotoState(canonicalCollection[i], a);
                    for(size_t k=0; k<canonicalCollection.size(); k++) {
                        if(canonicalCollection[k] == nextI) {
                            if(actionTable.count({i, a}) && actionTable[{i, a}].type == REDUCE) {
                                // Keep shift (shift-reduce conflict resolution)
                            }
                            actionTable[{i, a}] = {SHIFT, (int)k};
                            break;
                        }
                    }
                }
            } else {
                if (grammar[item.prodId - 1].lhs == "S'") {
                    actionTable[{i, "$"}] = {ACC, 0};
                } else {
                    for (auto& f : follow[grammar[item.prodId - 1].lhs]) {
                        if(actionTable.count({i, f}) && actionTable[{i, f}].type == SHIFT) {
                            continue; // Shift-reduce: prefer shift
                        }
                        actionTable[{i, f}] = {REDUCE, item.prodId};
                    }
                }
            }
        }
    }
}

bool SLRParser::parse(const std::vector<Token>& tokens) {
    std::vector<int> stateStack = {0};
    std::vector<SemanticValue> valueStack;
    
    size_t ip = 0;
    hasError = false;
    
    while (true) {
        int s = stateStack.back();
        std::string a;
        if (ip >= tokens.size()) {
            a = "$";
        } else {
            a = getTokenSymbol(tokens[ip]);
        }
        
        if (actionTable.find({s, a}) == actionTable.end()) {
            std::cerr << "Parse error at token: " << (ip < tokens.size() ? tokens[ip].value : "$") << std::endl;
            hasError = true;
            return false;
        }
        
        Action act = actionTable[{s, a}];
        
        if (act.type == SHIFT) {
            stateStack.push_back(act.target);
            SemanticValue val;
            if (ip < tokens.size()) {
                val.terminal = tokens[ip].value;
            }
            valueStack.push_back(val);
            ip++;
        } else if (act.type == REDUCE) {
            Production p = grammar[act.target - 1];
            int len = p.rhs.size();
            if (p.rhs[0] == "epsilon") len = 0;
            
            std::vector<SemanticValue> rhsValues;
            for (int k = 0; k < len; k++) {
                rhsValues.push_back(valueStack[valueStack.size() - len + k]);
            }
            
            for (int k = 0; k < len; k++) {
                stateStack.pop_back();
                valueStack.pop_back();
            }
            
            SemanticValue result = reduce(act.target, rhsValues);
            
            int t = stateStack.back();
            if (gotoTable.find({t, p.lhs}) == gotoTable.end()) {
                std::cerr << "Goto error" << std::endl;
                hasError = true;
                return false;
            }
            stateStack.push_back(gotoTable[{t, p.lhs}]);
            valueStack.push_back(result);
        } else if (act.type == ACC) {
            if (!valueStack.empty()) {
                astRoot = valueStack.back().compUnit;
            }
            return true;
        }
    }
}

// Semantic actions - this is where AST is constructed
SemanticValue SLRParser::reduce(int prodId, std::vector<SemanticValue>& vals) {
    SemanticValue result;
    Production p = grammar[prodId - 1];
    
    // S' -> Program
    if (prodId == 1) {
        result = vals[0];
    }
    // Program -> compUnit
    else if (prodId == 2) {
        result = vals[0];
    }
    // compUnit -> compUnit element
    else if (prodId == 3) {
        result = vals[0];
        if (vals[1].decl) {
            result.compUnit->decls.push_back(vals[1].decl);
        } else if (vals[1].funcDef) {
            result.compUnit->funcDefs.push_back(vals[1].funcDef);
        }
    }
    // compUnit -> element
    else if (prodId == 4) {
        result.compUnit = std::make_shared<CompUnitNode>();
        if (vals[0].decl) {
            result.compUnit->decls.push_back(vals[0].decl);
        } else if (vals[0].funcDef) {
            result.compUnit->funcDefs.push_back(vals[0].funcDef);
        }
    }
    // element -> decl
    else if (prodId == 5) {
        result = vals[0];
    }
    // element -> funcDef
    else if (prodId == 6) {
        result = vals[0];
    }
    // decl -> constDecl
    else if (prodId == 7) {
        result.decl = vals[0].constDecl;
    }
    // decl -> varDecl
    else if (prodId == 8) {
        result.decl = vals[0].varDecl;
    }
    // constDecl -> const bType constDefList ;
    else if (prodId == 9) {
        result.constDecl = std::make_shared<ConstDeclNode>();
        result.constDecl->bType = vals[1].bType;
        result.constDecl->constDefs = vals[2].constDefList;
    }
    // constDefList -> constDefList , constDef
    else if (prodId == 10) {
        result.constDefList = vals[0].constDefList;
        result.constDefList.push_back(vals[2].constDef);
    }
    // constDefList -> constDef
    else if (prodId == 11) {
        result.constDefList.push_back(vals[0].constDef);
    }
    // bType -> int
    else if (prodId == 12) {
        result.bType = BType::INT;
    }
    // bType -> float
    else if (prodId == 13) {
        result.bType = BType::FLOAT;
    }
    // constDef -> Ident = constInitVal
    else if (prodId == 14) {
        result.constDef = std::make_shared<ConstDefNode>();
        result.constDef->ident = vals[0].terminal;
        result.constDef->initVal = vals[2].exp;
    }
    // constInitVal -> constExp
    else if (prodId == 15) {
        result.exp = vals[0].exp;
    }
    // varDecl -> bType varDefList ;
    else if (prodId == 16) {
        result.varDecl = std::make_shared<VarDeclNode>();
        result.varDecl->bType = vals[0].bType;
        result.varDecl->varDefs = vals[1].varDefList;
    }
    // varDefList -> varDefList , varDef
    else if (prodId == 17) {
        result.varDefList = vals[0].varDefList;
        result.varDefList.push_back(vals[2].varDef);
    }
    // varDefList -> varDef
    else if (prodId == 18) {
        result.varDefList.push_back(vals[0].varDef);
    }
    // varDef -> Ident
    else if (prodId == 19) {
        result.varDef = std::make_shared<VarDefNode>();
        result.varDef->ident = vals[0].terminal;
    }
    // varDef -> Ident = initVal
    else if (prodId == 20) {
        result.varDef = std::make_shared<VarDefNode>();
        result.varDef->ident = vals[0].terminal;
        result.varDef->initVal = vals[2].exp;
    }
    // initVal -> exp
    else if (prodId == 21) {
        result.exp = vals[0].exp;
    }
    // funcDef -> funcType Ident ( ) block
    else if (prodId == 22) {
        result.funcDef = std::make_shared<FuncDefNode>();
        result.funcDef->returnType = BType::VOID;
        result.funcDef->ident = vals[1].terminal;
        result.funcDef->block = vals[4].block;
    }
    // funcDef -> bType Ident ( ) block
    else if (prodId == 23) {
        result.funcDef = std::make_shared<FuncDefNode>();
        result.funcDef->returnType = vals[0].bType;
        result.funcDef->ident = vals[1].terminal;
        result.funcDef->block = vals[4].block;
    }
    // funcDef -> funcType Ident ( funcFParams ) block
    else if (prodId == 24) {
        result.funcDef = std::make_shared<FuncDefNode>();
        result.funcDef->returnType = BType::VOID;
        result.funcDef->ident = vals[1].terminal;
        result.funcDef->params = vals[3].funcFParams;
        result.funcDef->block = vals[5].block;
    }
    // funcDef -> bType Ident ( funcFParams ) block
    else if (prodId == 25) {
        result.funcDef = std::make_shared<FuncDefNode>();
        result.funcDef->returnType = vals[0].bType;
        result.funcDef->ident = vals[1].terminal;
        result.funcDef->params = vals[3].funcFParams;
        result.funcDef->block = vals[5].block;
    }
    // funcType -> void
    else if (prodId == 26) {
        result.bType = BType::VOID;
    }
    // funcFParams -> funcFParams , funcFParam
    else if (prodId == 27) {
        result.funcFParams = vals[0].funcFParams;
        result.funcFParams.push_back(vals[2].funcFParam);
    }
    // funcFParams -> funcFParam
    else if (prodId == 28) {
        result.funcFParams.push_back(vals[0].funcFParam);
    }
    // funcFParam -> bType Ident
    else if (prodId == 29) {
        result.funcFParam = std::make_shared<FuncFParamNode>();
        result.funcFParam->bType = vals[0].bType;
        result.funcFParam->ident = vals[1].terminal;
    }
    // block -> { blockItemList }
    else if (prodId == 30) {
        result.block = std::make_shared<BlockNode>();
        result.block->items = vals[1].blockItemList;
    }
    // block -> { }
    else if (prodId == 31) {
        result.block = std::make_shared<BlockNode>();
    }
    // blockItemList -> blockItemList blockItem
    else if (prodId == 32) {
        result.blockItemList = vals[0].blockItemList;
        result.blockItemList.push_back(vals[1].blockItem);
    }
    // blockItemList -> blockItem
    else if (prodId == 33) {
        result.blockItemList.push_back(vals[0].blockItem);
    }
    // blockItem -> decl
    else if (prodId == 34) {
        result.blockItem = std::make_shared<BlockItemNode>();
        result.blockItem->decl = vals[0].decl;
    }
    // blockItem -> stmt
    else if (prodId == 35) {
        result.blockItem = std::make_shared<BlockItemNode>();
        result.blockItem->stmt = vals[0].stmt;
    }
    // stmt -> lVal = exp ;
    else if (prodId == 36) {
        result.stmt = std::make_shared<StmtNode>();
        result.stmt->stmtType = StmtType::ASSIGN;
        result.stmt->lVal = vals[0].lVal;
        result.stmt->exp = vals[2].exp;
    }
    // stmt -> exp ;
    else if (prodId == 37) {
        result.stmt = std::make_shared<StmtNode>();
        result.stmt->stmtType = StmtType::EXP;
        result.stmt->exp = vals[0].exp;
    }
    // stmt -> ;
    else if (prodId == 38) {
        result.stmt = std::make_shared<StmtNode>();
        result.stmt->stmtType = StmtType::EXP;
    }
    // stmt -> block
    else if (prodId == 39) {
        result.stmt = std::make_shared<StmtNode>();
        result.stmt->stmtType = StmtType::BLOCK;
        result.stmt->block = vals[0].block;
    }
    // stmt -> if ( cond ) stmt ElsePart
    else if (prodId == 40) {
        result.stmt = std::make_shared<StmtNode>();
        result.stmt->stmtType = StmtType::IF;
        result.stmt->cond = vals[2].cond;
        result.stmt->thenStmt = vals[4].stmt;
        result.stmt->elseStmt = vals[5].stmt;
    }
    // stmt -> return exp ;
    else if (prodId == 41) {
        result.stmt = std::make_shared<StmtNode>();
        result.stmt->stmtType = StmtType::RETURN;
        result.stmt->exp = vals[1].exp;
    }
    // stmt -> return ;
    else if (prodId == 42) {
        result.stmt = std::make_shared<StmtNode>();
        result.stmt->stmtType = StmtType::RETURN;
    }
    // ElsePart -> else stmt
    else if (prodId == 43) {
        result.stmt = vals[1].stmt;
    }
    // ElsePart -> epsilon
    else if (prodId == 44) {
        result.stmt = nullptr;
    }
    // lVal -> Ident
    else if (prodId == 45) {
        result.lVal = std::make_shared<LValNode>();
        result.lVal->ident = vals[0].terminal;
    }
    // exp -> lOrExp
    else if (prodId == 46) {
        result.exp = std::make_shared<AddExpNode>();
        result.exp->left = nullptr;
        auto mulExp = std::make_shared<MulExpNode>();
        mulExp->left = nullptr;
        auto unaryExp = std::make_shared<UnaryExpNode>();
        unaryExp->unaryType = UnaryExpNode::UnaryType::PRIMARY;
        auto primaryExp = std::make_shared<PrimaryExpNode>();
        primaryExp->primaryType = PrimaryExpNode::PrimaryType::PAREN_EXP;
        primaryExp->exp = vals[0].lOrExp;
        unaryExp->primaryExp = primaryExp;
        mulExp->right = unaryExp;
        result.exp->right = mulExp;
    }
    // lOrExp -> lAndExp
    else if (prodId == 47) {
        result.lOrExp = std::make_shared<LOrExpNode>();
        result.lOrExp->left = nullptr;
        result.lOrExp->right = vals[0].lAndExp;
    }
    // lOrExp -> lOrExp || lAndExp
    else if (prodId == 48) {
        result.lOrExp = std::make_shared<LOrExpNode>();
        result.lOrExp->left = vals[0].lOrExp;
        result.lOrExp->right = vals[2].lAndExp;
    }
    // lAndExp -> eqExp
    else if (prodId == 49) {
        result.lAndExp = std::make_shared<LAndExpNode>();
        result.lAndExp->left = nullptr;
        result.lAndExp->right = vals[0].eqExp;
    }
    // lAndExp -> lAndExp && eqExp
    else if (prodId == 50) {
        result.lAndExp = std::make_shared<LAndExpNode>();
        result.lAndExp->left = vals[0].lAndExp;
        result.lAndExp->right = vals[2].eqExp;
    }
    // eqExp -> relExp
    else if (prodId == 51) {
        result.eqExp = std::make_shared<EqExpNode>();
        result.eqExp->left = nullptr;
        result.eqExp->right = vals[0].relExp;
    }
    // eqExp -> eqExp == relExp
    else if (prodId == 52) {
        result.eqExp = std::make_shared<EqExpNode>();
        result.eqExp->left = vals[0].eqExp;
        result.eqExp->op = EqOp::EQ;
        result.eqExp->right = vals[2].relExp;
    }
    // eqExp -> eqExp != relExp
    else if (prodId == 53) {
        result.eqExp = std::make_shared<EqExpNode>();
        result.eqExp->left = vals[0].eqExp;
        result.eqExp->op = EqOp::NE;
        result.eqExp->right = vals[2].relExp;
    }
    // relExp -> addExp
    else if (prodId == 54) {
        result.relExp = std::make_shared<RelExpNode>();
        result.relExp->left = nullptr;
        result.relExp->right = vals[0].addExp;
    }
    // relExp -> relExp < addExp
    else if (prodId == 55) {
        result.relExp = std::make_shared<RelExpNode>();
        result.relExp->left = vals[0].relExp;
        result.relExp->op = RelOp::LT;
        result.relExp->right = vals[2].addExp;
    }
    // relExp -> relExp > addExp
    else if (prodId == 56) {
        result.relExp = std::make_shared<RelExpNode>();
        result.relExp->left = vals[0].relExp;
        result.relExp->op = RelOp::GT;
        result.relExp->right = vals[2].addExp;
    }
    // relExp -> relExp <= addExp
    else if (prodId == 57) {
        result.relExp = std::make_shared<RelExpNode>();
        result.relExp->left = vals[0].relExp;
        result.relExp->op = RelOp::LE;
        result.relExp->right = vals[2].addExp;
    }
    // relExp -> relExp >= addExp
    else if (prodId == 58) {
        result.relExp = std::make_shared<RelExpNode>();
        result.relExp->left = vals[0].relExp;
        result.relExp->op = RelOp::GE;
        result.relExp->right = vals[2].addExp;
    }
    // addExp -> mulExp
    else if (prodId == 59) {
        result.addExp = std::make_shared<AddExpNode>();
        result.addExp->left = nullptr;
        result.addExp->right = vals[0].mulExp;
    }
    // addExp -> addExp + mulExp
    else if (prodId == 60) {
        result.addExp = std::make_shared<AddExpNode>();
        result.addExp->left = vals[0].addExp;
        result.addExp->op = BinaryOp::ADD;
        result.addExp->right = vals[2].mulExp;
    }
    // addExp -> addExp - mulExp
    else if (prodId == 61) {
        result.addExp = std::make_shared<AddExpNode>();
        result.addExp->left = vals[0].addExp;
        result.addExp->op = BinaryOp::SUB;
        result.addExp->right = vals[2].mulExp;
    }
    // mulExp -> unaryExp
    else if (prodId == 62) {
        result.mulExp = std::make_shared<MulExpNode>();
        result.mulExp->left = nullptr;
        result.mulExp->right = vals[0].unaryExp;
    }
    // mulExp -> mulExp * unaryExp
    else if (prodId == 63) {
        result.mulExp = std::make_shared<MulExpNode>();
        result.mulExp->left = vals[0].mulExp;
        result.mulExp->op = BinaryOp::MUL;
        result.mulExp->right = vals[2].unaryExp;
    }
    // mulExp -> mulExp / unaryExp
    else if (prodId == 64) {
        result.mulExp = std::make_shared<MulExpNode>();
        result.mulExp->left = vals[0].mulExp;
        result.mulExp->op = BinaryOp::DIV;
        result.mulExp->right = vals[2].unaryExp;
    }
    // mulExp -> mulExp % unaryExp
    else if (prodId == 65) {
        result.mulExp = std::make_shared<MulExpNode>();
        result.mulExp->left = vals[0].mulExp;
        result.mulExp->op = BinaryOp::MOD;
        result.mulExp->right = vals[2].unaryExp;
    }
    // unaryExp -> primaryExp
    else if (prodId == 66) {
        result.unaryExp = std::make_shared<UnaryExpNode>();
        result.unaryExp->unaryType = UnaryExpNode::UnaryType::PRIMARY;
        result.unaryExp->primaryExp = vals[0].primaryExp;
    }
    // unaryExp -> unaryOp unaryExp
    else if (prodId == 67) {
        result.unaryExp = std::make_shared<UnaryExpNode>();
        result.unaryExp->unaryType = UnaryExpNode::UnaryType::UNARY_OP;
        result.unaryExp->unaryOp = vals[0].unaryOp;
        result.unaryExp->unaryExp = vals[1].unaryExp;
    }
    // unaryExp -> Ident ( )
    else if (prodId == 68) {
        result.unaryExp = std::make_shared<UnaryExpNode>();
        result.unaryExp->unaryType = UnaryExpNode::UnaryType::FUNC_CALL;
        result.unaryExp->funcName = vals[0].terminal;
    }
    // unaryExp -> Ident ( funcRParams )
    else if (prodId == 69) {
        result.unaryExp = std::make_shared<UnaryExpNode>();
        result.unaryExp->unaryType = UnaryExpNode::UnaryType::FUNC_CALL;
        result.unaryExp->funcName = vals[0].terminal;
        result.unaryExp->args = vals[2].funcRParams;
    }
    // primaryExp -> ( exp )
    else if (prodId == 70) {
        result.primaryExp = std::make_shared<PrimaryExpNode>();
        result.primaryExp->primaryType = PrimaryExpNode::PrimaryType::PAREN_EXP;
        result.primaryExp->exp = vals[1].exp;
    }
    // primaryExp -> lVal
    else if (prodId == 71) {
        result.primaryExp = std::make_shared<PrimaryExpNode>();
        result.primaryExp->primaryType = PrimaryExpNode::PrimaryType::LVAL;
        result.primaryExp->lVal = vals[0].lVal;
    }
    // primaryExp -> number
    else if (prodId == 72) {
        result.primaryExp = std::make_shared<PrimaryExpNode>();
        result.primaryExp->primaryType = PrimaryExpNode::PrimaryType::NUMBER;
        result.primaryExp->number = vals[0].number;
    }
    // number -> IntConst
    else if (prodId == 73) {
        result.number = std::make_shared<NumberNode>();
        result.number->isFloat = false;
        result.number->intVal = std::stoi(vals[0].terminal);
    }
    // number -> floatConst
    else if (prodId == 74) {
        result.number = std::make_shared<NumberNode>();
        result.number->isFloat = true;
        result.number->floatVal = std::stof(vals[0].terminal);
    }
    // unaryOp -> +
    else if (prodId == 75) {
        result.unaryOp = UnaryOp::PLUS;
    }
    // unaryOp -> -
    else if (prodId == 76) {
        result.unaryOp = UnaryOp::MINUS;
    }
    // unaryOp -> !
    else if (prodId == 77) {
        result.unaryOp = UnaryOp::NOT;
    }
    // funcRParams -> exp , funcRParams
    else if (prodId == 78) {
        result.funcRParams.push_back(vals[0].exp);
        for (auto& arg : vals[2].funcRParams) {
            result.funcRParams.push_back(arg);
        }
    }
    // funcRParams -> exp
    else if (prodId == 79) {
        result.funcRParams.push_back(vals[0].exp);
    }
    // constExp -> addExp
    else if (prodId == 80) {
        result.exp = vals[0].addExp;
    }
    // cond -> lOrExp
    else if (prodId == 81) {
        result.cond = std::make_shared<CondNode>();
        result.cond->lOrExp = vals[0].lOrExp;
    }
    
    return result;
}
