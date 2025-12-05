/*!
 * @file main.cpp
 * @brief C--编译器主程序
 * @version 1.0.3
 * @date 2024
 */

#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <vector>
#include <exception>
#include <dirent.h>
#include "SLRLexer.h"
#include "SLRParser.h"
#include "AST.h"
#include "IRGenerator.h"

/**
 * @brief 打印使用说明
 */
void printUsage(const char* programName) {
    std::cout << "用法: " << programName << " [选项] <源文件>" << std::endl;
    std::cout << "选项:" << std::endl;
    std::cout << "  -l, --lexer    仅执行词法分析" << std::endl;
    std::cout << "  -p, --parser   执行词法和语法分析" << std::endl;
    std::cout << "  -i, --ir       执行完整编译（生成LLVM IR）" << std::endl;
    std::cout << "  -t, --test     运行内置测试" << std::endl;
    std::cout << "  -a, --all      运行所有测试用例并输出结果到文件" << std::endl;
    std::cout << "  -h, --help     显示此帮助信息" << std::endl;
}

/**
 * @brief 详细分析文件（语法分析 + 词法分析）
 * @return 0 for accept, 1 for error
 */
int analyzeFileVerbose(const std::string& filename) {
    std::cout << "========================================" << std::endl;
    std::cout << "分析文件: " << filename << std::endl;
    std::cout << "========================================" << std::endl;

    // 读取源文件
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "错误: 无法打开文件" << std::endl;
        return 1;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string sourceCode = buffer.str();
    file.close();

    // 词法分析
    SLRLexer lexer;
    auto tokens = lexer.analyze(sourceCode);

    std::cout << "\n========== 词法分析结果 ==========" << std::endl;
    for (const auto& token : tokens) {
        if (token.type != TokenType::END_OF_FILE) {
            std::cout << token.toString() << std::endl;
        }
    }

    // 检查词法错误
    bool hasLexError = false;
    for (const auto& token : tokens) {
        if (token.type == TokenType::ERROR) {
            hasLexError = true;
            std::cout << "词法错误: 非法字符 '" << token.value
                      << "' at line " << token.line << std::endl;
        }
    }

    std::cout << "\n词法分析结果: " << (hasLexError ? "有错误" : "通过") << std::endl;

    // 语法分析
    std::cout << "\n========== 语法分析结果 ==========" << std::endl;
    SLRParser parser;
    bool parseSuccess = parser.parse(tokens);

    std::cout << "\n最终结果: " << (parseSuccess ? "accept" : "error") << std::endl;

    return (hasLexError || !parseSuccess) ? 1 : 0;
}

/**
 * @brief 运行所有测试用例并生成输出文件
 */
