#include "IRGenerator.h"
#include "Instruction.h"
#include "GlobalVariable.h"
#include "Constant.h"
#include <iostream>

// ============ AST 节点的 accept 实现 ============
void TypeNode::accept(ASTVisitor* visitor) { visitor->visit(this); }
void NumberNode::accept(ASTVisitor* visitor) { visitor->visit(this); }
void VarRefNode::accept(ASTVisitor* visitor) { visitor->visit(this); }
void BinaryExprNode::accept(ASTVisitor* visitor) { visitor->visit(this); }
void UnaryExprNode::accept(ASTVisitor* visitor) { visitor->visit(this); }
void CallExprNode::accept(ASTVisitor* visitor) { visitor->visit(this); }
void AssignStmtNode::accept(ASTVisitor* visitor) { visitor->visit(this); }
void ExprStmtNode::accept(ASTVisitor* visitor) { visitor->visit(this); }
void BlockNode::accept(ASTVisitor* visitor) { visitor->visit(this); }
void IfStmtNode::accept(ASTVisitor* visitor) { visitor->visit(this); }
void ReturnStmtNode::accept(ASTVisitor* visitor) { visitor->visit(this); }
void VarDefNode::accept(ASTVisitor* visitor) { visitor->visit(this); }
void VarDeclNode::accept(ASTVisitor* visitor) { visitor->visit(this); }
void FuncParamNode::accept(ASTVisitor* visitor) { visitor->visit(this); }
void FuncDefNode::accept(ASTVisitor* visitor) { visitor->visit(this); }
void CompUnitNode::accept(ASTVisitor* visitor) { visitor->visit(this); }

// ============ IRGenerator 实现 ============

IRGenerator::IRGenerator(const std::string& moduleName) 
    : currentFunction_(nullptr), currentValue_(nullptr), 
      currentType_(nullptr), isGlobalScope_(true) {
    // 创建模块
    module_ = new Module(moduleName);
    
    // 创建 IRBuilder
    // 注意：需要一个初始的 BasicBlock，这里先设为 nullptr
    builder_ = new IRBuilder(nullptr, module_);
    
    // 创建运行时库函数声明
    createRuntimeFunctions();
}

IRGenerator::~IRGenerator() {
    delete builder_;
    // module_ 由外部管理生命周期
}

void IRGenerator::createRuntimeFunctions() {
    // 创建 C-- 运行时库函数声明
    // declare i32 @getint()
    auto getintType = FunctionType::get(Type::get_int32_type(module_), {});
    Function::create(getintType, "getint", module_);
    
    // declare i32 @getch()
    auto getchType = FunctionType::get(Type::get_int32_type(module_), {});
    Function::create(getchType, "getch", module_);
    
    // declare void @putint(i32)
    auto putintType = FunctionType::get(Type::get_void_type(module_), 
                                        {Type::get_int32_type(module_)});
    Function::create(putintType, "putint", module_);
    
    // declare void @putch(i32)
    auto putchType = FunctionType::get(Type::get_void_type(module_), 
                                       {Type::get_int32_type(module_)});
    Function::create(putchType, "putch", module_);
}

Type* IRGenerator::getTypeFromASTType(TypeNode::Type type) {
    switch (type) {
        case TypeNode::INT:
            return Type::get_int32_type(module_);
        case TypeNode::FLOAT:
            return Type::get_float_type(module_);
        case TypeNode::VOID:
            return Type::get_void_type(module_);
        default:
            return nullptr;
    }
}

void IRGenerator::generate(CompUnitNode* root) {
    root->accept(this);
}

std::string IRGenerator::print() {
    return module_->print();
}

void IRGenerator::enterScope() {
    // 简化版本：不实现嵌套作用域
    // 完整版本应该使用符号表栈
}

void IRGenerator::leaveScope() {
    // 简化版本
}

// ============ Visitor 实现 ============

void IRGenerator::visit(TypeNode* node) {
    currentType_ = getTypeFromASTType(node->type);
}

void IRGenerator::visit(NumberNode* node) {
    if (node->numType == NumberNode::INT_NUM) {
        // 创建整数常量
        currentValue_ = ConstantInt::get(node->intVal, module_);
    } else {
        // 创建浮点常量
        currentValue_ = ConstantFP::get(node->floatVal, module_);
    }
}

void IRGenerator::visit(VarRefNode* node) {
    // 从符号表中查找变量
    auto it = namedValues_.find(node->name);
    if (it == namedValues_.end()) {
        std::cerr << "Error: Unknown variable name: " << node->name << std::endl;
        currentValue_ = nullptr;
        return;
    }
    
    Value* varPtr = it->second;
    
    // 加载变量的值
    currentValue_ = builder_->create_load(varPtr);
}

