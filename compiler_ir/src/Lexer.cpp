/*!
 * @file Lexer.cpp
 * @brief 词法分析器实现文件
 * @version 1.0.0
 * @date 2025
 */

#include "Lexer.h"
#include <algorithm>
#include <cctype>

// ==================== 构造函数 ====================

Lexer::Lexer() : pos(0), line(1), column(1) {
    initKeywords();
}

void Lexer::initKeywords() {
    // 关键字不区分大小写
    // 注意: main不作为关键字，而是普通标识符
    keywords["int"] = TokenType::KW_INT;
    keywords["void"] = TokenType::KW_VOID;
    keywords["return"] = TokenType::KW_RETURN;
    keywords["const"] = TokenType::KW_CONST;
    keywords["float"] = TokenType::KW_FLOAT;
    keywords["if"] = TokenType::KW_IF;
    keywords["else"] = TokenType::KW_ELSE;
}

std::string Lexer::toLower(const std::string& s) const {
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

// ==================== 加载源代码 ====================

bool Lexer::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: 无法打开文件 " << filename << std::endl;
        return false;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    source = buffer.str();
    file.close();
    
    pos = 0;
    line = 1;
    column = 1;
    tokens.clear();
    
    return true;
}

void Lexer::loadFromString(const std::string& code) {
    source = code;
    pos = 0;
    line = 1;
    column = 1;
    tokens.clear();
}

// ==================== 字符操作 ====================

char Lexer::currentChar() const {
    if (pos >= source.length()) {
        return '\0';
    }
    return source[pos];
}

char Lexer::peekChar() const {
    if (pos + 1 >= source.length()) {
        return '\0';
    }
    return source[pos + 1];
}

void Lexer::advance() {
    if (pos < source.length()) {
        if (source[pos] == '\n') {
            line++;
            column = 1;
        } else {
            column++;
        }
        pos++;
    }
}

void Lexer::skipWhitespace() {
    while (pos < source.length() && 
           (source[pos] == ' ' || source[pos] == '\t' || 
            source[pos] == '\n' || source[pos] == '\r')) {
        advance();
    }
}

bool Lexer::skipComment() {
    if (currentChar() == '/' && peekChar() == '/') {
        // 行注释
        while (currentChar() != '\0' && currentChar() != '\n') {
            advance();
        }
        return true;
    } else if (currentChar() == '/' && peekChar() == '*') {
        // 块注释
        advance(); // skip /
        advance(); // skip *
        while (currentChar() != '\0') {
            if (currentChar() == '*' && peekChar() == '/') {
                advance(); // skip *
                advance(); // skip /
                return true;
            }
            advance();
        }
        std::cerr << "Error: 未闭合的块注释" << std::endl;
        return true;
    }
    return false;
}

// ==================== 扫描函数 ====================

Token Lexer::scanIdentifier() {
    int startLine = line;
    int startCol = column;
    std::string value;
    
    while (isAlnum(currentChar())) {
        value += currentChar();
        advance();
    }
    
    // 检查是否是关键字（不区分大小写）
    std::string lowerValue = toLower(value);
    auto it = keywords.find(lowerValue);
    if (it != keywords.end()) {
        return Token(it->second, value, startLine, startCol);
    }
    
    return Token(TokenType::IDN, value, startLine, startCol);
}

Token Lexer::scanNumber() {
    int startLine = line;
    int startCol = column;
    std::string value;
    bool isFloat = false;
    
    // 整数部分
    while (isDigit(currentChar())) {
        value += currentChar();
        advance();
    }
    
    // 检查小数点
    if (currentChar() == '.' && isDigit(peekChar())) {
        isFloat = true;
        value += currentChar();
        advance();
        
        // 小数部分
        while (isDigit(currentChar())) {
            value += currentChar();
            advance();
        }
    }
    
    if (isFloat) {
        return Token(TokenType::FLOAT, value, startLine, startCol);
    } else {
        return Token(TokenType::INT, value, startLine, startCol);
    }
}