void runAllTestcases(const std::string& testDir) {
    std::cout << "============================================" << std::endl;
    std::cout << "        批量运行测试并生成结果文件" << std::endl;
    std::cout << "============================================" << std::endl;
    std::cout << "输出目录: " << testDir << std::endl;
    std::cout << "说明: \n  *.tok - 词法分析结果\n  *.ll  - LLVM IR中间代码\n" << std::endl;

    // 合并所有测试用例，统一处理
    std::vector<std::string> allCases = {
        "accept1.sy", "accept2.sy", "accept3.sy",
        "accept4.sy", "accept5.sy", "accept6.sy",
        "refuse1.sy", "refuse2.sy", "refuse3.sy", "refuse4.sy"
    };

    int successCount = 0;

    for (const auto& testCase : allCases) {
        std::string filepath = testDir + "/" + testCase;
        // 获取不带后缀的文件名 (例如 accept1)
        std::string stem = filepath.substr(0, filepath.find_last_of('.'));

        std::cout << "Processing [" << testCase << "]..." << std::flush;

        // 1. 读取源文件
        std::ifstream file(filepath);
        if (!file.is_open()) {
            std::cout << " FILE NOT FOUND" << std::endl;
            continue;
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string sourceCode = buffer.str();
        file.close();

        // 2. 词法分析 & 输出到 .tok 文件
        SLRLexer lexer;
        auto tokens = lexer.analyze(sourceCode);

        {
            std::ofstream tokFile(stem + ".tok");
            for (const auto& token : tokens) {
                if (token.type != TokenType::END_OF_FILE) {
                    tokFile << token.toString() << std::endl;
                }
            }
        }

        bool hasLexError = false;
        for (const auto& token : tokens) {
            if (token.type == TokenType::ERROR) hasLexError = true;
        }

        // 3. 语法分析
        SLRParser parser;
        bool parseSuccess = parser.parse(tokens);
        std::string caseName = testCase;
        size_t dotPos = caseName.find_last_of('.');
        if (dotPos != std::string::npos) caseName = caseName.substr(0, dotPos);
        std::string relPath = stem + ".spe";
        try {
            parser.saveParseLog(relPath);
        } catch (const std::exception& e) {
            std::cerr << "无法写入语法输出文件 " << relPath << ": " << e.what() << std::endl;
        }

        // 3. 中间代码生成 & 输出到 .ll 文件 (仅当语法正确时)
        bool irGenerated = false;
        if (parseSuccess && !hasLexError) {
            auto ast = parser.getAST();
            if (ast) {
                IRGenerator generator(filepath);
                generator.generate(ast);

                std::ofstream llFile(stem + ".ll");
                llFile << generator.print();
                llFile.close();
                irGenerated = true;
            }
        }

        // 打印摘要
        if (hasLexError) {
            std::cout << " LEX ERROR -> " << testCase << ".tok" << std::endl;
        } else if (!parseSuccess) {
            std::cout << " PARSE ERROR -> " << testCase << ".tok" << std::endl;
        } else {
            std::cout << " OK -> " << testCase << ".tok, " << testCase << ".ll" << std::endl;
            successCount++;
        }
    }

    std::cout << "\n======================================" << std::endl;
    std::cout << "完成! 成功生成IR: " << successCount << "/" << allCases.size() << std::endl;
    std::cout << "请检查 " << testDir << " 目录下的生成文件。" << std::endl;
    std::cout << "======================================" << std::endl;
}

/**
 * @brief 显示详细的词法分析结果
 * @return 0 for success, 1 for error
 */
int showDetailedLexer(const std::string& filename) {
    std::cout << "========================================" << std::endl;
    std::cout << "词法分析: " << filename << std::endl;
    std::cout << "========================================" << std::endl;

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "错误: 无法打开文件" << std::endl;
        return 1;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string sourceCode = buffer.str();
    file.close();

    SLRLexer lexer;
    auto tokens = lexer.analyze(sourceCode);

    std::cout << "\n单词符号序列:" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    for (const auto& token : tokens) {
        if (token.type != TokenType::END_OF_FILE) {
            std::cout << token.toString() << std::endl;
        }
    }
    std::cout << "----------------------------------------" << std::endl;

    int errorCount = 0;
    for (const auto& token : tokens) {
        if (token.type == TokenType::ERROR) {
            std::cout << "错误: 非法字符 '" << token.value
                      << "' at line " << token.line << ", column " << token.column << std::endl;
            errorCount++;
        }
    }

    std::cout << "\nToken总数: " << tokens.size() - 1 << " (不含EOF)" << std::endl;
    std::cout << "错误数量: " << errorCount << std::endl;
    std::cout << "结果: " << (errorCount == 0 ? "PASS" : "FAIL") << std::endl;

    return (errorCount == 0) ? 0 : 1;
}

/**
 * @brief 手动构建AST进行IR生成测试 (仅用于 -t 测试)
 */
std::shared_ptr<CompUnitNode> buildSimpleAST() {
    auto compUnit = std::make_shared<CompUnitNode>();

    // int a = 10;
    auto varDecl = std::make_shared<VarDeclNode>();
    varDecl->bType = BType::INT;
    auto varDef = std::make_shared<VarDefNode>();
    varDef->ident = "a";
    auto num10 = std::make_shared<NumberNode>();
    num10->intVal = 10;
    auto primary10 = std::make_shared<PrimaryExpNode>();
    primary10->primaryType = PrimaryExpNode::PrimaryType::NUMBER;
    primary10->number = num10;
    auto unary10 = std::make_shared<UnaryExpNode>();
    unary10->unaryType = UnaryExpNode::UnaryType::PRIMARY;
    unary10->primaryExp = primary10;
    auto mul10 = std::make_shared<MulExpNode>();
    mul10->left = nullptr;
    mul10->right = unary10;
    auto add10 = std::make_shared<AddExpNode>();
    add10->left = nullptr;
    add10->right = mul10;
    varDef->initVal = add10;
    varDecl->varDefs.push_back(varDef);
    compUnit->decls.push_back(varDecl);

    // int main() { a = 10; return 0; }
    auto mainFunc = std::make_shared<FuncDefNode>();
    mainFunc->returnType = BType::INT;
    mainFunc->ident = "main";
    auto block = std::make_shared<BlockNode>();

    auto assignStmt = std::make_shared<StmtNode>();
    assignStmt->stmtType = StmtType::ASSIGN;
    auto lVal = std::make_shared<LValNode>();
    lVal->ident = "a";
    assignStmt->lVal = lVal;
    auto num10_2 = std::make_shared<NumberNode>();
    num10_2->intVal = 10;
    auto primary10_2 = std::make_shared<PrimaryExpNode>();
    primary10_2->primaryType = PrimaryExpNode::PrimaryType::NUMBER;
    primary10_2->number = num10_2;
    auto unary10_2 = std::make_shared<UnaryExpNode>();
    unary10_2->unaryType = UnaryExpNode::UnaryType::PRIMARY;
    unary10_2->primaryExp = primary10_2;
    auto mul10_2 = std::make_shared<MulExpNode>();
    mul10_2->left = nullptr;
    mul10_2->right = unary10_2;
    auto add10_2 = std::make_shared<AddExpNode>();
    add10_2->left = nullptr;
    add10_2->right = mul10_2;
    assignStmt->exp = add10_2;
    auto item1 = std::make_shared<BlockItemNode>();
    item1->stmt = assignStmt;
    block->items.push_back(item1);

    auto retStmt = std::make_shared<StmtNode>();
    retStmt->stmtType = StmtType::RETURN;
    auto num0 = std::make_shared<NumberNode>();
    num0->intVal = 0;
    auto primary0 = std::make_shared<PrimaryExpNode>();
    primary0->primaryType = PrimaryExpNode::PrimaryType::NUMBER;
    primary0->number = num0;
    auto unary0 = std::make_shared<UnaryExpNode>();
    unary0->unaryType = UnaryExpNode::UnaryType::PRIMARY;
    unary0->primaryExp = primary0;
    auto mul0 = std::make_shared<MulExpNode>();
    mul0->left = nullptr;
    mul0->right = unary0;
    auto add0 = std::make_shared<AddExpNode>();
    add0->left = nullptr;
    add0->right = mul0;
    retStmt->exp = add0;
    auto item2 = std::make_shared<BlockItemNode>();
    item2->stmt = retStmt;
    block->items.push_back(item2);

    mainFunc->block = block;
    compUnit->funcDefs.push_back(mainFunc);

    return compUnit;
}

/**
 * @brief 内置测试
 */
void runBuiltinTests() {
    std::cout << "============================================" << std::endl;
    std::cout << "            内置测试" << std::endl;
    std::cout << "============================================" << std::endl;

    // 测试IR生成
    std::cout << "\n========== 中间代码生成测试 ==========" << std::endl;
    std::string code = R"(
int a = 10;
int main() {
    a = 10;
    return 0;
}
)";
    std::cout << "源代码:" << code << std::endl;

    auto ast = buildSimpleAST();
    IRGenerator generator("test.sy");
    generator.generate(ast);

    std::cout << "生成的LLVM IR:" << std::endl;
    std::cout << generator.print() << std::endl;
}