void IRGenerator::visit(BinaryExprNode* node) {
    // 计算左操作数
    node->left->accept(this);
    Value* left = currentValue_;
    
    // 计算右操作数
    node->right->accept(this);
    Value* right = currentValue_;
    
    if (!left || !right) {
        currentValue_ = nullptr;
        return;
    }
    
    // 根据运算符类型生成相应的指令
    switch (node->op) {
        case BinaryExprNode::ADD:
            currentValue_ = builder_->create_iadd(left, right);
            break;
        case BinaryExprNode::SUB:
            currentValue_ = builder_->create_isub(left, right);
            break;
        case BinaryExprNode::MUL:
            currentValue_ = builder_->create_imul(left, right);
            break;
        case BinaryExprNode::DIV:
            currentValue_ = builder_->create_isdiv(left, right);
            break;
        case BinaryExprNode::MOD:
            currentValue_ = builder_->create_irem(left, right);
            break;
        case BinaryExprNode::LT:
            currentValue_ = builder_->create_icmp_lt(left, right);
            break;
        case BinaryExprNode::LE:
            currentValue_ = builder_->create_icmp_le(left, right);
            break;
        case BinaryExprNode::GT:
            currentValue_ = builder_->create_icmp_gt(left, right);
            break;
        case BinaryExprNode::GE:
            currentValue_ = builder_->create_icmp_ge(left, right);
            break;
        case BinaryExprNode::EQ:
            currentValue_ = builder_->create_icmp_eq(left, right);
            break;
        case BinaryExprNode::NE:
            currentValue_ = builder_->create_icmp_ne(left, right);
            break;
        case BinaryExprNode::AND:
        case BinaryExprNode::OR:
            // 逻辑运算需要短路求值，这里简化处理
            // 完整实现需要创建额外的基本块
            std::cerr << "Warning: Logical operators not fully implemented" << std::endl;
            currentValue_ = nullptr;
            break;
        default:
            currentValue_ = nullptr;
            break;
    }
}

void IRGenerator::visit(UnaryExprNode* node) {
    node->operand->accept(this);
    Value* operand = currentValue_;
    
    if (!operand) {
        currentValue_ = nullptr;
        return;
    }
    
    switch (node->op) {
        case UnaryExprNode::PLUS:
            // +a 就是 a 本身
            currentValue_ = operand;
            break;
        case UnaryExprNode::MINUS:
            // -a 等价于 0 - a
            currentValue_ = builder_->create_isub(
                ConstantInt::get(0, module_), operand);
            break;
        case UnaryExprNode::NOT:
            // !a 等价于 a == 0
            currentValue_ = builder_->create_icmp_eq(
                operand, ConstantInt::get(0, module_));
            break;
        default:
            currentValue_ = nullptr;
            break;
    }
}

void IRGenerator::visit(CallExprNode* node) {
    // 查找函数
    Function* callee = nullptr;
    for (auto func : module_->get_functions()) {
        if (func->get_name() == node->funcName) {
            callee = func;
            break;
        }
    }
    
    if (!callee) {
        std::cerr << "Error: Unknown function: " << node->funcName << std::endl;
        currentValue_ = nullptr;
        return;
    }
    
    // 计算参数
    std::vector<Value*> args;
    for (auto& arg : node->args) {
        arg->accept(this);
        if (currentValue_) {
            args.push_back(currentValue_);
        }
    }
    
    // 创建函数调用指令
    currentValue_ = builder_->create_call(callee, args);
}

void IRGenerator::visit(AssignStmtNode* node) {
    // 计算右值
    node->expr->accept(this);
    Value* value = currentValue_;
    
    if (!value) {
        return;
    }
    
    // 查找变量
    auto it = namedValues_.find(node->varName);
    if (it == namedValues_.end()) {
        std::cerr << "Error: Unknown variable: " << node->varName << std::endl;
        return;
    }
    
    // 存储值到变量
    builder_->create_store(value, it->second);
}

void IRGenerator::visit(ExprStmtNode* node) {
    if (node->expr) {
        node->expr->accept(this);
    }
}

void IRGenerator::visit(BlockNode* node) {
    // 遍历块中的所有项
    for (auto& item : node->items) {
        item->accept(this);
    }
}

void IRGenerator::visit(IfStmtNode* node) {
    // 计算条件
    node->condition->accept(this);
    Value* condValue = currentValue_;
    
    if (!condValue) {
        return;
    }
    
    // 创建基本块
    BasicBlock* thenBB = BasicBlock::create(module_, "if_then", currentFunction_);
    BasicBlock* elseBB = node->elseStmt ? 
        BasicBlock::create(module_, "if_else", currentFunction_) : nullptr;
    BasicBlock* mergeBB = BasicBlock::create(module_, "if_merge", currentFunction_);
    
    // 创建条件分支
    if (elseBB) {
        builder_->create_cond_br(condValue, thenBB, elseBB);
    } else {
        builder_->create_cond_br(condValue, thenBB, mergeBB);
    }
    
    // 生成 then 分支
    builder_->set_insert_point(thenBB);
    node->thenStmt->accept(this);
    // 如果 then 分支没有终结指令，添加跳转到 merge
    if (!thenBB->get_terminator()) {
        builder_->create_br(mergeBB);
    }
    
    // 生成 else 分支（如果存在）
    if (elseBB && node->elseStmt) {
        builder_->set_insert_point(elseBB);
        node->elseStmt->accept(this);
        // 如果 else 分支没有终结指令，添加跳转到 merge
        if (!elseBB->get_terminator()) {
            builder_->create_br(mergeBB);
        }
    }
    
    // 切换到 merge 块
    builder_->set_insert_point(mergeBB);
}