Token Lexer::scanOperator() {
    int startLine = line;
    int startCol = column;
    char c = currentChar();
    
    switch (c) {
        case '+':
            advance();
            return Token(TokenType::OP_PLUS, "+", startLine, startCol);
        case '-':
            advance();
            return Token(TokenType::OP_MINUS, "-", startLine, startCol);
        case '*':
            advance();
            return Token(TokenType::OP_MUL, "*", startLine, startCol);
        case '/':
            advance();
            return Token(TokenType::OP_DIV, "/", startLine, startCol);
        case '%':
            advance();
            return Token(TokenType::OP_MOD, "%", startLine, startCol);
        case '=':
            advance();
            if (currentChar() == '=') {
                advance();
                return Token(TokenType::OP_EQ, "==", startLine, startCol);
            }
            return Token(TokenType::OP_ASSIGN, "=", startLine, startCol);
        case '<':
            advance();
            if (currentChar() == '=') {
                advance();
                return Token(TokenType::OP_LE, "<=", startLine, startCol);
            }
            return Token(TokenType::OP_LT, "<", startLine, startCol);
        case '>':
            advance();
            if (currentChar() == '=') {
                advance();
                return Token(TokenType::OP_GE, ">=", startLine, startCol);
            }
            return Token(TokenType::OP_GT, ">", startLine, startCol);
        case '!':
            advance();
            if (currentChar() == '=') {
                advance();
                return Token(TokenType::OP_NE, "!=", startLine, startCol);
            }
            // 单独的!是逻辑非运算符
            return Token(TokenType::OP_NOT, "!", startLine, startCol);
        case '&':
            advance();
            if (currentChar() == '&') {
                advance();
                return Token(TokenType::OP_AND, "&&", startLine, startCol);
            }
            return Token(TokenType::ERROR, "&", startLine, startCol);
        case '|':
            advance();
            if (currentChar() == '|') {
                advance();
                return Token(TokenType::OP_OR, "||", startLine, startCol);
            }
            return Token(TokenType::ERROR, "|", startLine, startCol);
        default:
            return Token(TokenType::ERROR, std::string(1, c), startLine, startCol);
    }
}

Token Lexer::scanSeparator() {
    int startLine = line;
    int startCol = column;
    char c = currentChar();
    
    switch (c) {
        case '(':
            advance();
            return Token(TokenType::SE_LPAREN, "(", startLine, startCol);
        case ')':
            advance();
            return Token(TokenType::SE_RPAREN, ")", startLine, startCol);
        case '{':
            advance();
            return Token(TokenType::SE_LBRACE, "{", startLine, startCol);
        case '}':
            advance();
            return Token(TokenType::SE_RBRACE, "}", startLine, startCol);
        case ';':
            advance();
            return Token(TokenType::SE_SEMI, ";", startLine, startCol);
        case ',':
            advance();
            return Token(TokenType::SE_COMMA, ",", startLine, startCol);
        default:
            return Token(TokenType::ERROR, std::string(1, c), startLine, startCol);
    }
}

// ==================== 主分析函数 ====================

bool Lexer::tokenize() {
    tokens.clear();
    
    while (pos < source.length()) {
        // 跳过空白
        skipWhitespace();
        
        if (pos >= source.length()) {
            break;
        }
        
        // 检查注释
        if (currentChar() == '/' && (peekChar() == '/' || peekChar() == '*')) {
            skipComment();
            continue;
        }
        
        char c = currentChar();
        
        // DFA状态转换
        if (isAlpha(c) || c == '_') {
            // 标识符或关键字
            tokens.push_back(scanIdentifier());
        } else if (isDigit(c)) {
            // 数字
            tokens.push_back(scanNumber());
        } else if (c == '(' || c == ')' || c == '{' || c == '}' || c == ';' || c == ',') {
            // 界符
            tokens.push_back(scanSeparator());
        } else if (c == '+' || c == '-' || c == '*' || c == '/' || c == '%' ||
                   c == '=' || c == '<' || c == '>' || c == '!' || c == '&' || c == '|') {
            // 运算符
            tokens.push_back(scanOperator());
        } else if (c == '\0') {
            break;
        } else {
            // 未知字符 - 创建ERROR token
            int startLine = line;
            int startCol = column;
            std::string ch(1, c);
            tokens.push_back(Token(TokenType::ERROR, ch, startLine, startCol));
            advance();
        }
    }
    
    // 添加EOF标记
    tokens.push_back(Token(TokenType::END_OF_FILE, "$", line, column));
    
    return true;
}

void Lexer::printTokens() const {
    std::cout << getTokensString();
}

std::string Lexer::getTokensString() const {
    std::stringstream ss;
    for (const auto& token : tokens) {
        if (token.type != TokenType::END_OF_FILE) {
            ss << token.toString() << std::endl;
        }
    }
    return ss.str();
}
