/*!
 * @file Lexer.h
 * @brief 词法分析器头文件
 * @version 1.0.3
 * @date 2025
 * * 实现基于有限自动机的词法分析器
 * 输出格式: [单词符号] TAB <[类型],[属性]>
 * 修改说明：
 * 1. 确保 IDN, INT, FLOAT 的属性输出为字面值。
 * 2. 确保 KW, OP, SE 的属性输出为种别码。
 * 3. 保持 main 的特殊处理（显示为 KW, 5）。
 */

#ifndef SYSYC_LEXER_H
#define SYSYC_LEXER_H

#include <string>
#include <vector>
#include <map>
#include <set>
#include <fstream>
#include <sstream>
#include <iostream>

/**
 * @brief Token类型枚举
 * 根据实验要求映射整数值 (1-28)
 */
enum class TokenType {
    // 关键字 KW (1-8)
    KW_INT = 1,      // int
    KW_VOID = 2,     // void
    KW_RETURN = 3,   // return
    KW_CONST = 4,    // const
    KW_MAIN = 5,     // main (内部可能为IDN，但输出需映射到此)
    KW_FLOAT = 6,    // float
    KW_IF = 7,       // if
    KW_ELSE = 8,     // else
    
    // 运算符 OP (9-22)
    OP_PLUS = 9,     // +
    OP_MINUS = 10,   // -
    OP_MUL = 11,     // *
    OP_DIV = 12,     // /
    OP_MOD = 13,     // %
    OP_ASSIGN = 14,  // =
    OP_GT = 15,      // >
    OP_LT = 16,      // <
    OP_EQ = 17,      // ==
    OP_LE = 18,      // <=
    OP_GE = 19,      // >=
    OP_NE = 20,      // !=
    OP_AND = 21,     // &&
    OP_OR = 22,      // ||
    
    // 界符 SE (23-28)
    SE_LPAREN = 23,  // (
    SE_RPAREN = 24,  // )
    SE_LBRACE = 25,  // {
    SE_RBRACE = 26,  // }
    SE_SEMI = 27,    // ;
    SE_COMMA = 28,   // ,
    
    // 额外运算符 (不在实验要求输出列表中)
    OP_NOT = 29,     // !
    
    // 标识符和常量
    IDN = 100,       // 标识符
    INT = 101,       // 整数
    FLOAT = 102,     // 浮点数
    
    // 特殊
    END_OF_FILE = 200,
    ERROR = 201
};

/**
 * @brief Token结构
 */
struct Token {
    TokenType type;
    std::string value;
    int line;
    int column;
    
    Token() : type(TokenType::ERROR), value(""), line(0), column(0) {}
    Token(TokenType t, const std::string& v, int l, int c) 
        : type(t), value(v), line(l), column(c) {}
    
    /**
     * @brief 判断Token是否应该输出到结果文件
     */
    bool shouldOutput() const {
        if (type == TokenType::OP_NOT) return false; // ! 不在要求的输出列表中
        if (type == TokenType::END_OF_FILE) return false;
        return true;
    }

    /**
     * @brief 获取类型的字符串表示
     */
    std::string getTypeString() const {
        // 特殊处理 main
        if (type == TokenType::IDN && value == "main") {
            return "KW";
        }

        if (type >= TokenType::KW_INT && type <= TokenType::KW_ELSE) {
            return "KW";
        } else if ((type >= TokenType::OP_PLUS && type <= TokenType::OP_OR) || type == TokenType::OP_NOT) {
            return "OP";
        } else if (type >= TokenType::SE_LPAREN && type <= TokenType::SE_COMMA) {
            return "SE";
        } else if (type == TokenType::IDN) {
            return "IDN";
        } else if (type == TokenType::INT) {
            return "INT";
        } else if (type == TokenType::FLOAT) {
            return "FLOAT";
        } else if (type == TokenType::END_OF_FILE) {
            return "EOF";
        }
        return "ERROR";
    }
    
    /**
     * @brief 获取类型编号
     */
    int getTypeCode() const {
        // 特殊处理 main
        if (type == TokenType::IDN && value == "main") {
            return static_cast<int>(TokenType::KW_MAIN);
        }
        return static_cast<int>(type);
    }
    
    /**
     * @brief 输出格式化的Token
     * 规则: 
     * - IDN, INT, FLOAT: 输出 <类型, 字面值>
     * - KW, OP, SE:      输出 <类型, 编码>
     */
    std::string toString() const {
        std::string typeStr = getTypeString();
        int code = getTypeCode();
        
        // 1. 特殊处理 main: 即使内部是 IDN，输出也要伪装成 KW，属性为编码 (5)
        if (type == TokenType::IDN && value == "main") {
             return value + "\t<KW," + std::to_string(code) + ">";
        }

        // 2. 标识符 (IDN)、整数 (INT)、浮点数 (FLOAT)：属性使用字面值 (value)
        if (type == TokenType::IDN) {
            return value + "\t<" + typeStr + "," + value + ">";
        } else if (type == TokenType::INT || type == TokenType::FLOAT) {
            return value + "\t<" + typeStr + "," + value + ">";
        } 
        
        // 3. 其他（关键字、运算符、界符）：属性使用种别码 (code)
        else {
            return value + "\t<" + typeStr + "," + std::to_string(code) + ">";
        }
    }
};

/**
 * @brief DFA状态枚举
 */
enum class DFAState {
    START,          // 初始状态
    IN_ID,          // 标识符状态
    IN_NUM,         // 整数状态
    IN_FLOAT,       // 浮点数状态  
    IN_ASSIGN,      // =状态
    IN_LT,          // <状态
    IN_GT,          // >状态
    IN_NOT,         // !状态
    IN_AND,         // &状态
    IN_OR,          // |状态
    IN_COMMENT_1,   // /状态
    IN_COMMENT_LINE,// 行注释
    IN_COMMENT_BLOCK,// 块注释
    IN_COMMENT_END, // 块注释可能结束
    DONE,           // 完成状态
    ERROR_STATE     // 错误状态
};

/**
 * @brief 词法分析器类
 */
class Lexer {
private:
    std::string source;           // 源代码
    size_t pos;                   // 当前位置
    int line;                     // 当前行号
    int column;                   // 当前列号
    std::vector<Token> tokens;    // Token序列
    std::map<std::string, TokenType> keywords; // 关键字表
    
public:
    Lexer();
    bool loadFromFile(const std::string& filename);
    void loadFromString(const std::string& code);
    bool tokenize();
    const std::vector<Token>& getTokens() const { return tokens; }
    void printTokens() const;
    std::string getTokensString() const;
    
private:
    char currentChar() const;
    char peekChar() const;
    void advance();
    void skipWhitespace();
    bool skipComment();
    Token scanIdentifier();
    Token scanNumber();
    Token scanOperator();
    Token scanSeparator();
    bool isAlpha(char c) const { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }
    bool isDigit(char c) const { return c >= '0' && c <= '9'; }
    bool isAlnum(char c) const { return isAlpha(c) || isDigit(c) || c == '_'; }
    void initKeywords();
    std::string toLower(const std::string& s) const;
};

#endif // SYSYC_LEXER_H