void IRGenerator::visit(ReturnStmtNode* node) {
    if (node->expr) {
        // 有返回值
        node->expr->accept(this);
        builder_->create_ret(currentValue_);
    } else {
        // 无返回值
        builder_->create_void_ret();
    }
}

void IRGenerator::visit(VarDefNode* node) {
    Type* varType = currentType_;  // 从 VarDeclNode 设置
    
    if (isGlobalScope_) {
        // 全局变量
        Value* initVal = nullptr;
        if (node->initVal) {
            node->initVal->accept(this);
            initVal = currentValue_;
        }
        
        // 创建全局变量
        // 如果没有初始值，使用 0 初始化
        if (!initVal) {
            if (varType->is_int32_type()) {
                initVal = ConstantInt::get(0, module_);
            } else if (varType->is_float_type()) {
                initVal = ConstantFP::get(0.0f, module_);
            }
        }
        
        GlobalVariable* gv = GlobalVariable::create(
            node->name, module_, varType, false, 
            dynamic_cast<Constant*>(initVal));
        
        namedValues_[node->name] = gv;
    } else {
        // 局部变量
        // 在函数入口块创建 alloca 指令
        BasicBlock* entryBB = &currentFunction_->get_entry_block();
        IRBuilder allocaBuilder(entryBB, module_);
        
        // 创建 alloca 指令
        AllocaInst* alloca = allocaBuilder.create_alloca(varType);
        alloca->set_name(node->name);
        
        // 如果有初始值，生成存储指令
        if (node->initVal) {
            node->initVal->accept(this);
            builder_->create_store(currentValue_, alloca);
        }
        
        namedValues_[node->name] = alloca;
    }
}

void IRGenerator::visit(VarDeclNode* node) {
    // 设置当前类型
    node->type->accept(this);
    
    // 处理所有变量定义
    for (auto& varDef : node->varDefs) {
        varDef->accept(this);
    }
}

void IRGenerator::visit(FuncParamNode* node) {
    // 参数在 FuncDefNode 中处理
}

void IRGenerator::visit(FuncDefNode* node) {
    isGlobalScope_ = false;
    
    // 获取返回类型
    node->returnType->accept(this);
    Type* retType = currentType_;
    
    // 收集参数类型
    std::vector<Type*> paramTypes;
    for (auto& param : node->params) {
        param->type->accept(this);
        paramTypes.push_back(currentType_);
    }
    
    // 创建函数类型
    FunctionType* funcType = FunctionType::get(retType, paramTypes);
    
    // 创建函数
    Function* func = Function::create(funcType, node->name, module_);
    currentFunction_ = func;
    builder_->set_curFunc(func);
    
    // 创建入口基本块
    BasicBlock* entryBB = BasicBlock::create(module_, "entry", func);
    builder_->set_insert_point(entryBB);
    
    // 为参数创建 alloca 并存储参数值
    auto argIt = func->arg_begin();
    for (size_t i = 0; i < node->params.size(); ++i, ++argIt) {
        Argument* arg = *argIt;
        arg->set_name(node->params[i]->name);
        
        // 为参数创建 alloca
        AllocaInst* alloca = builder_->create_alloca(arg->get_type());
        alloca->set_name(node->params[i]->name);
        
        // 存储参数值
        builder_->create_store(arg, alloca);
        
        // 添加到符号表
        namedValues_[node->params[i]->name] = alloca;
    }
    
    // 生成函数体
    if (node->body) {
        node->body->accept(this);
    }
    
    // 如果函数没有返回语句，添加默认返回
    BasicBlock* currentBB = builder_->get_insert_block();
    if (!currentBB->get_terminator()) {
        if (retType->is_void_type()) {
            builder_->create_void_ret();
        } else {
            // 返回默认值
            if (retType->is_int32_type()) {
                builder_->create_ret(ConstantInt::get(0, module_));
            } else if (retType->is_float_type()) {
                builder_->create_ret(ConstantFP::get(0.0f, module_));
            }
        }
    }
    
    // 清理
    namedValues_.clear();
    currentFunction_ = nullptr;
    isGlobalScope_ = true;
}

void IRGenerator::visit(CompUnitNode* node) {
    // 遍历所有顶层项（全局变量和函数）
    for (auto& item : node->items) {
        item->accept(this);
    }
}