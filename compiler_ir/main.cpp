#include "IRGenerator.h"
#include <iostream>

int main() {
    // 构建 AST（示例：int a = 10; int main() { a = a + 5; return a; }）
    
    // 创建编译单元
    auto compUnit = std::make_unique<CompUnitNode>();
    
    // 全局变量: int a = 10;
    auto globalVarDecl = std::make_unique<VarDeclNode>(new TypeNode(TypeNode::INT));
    auto varDef = std::make_unique<VarDefNode>("a", new NumberNode(10));
    globalVarDecl->varDefs.push_back(std::move(varDef));
    compUnit->items.push_back(std::move(globalVarDecl));
    
    // 函数: int main() { ... }
    auto mainFunc = std::make_unique<FuncDefNode>(
        new TypeNode(TypeNode::INT), "main");
    
    // 函数体
    auto body = std::make_unique<BlockNode>();
    
    // a = a + 5;
    auto assignStmt = std::make_unique<AssignStmtNode>(
        "a",
        new BinaryExprNode(
            BinaryExprNode::ADD,
            new VarRefNode("a"),
            new NumberNode(5)
        )
    );
    body->items.push_back(std::move(assignStmt));
    
    // return a;
    auto returnStmt = std::make_unique<ReturnStmtNode>(new VarRefNode("a"));
    body->items.push_back(std::move(returnStmt));
    
    mainFunc->body = std::move(body);
    compUnit->items.push_back(std::move(mainFunc));
    
    // 生成 IR
    IRGenerator generator("test_module");
    generator.generate(compUnit.get());
    
    // 打印 IR
    std::cout << generator.print() << std::endl;
    
    return 0;
}