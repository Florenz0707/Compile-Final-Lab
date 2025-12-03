/*!
 * @file Lexer.h
 * @brief 词法分析器头文件
 * @version 1.0.0
 * @date 2024
 * 
 * 实现基于有限自动机的词法分析器
 * 输出格式: [单词符号] TAB <[类型],[属性]>
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
 */
enum class TokenType {
    // 关键字 KW (1-8)
    KW_INT = 1,      // int
    KW_VOID = 2,     // void
    KW_RETURN = 3,   // return
    KW_CONST = 4,    // const
    KW_MAIN = 5,     // main
    KW_FLOAT = 6,    // float
    KW_IF = 7,       // if
    KW_ELSE = 8,     // else
    
    // 运算符 OP (30-43)
    OP_PLUS = 30,    // +
    OP_MINUS = 31,   // -
    OP_MUL = 32,     // *
    OP_DIV = 33,     // /
    OP_MOD = 34,     // %
    OP_ASSIGN = 35,  // =
    OP_GT = 36,      // >
    OP_LT = 37,      // <
    OP_EQ = 38,      // ==
    OP_LE = 39,      // <=
    OP_GE = 40,      // >=
    OP_NE = 41,      // !=
    OP_AND = 42,     // &&
    OP_OR = 43,      // ||
    OP_NOT = 44,     // ! (单独的逻辑非)
    
    // 界符 SE (50-55)
    SE_LPAREN = 50,  // (
    SE_RPAREN = 51,  // )
    SE_LBRACE = 52,  // {
    SE_RBRACE = 53,  // }
    SE_SEMI = 54,    // ;
    SE_COMMA = 55,   // ,
    
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
     * @brief 获取类型的字符串表示
     */
    std::string getTypeString() const {
        if (type >= TokenType::KW_INT && type <= TokenType::KW_ELSE) {
            return "KW";
        } else if (type >= TokenType::OP_PLUS && type <= TokenType::OP_OR) {
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
        return static_cast<int>(type);
    }
    
    /**
     * @brief 输出格式化的Token
     */
    std::string toString() const {
        std::string typeStr = getTypeString();
        int code = getTypeCode();
        if (type == TokenType::IDN) {
            return value + "\t<" + typeStr + "," + value + ">";
        } else if (type == TokenType::INT || type == TokenType::FLOAT) {
            return value + "\t<" + typeStr + "," + value + ">";
        } else {
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
    IN_ASSIGN,      // =状态（可能是=或==）
    IN_LT,          // <状态（可能是<或<=）
    IN_GT,          // >状态（可能是>或>=）
    IN_NOT,         // !状态（需要!=）
    IN_AND,         // &状态（需要&&）
    IN_OR,          // |状态（需要||）
    IN_COMMENT_1,   // /状态（可能是/或注释）
    IN_COMMENT_LINE,// 行注释
    IN_COMMENT_BLOCK,// 块注释
    IN_COMMENT_END, // 块注释可能结束(*)
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
    /**
     * @brief 构造函数
     */
    Lexer();
    
    /**
     * @brief 从文件加载源代码
     */
    bool loadFromFile(const std::string& filename);
    
    /**
     * @brief 从字符串加载源代码
     */
    void loadFromString(const std::string& code);
    
    /**
     * @brief 执行词法分析
     * @return bool 是否成功
     */
    bool tokenize();
    
    /**
     * @brief 获取所有Token
     */
    const std::vector<Token>& getTokens() const { return tokens; }
    
    /**
     * @brief 打印Token序列
     */
    void printTokens() const;
    
    /**
     * @brief 获取Token序列的字符串形式
     */
    std::string getTokensString() const;
    
private:
    /**
     * @brief 获取当前字符
     */
    char currentChar() const;
    
    /**
     * @brief 查看下一个字符
     */
    char peekChar() const;
    
    /**
     * @brief 前进到下一个字符
     */
    void advance();
    
    /**
     * @brief 跳过空白字符
     */
    void skipWhitespace();
    
    /**
     * @brief 跳过注释
     */
    bool skipComment();
    
    /**
     * @brief 扫描标识符或关键字
     */
    Token scanIdentifier();
    
    /**
     * @brief 扫描数字（整数或浮点数）
     */
    Token scanNumber();
    
    /**
     * @brief 扫描运算符
     */
    Token scanOperator();
    
    /**
     * @brief 扫描界符
     */
    Token scanSeparator();
    
    /**
     * @brief 判断是否是字母
     */
    bool isAlpha(char c) const { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }
    
    /**
     * @brief 判断是否是数字
     */
    bool isDigit(char c) const { return c >= '0' && c <= '9'; }
    
    /**
     * @brief 判断是否是字母或数字或下划线
     */
    bool isAlnum(char c) const { return isAlpha(c) || isDigit(c) || c == '_'; }
    
    /**
     * @brief 初始化关键字表
     */
    void initKeywords();
    
    /**
     * @brief 字符串转小写
     */
    std::string toLower(const std::string& s) const;
};

#endif // SYSYC_LEXER_H