/**
 * @brief 主函数
 */
int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    std::string arg1 = argv[1];

    if (arg1 == "-h" || arg1 == "--help") {
        printUsage(argv[0]);
        return 0;
    }

    if (arg1 == "-t" || arg1 == "--test") {
        runBuiltinTests();
        return 0;
    }

    if (arg1 == "-a" || arg1 == "--all") {
        std::string testDir = "../testcase";
        if (argc >= 3) {
            testDir = argv[2];
        }
        runAllTestcases(testDir);
        return 0;
    }

    if (arg1 == "-l" || arg1 == "--lexer") {
        if (argc < 3) {
            std::cerr << "错误: 请指定源文件" << std::endl;
            return 1;
        }
        // 现在会返回 0 或 1
        return showDetailedLexer(argv[2]);
    }

    if (arg1 == "-p" || arg1 == "--parser") {
        if (argc < 3) {
            std::cerr << "错误: 请指定源文件" << std::endl;
            return 1;
        }
        return analyzeFileVerbose(argv[2]);
    }

    if (arg1 == "-i" || arg1 == "--ir") {
        if (argc < 3) {
            std::cerr << "错误: 请指定源文件" << std::endl;
            return 1;
        }

        std::string filename = argv[2];
        std::cout << "========================================" << std::endl;
        std::cout << "分析文件并生成IR: " << filename << std::endl;
        std::cout << "========================================" << std::endl;

        // 1. 读取源文件
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "错误: 无法打开文件" << std::endl;
            return 1;
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string sourceCode = buffer.str();
        file.close();

        // 2. 词法分析
        SLRLexer lexer;
        auto tokens = lexer.analyze(sourceCode);

        // 3. 语法分析
        SLRParser parser;
        bool parseSuccess = parser.parse(tokens);

        if (!parseSuccess) {
            std::cerr << "错误: 语法分析失败，无法生成中间代码" << std::endl;
            return 1;
        }

        // 4. 中间代码生成
        std::cout << "\n========== 中间代码生成 ==========" << std::endl;
        auto ast = parser.getAST();

        if (!ast) {
             std::cerr << "错误: AST为空" << std::endl;
             return 1;
        }

        IRGenerator generator(filename);
        generator.generate(ast);
        std::cout << generator.print() << std::endl;
        return 0;
    }

    // 默认只做词法分析
    return showDetailedLexer(arg1);
}
