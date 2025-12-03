/*!
 * @file IRGenerator.cpp
 * @brief IR生成器实现文件
 * @version 1.0.0
 * @date 2024
 */

#include "IRGenerator.h"
#include <stdexcept>
#include <sstream>

// ==================== 构造函数和析构函数 ====================

IRGenerator::IRGenerator(const std::string& sourceFileName) {
    // 创建模块
    module = new Module("sysy2022_compiler");
    
    // 创建IR构建器（初始基本块为nullptr）
    builder = new IRBuilder(nullptr, module);
    
    // 初始化成员变量
    currentFunction = nullptr;
    currentBB = nullptr;
    tmpIntVal = 0;
    tmpFloatVal = 0.0f;
    tmpIsFloat = false;
    isConstExpr = false;
    trueBB = nullptr;
    falseBB = nullptr;
    
    // 声明运行时库函数
    declareRuntimeFunctions();
}

IRGenerator::~IRGenerator() {
    delete builder;
    // Module由用户管理，不在这里删除
}

void IRGenerator::generate(std::shared_ptr<CompUnitNode> ast) {
    if (ast) {
        visitCompUnit(ast);
        // 设置打印名称
        module->set_print_name();
    }
}

std::string IRGenerator::print() {
    return module->print();
}

// ==================== 辅助函数 ====================

Type* IRGenerator::bTypeToLLVMType(BType bType) {
    switch (bType) {
        case BType::INT:
            return module->get_int32_type();
        case BType::FLOAT:
            return module->get_float_type();
        case BType::VOID:
            return module->get_void_type();
        default:
            return module->get_int32_type();
    }
}

GlobalVariable* IRGenerator::createGlobalVariable(const std::string& name, Type* type, 
                                                   bool isConst, Constant* init) {
    return GlobalVariable::create(name, module, type, isConst, init);
}

AllocaInst* IRGenerator::createLocalVariable(const std::string& name, Type* type) {
    return builder->create_alloca(type);
}

void IRGenerator::declareRuntimeFunctions() {
    // 声明运行时库函数
    std::vector<Type*> emptyParams;
    std::vector<Type*> intParam = {module->get_int32_type()};
    std::vector<Type*> intPtrParam = {module->get_int32_ptr_type()};
    std::vector<Type*> putArrayParams = {module->get_int32_type(), module->get_int32_ptr_type()};
    
    // int getint()
    FunctionType* getintType = FunctionType::get(module->get_int32_type(), emptyParams);
    Function::create(getintType, "getint", module);
    
    // int getch()
    FunctionType* getchType = FunctionType::get(module->get_int32_type(), emptyParams);
    Function::create(getchType, "getch", module);
    
    // int getarray(int*)
    FunctionType* getarrayType = FunctionType::get(module->get_int32_type(), intPtrParam);
    Function::create(getarrayType, "getarray", module);
    
    // void putint(int)
    FunctionType* putintType = FunctionType::get(module->get_void_type(), intParam);
    Function::create(putintType, "putint", module);
    
    // void putch(int)
    FunctionType* putchType = FunctionType::get(module->get_void_type(), intParam);
    Function::create(putchType, "putch", module);
    
    // void putarray(int, int*)
    FunctionType* putarrayType = FunctionType::get(module->get_void_type(), putArrayParams);
    Function::create(putarrayType, "putarray", module);
    
    // void starttime()
    FunctionType* starttimeType = FunctionType::get(module->get_void_type(), emptyParams);
    Function::create(starttimeType, "starttime", module);
    
    // void stoptime()
    FunctionType* stoptimeType = FunctionType::get(module->get_void_type(), emptyParams);
    Function::create(stoptimeType, "stoptime", module);
}

Value* IRGenerator::ensureInt32(Value* val) {
    if (val->get_type()->is_int1_type()) {
        return builder->create_zext(val, module->get_int32_type());
    }
    return val;
}

Value* IRGenerator::ensureInt1(Value* val) {
    if (val->get_type()->is_int32_type()) {
        // 与0比较，非零为true
        return builder->create_icmp_ne(val, ConstantInt::get(0, module));
    }
    return val;
}

int IRGenerator::evalConstInt(std::shared_ptr<ExpNode> node) {
    // 简化的常量求值 - 仅支持数字字面量
    if (auto numNode = std::dynamic_pointer_cast<NumberNode>(node)) {
        return numNode->intVal;
    }
    if (auto addNode = std::dynamic_pointer_cast<AddExpNode>(node)) {
        if (!addNode->left && addNode->right) {
            // 仅有右操作数（mulExp）
            return evalConstInt(addNode->right);
        }
    }
    if (auto mulNode = std::dynamic_pointer_cast<MulExpNode>(node)) {
        if (!mulNode->left && mulNode->right) {
            // 仅有右操作数（unaryExp）
            return evalConstInt(mulNode->right);
        }
    }
    if (auto unaryNode = std::dynamic_pointer_cast<UnaryExpNode>(node)) {
        if (unaryNode->unaryType == UnaryExpNode::UnaryType::PRIMARY) {
            return evalConstInt(unaryNode->primaryExp);
        } else if (unaryNode->unaryType == UnaryExpNode::UnaryType::UNARY_OP) {
            int val = evalConstInt(unaryNode->unaryExp);
            switch (unaryNode->unaryOp) {
                case UnaryOp::PLUS: return val;
                case UnaryOp::MINUS: return -val;
                case UnaryOp::NOT: return val == 0 ? 1 : 0;
            }
        }
    }
    if (auto primaryNode = std::dynamic_pointer_cast<PrimaryExpNode>(node)) {
        if (primaryNode->primaryType == PrimaryExpNode::PrimaryType::NUMBER) {
            return primaryNode->number->intVal;
        } else if (primaryNode->primaryType == PrimaryExpNode::PrimaryType::PAREN_EXP) {
            return evalConstInt(primaryNode->exp);
        }
    }
    return 0;
}

float IRGenerator::evalConstFloat(std::shared_ptr<ExpNode> node) {
    if (auto numNode = std::dynamic_pointer_cast<NumberNode>(node)) {
        return numNode->isFloat ? numNode->floatVal : (float)numNode->intVal;
    }
    // 其他情况返回0
    return 0.0f;
}

// ==================== Visitor函数实现 ====================

void IRGenerator::visitCompUnit(std::shared_ptr<CompUnitNode> node) {
    // 先处理全局声明
    for (auto& decl : node->decls) {
        visitDecl(decl);
    }
    
    // 再处理函数定义
    for (auto& funcDef : node->funcDefs) {
        visitFuncDef(funcDef);
    }
}

void IRGenerator::visitDecl(std::shared_ptr<DeclNode> node) {
    if (auto constDecl = std::dynamic_pointer_cast<ConstDeclNode>(node)) {
        visitConstDecl(constDecl);
    } else if (auto varDecl = std::dynamic_pointer_cast<VarDeclNode>(node)) {
        visitVarDecl(varDecl);
    }
}

void IRGenerator::visitConstDecl(std::shared_ptr<ConstDeclNode> node) {
    for (auto& constDef : node->constDefs) {
        visitConstDef(constDef, node->bType);
    }
}

void IRGenerator::visitVarDecl(std::shared_ptr<VarDeclNode> node) {
    for (auto& varDef : node->varDefs) {
        visitVarDef(varDef, node->bType);
    }
}

void IRGenerator::visitConstDef(std::shared_ptr<ConstDefNode> node, BType bType) {
    Type* type = bTypeToLLVMType(bType);
    std::string name = node->ident;
    
    // 检查重复定义
    if (symbolTable.lookupCurrentScope(name) != nullptr) {
        std::cerr << "Error: 重复定义变量 " << name << std::endl;
        return;
    }
    
    if (symbolTable.isGlobalScope()) {
        // 全局常量
        Constant* init = nullptr;
        if (node->initVal) {
            if (bType == BType::INT) {
                int val = evalConstInt(node->initVal);
                init = ConstantInt::get(val, module);
            } else {
                float val = evalConstFloat(node->initVal);
                init = ConstantFP::get(val, module);
            }
        } else {
            init = ConstantInt::get(0, module);
        }
        
        GlobalVariable* gv = createGlobalVariable(name, type, true, init);
        symbolTable.insert(name, gv, type, true);
    } else {
        // 局部常量 - 当作普通局部变量处理
        AllocaInst* alloca = createLocalVariable(name, type);
        
        if (node->initVal) {
            Value* initVal = visitExp(node->initVal);
            builder->create_store(initVal, alloca);
        }
        
        symbolTable.insert(name, alloca, type, true);
    }
}

void IRGenerator::visitVarDef(std::shared_ptr<VarDefNode> node, BType bType) {
    Type* type = bTypeToLLVMType(bType);
    std::string name = node->ident;
    
    // 检查重复定义
    if (symbolTable.lookupCurrentScope(name) != nullptr) {
        std::cerr << "Error: 重复定义变量 " << name << std::endl;
        return;
    }
    
    if (symbolTable.isGlobalScope()) {
        // 全局变量
        Constant* init = nullptr;
        if (node->initVal) {
            if (bType == BType::INT) {
                int val = evalConstInt(node->initVal);
                init = ConstantInt::get(val, module);
            } else {
                float val = evalConstFloat(node->initVal);
                init = ConstantFP::get(val, module);
            }
        } else {
            // 默认初始化为0
            init = ConstantInt::get(0, module);
        }
        
        GlobalVariable* gv = createGlobalVariable(name, type, false, init);
        symbolTable.insert(name, gv, type, false);
    } else {
        // 局部变量
        AllocaInst* alloca = createLocalVariable(name, type);
        
        if (node->initVal) {
            Value* initVal = visitExp(node->initVal);
            builder->create_store(initVal, alloca);
        }
        
        symbolTable.insert(name, alloca, type, false);
    }
}

void IRGenerator::visitFuncDef(std::shared_ptr<FuncDefNode> node) {
    // 获取返回类型
    Type* retType = bTypeToLLVMType(node->returnType);
    
    // 获取参数类型
    std::vector<Type*> paramTypes;
    for (auto& param : node->params) {
        paramTypes.push_back(bTypeToLLVMType(param->bType));
    }
    
    // 创建函数类型
    FunctionType* funcType = FunctionType::get(retType, paramTypes);
    
    // 创建函数
    Function* func = Function::create(funcType, node->ident, module);
    currentFunction = func;
    builder->set_curFunc(func);
    
    // 将函数添加到符号表（用于递归调用）
    symbolTable.put(node->ident, func);
    
    // 创建入口基本块
    BasicBlock* entryBB = BasicBlock::create(module, node->ident + "_ENTRY", func);
    currentBB = entryBB;
    builder->set_insert_point(entryBB);
    
    // 进入函数作用域
    symbolTable.enterScope();
    
    // 处理参数 - 为每个参数创建alloca并存储
    auto argIt = func->arg_begin();
    for (auto& param : node->params) {
        Type* paramType = bTypeToLLVMType(param->bType);
        AllocaInst* alloca = createLocalVariable(param->ident, paramType);
        builder->create_store(*argIt, alloca);
        symbolTable.insert(param->ident, alloca, paramType, false);
        ++argIt;
    }
    
    // 处理函数体
    if (node->block) {
        // 不要再次调用enterScope，因为上面已经进入了函数作用域
        for (auto& item : node->block->items) {
            visitBlockItem(item);
        }
    }
    
    // 如果基本块没有终结指令，添加默认返回
    if (currentBB && !currentBB->get_terminator()) {
        if (retType->is_void_type()) {
            builder->create_void_ret();
        } else {
            builder->create_ret(ConstantInt::get(0, module));
        }
    }
    
    // 退出函数作用域
    symbolTable.exitScope();
    
    currentFunction = nullptr;
}

void IRGenerator::visitBlock(std::shared_ptr<BlockNode> node) {
    // 进入新作用域
    symbolTable.enterScope();
    
    for (auto& item : node->items) {
        visitBlockItem(item);
    }
    
    // 退出作用域
    symbolTable.exitScope();
}

void IRGenerator::visitBlockItem(std::shared_ptr<BlockItemNode> node) {
    if (node->decl) {
        visitDecl(node->decl);
    } else if (node->stmt) {
        visitStmt(node->stmt);
    }
}

void IRGenerator::visitStmt(std::shared_ptr<StmtNode> node) {
    switch (node->stmtType) {
        case StmtType::ASSIGN:
            visitAssignStmt(node);
            break;
        case StmtType::EXP:
            visitExpStmt(node);
            break;
        case StmtType::BLOCK:
            visitBlockStmt(node);
            break;
        case StmtType::IF:
            visitIfStmt(node);
            break;
        case StmtType::RETURN:
            visitReturnStmt(node);
            break;
    }
}

void IRGenerator::visitAssignStmt(std::shared_ptr<StmtNode> node) {
    // 获取左值地址
    Value* addr = visitLVal(node->lVal, false);
    
    // 计算右值
    Value* val = visitExp(node->exp);
    
    // 存储
    builder->create_store(val, addr);
}

void IRGenerator::visitExpStmt(std::shared_ptr<StmtNode> node) {
    if (node->exp) {
        visitExp(node->exp);
    }
}

void IRGenerator::visitBlockStmt(std::shared_ptr<StmtNode> node) {
    if (node->block) {
        visitBlock(node->block);
    }
}

void IRGenerator::visitIfStmt(std::shared_ptr<StmtNode> node) {
    // 创建基本块
    BasicBlock* thenBB = BasicBlock::create(module, "", currentFunction);
    BasicBlock* elseBB = node->elseStmt ? 
        BasicBlock::create(module, "", currentFunction) : nullptr;
    BasicBlock* mergeBB = BasicBlock::create(module, "", currentFunction);
    
    // 计算条件
    Value* condVal = visitCond(node->cond);
    condVal = ensureInt1(condVal);
    
    // 条件跳转
    if (elseBB) {
        builder->create_cond_br(condVal, thenBB, elseBB);
    } else {
        builder->create_cond_br(condVal, thenBB, mergeBB);
    }
    
    // 生成then分支
    currentBB = thenBB;
    builder->set_insert_point(thenBB);
    visitStmt(node->thenStmt);
    
    // 如果then分支没有终结指令，跳转到merge
    if (!currentBB->get_terminator()) {
        builder->create_br(mergeBB);
    }
    
    // 生成else分支
    if (elseBB && node->elseStmt) {
        currentBB = elseBB;
        builder->set_insert_point(elseBB);
        visitStmt(node->elseStmt);
        
        // 如果else分支没有终结指令，跳转到merge
        if (!currentBB->get_terminator()) {
            builder->create_br(mergeBB);
        }
    }
    
    // 设置当前基本块为merge
    currentBB = mergeBB;
    builder->set_insert_point(mergeBB);
}

void IRGenerator::visitReturnStmt(std::shared_ptr<StmtNode> node) {
    if (node->exp) {
        Value* retVal = visitExp(node->exp);
        builder->create_ret(retVal);
    } else {
        builder->create_void_ret();
    }
}

Value* IRGenerator::visitExp(std::shared_ptr<ExpNode> node) {
    // 根据具体类型分发
    if (auto addExp = std::dynamic_pointer_cast<AddExpNode>(node)) {
        return visitAddExp(addExp);
    }
    if (auto mulExp = std::dynamic_pointer_cast<MulExpNode>(node)) {
        return visitMulExp(mulExp);
    }
    if (auto unaryExp = std::dynamic_pointer_cast<UnaryExpNode>(node)) {
        return visitUnaryExp(unaryExp);
    }
    if (auto primaryExp = std::dynamic_pointer_cast<PrimaryExpNode>(node)) {
        return visitPrimaryExp(primaryExp);
    }
    if (auto lVal = std::dynamic_pointer_cast<LValNode>(node)) {
        return visitLVal(lVal, true);
    }
    if (auto number = std::dynamic_pointer_cast<NumberNode>(node)) {
        return visitNumber(number);
    }
    if (auto relExp = std::dynamic_pointer_cast<RelExpNode>(node)) {
        return visitRelExp(relExp);
    }
    if (auto eqExp = std::dynamic_pointer_cast<EqExpNode>(node)) {
        return visitEqExp(eqExp);
    }
    if (auto lAndExp = std::dynamic_pointer_cast<LAndExpNode>(node)) {
        return visitLAndExp(lAndExp);
    }
    if (auto lOrExp = std::dynamic_pointer_cast<LOrExpNode>(node)) {
        return visitLOrExp(lOrExp);
    }
    
    return nullptr;
}

Value* IRGenerator::visitCond(std::shared_ptr<CondNode> node) {
    if (node->lOrExp) {
        return visitLOrExp(node->lOrExp);
    }
    return nullptr;
}

Value* IRGenerator::visitLVal(std::shared_ptr<LValNode> node, bool load) {
    SymbolInfo* info = symbolTable.lookup(node->ident);
    if (!info) {
        std::cerr << "Error: 未定义的变量 " << node->ident << std::endl;
        return nullptr;
    }
    
    Value* addr = info->value;
    
    if (load) {
        // 检查是否是函数参数（Argument类型）
        if (dynamic_cast<Argument*>(addr)) {
            return addr;  // 参数直接返回值
        }
        // 加载值
        Type* loadType = addr->get_type()->get_pointer_element_type();
        return builder->create_load(loadType, addr);
    } else {
        return addr;  // 返回地址
    }
}

Value* IRGenerator::visitNumber(std::shared_ptr<NumberNode> node) {
    if (node->isFloat) {
        return ConstantFP::get(node->floatVal, module);
    } else {
        return ConstantInt::get(node->intVal, module);
    }
}

Value* IRGenerator::visitPrimaryExp(std::shared_ptr<PrimaryExpNode> node) {
    switch (node->primaryType) {
        case PrimaryExpNode::PrimaryType::PAREN_EXP:
            return visitExp(node->exp);
        case PrimaryExpNode::PrimaryType::LVAL:
            return visitLVal(node->lVal, true);
        case PrimaryExpNode::PrimaryType::NUMBER:
            return visitNumber(node->number);
        default:
            return nullptr;
    }
}

Value* IRGenerator::visitUnaryExp(std::shared_ptr<UnaryExpNode> node) {
    switch (node->unaryType) {
        case UnaryExpNode::UnaryType::PRIMARY:
            return visitPrimaryExp(node->primaryExp);
            
        case UnaryExpNode::UnaryType::FUNC_CALL: {
            // 查找函数
            Value* funcVal = symbolTable.getValue(node->funcName);
            if (!funcVal) {
                std::cerr << "Error: 未定义的函数 " << node->funcName << std::endl;
                return nullptr;
            }
            
            // 准备参数
            std::vector<Value*> args;
            for (auto& arg : node->args) {
                Value* argVal = visitExp(arg);
                args.push_back(argVal);
            }
            
            // 创建调用指令
            return builder->create_call(funcVal, args);
        }
        
        case UnaryExpNode::UnaryType::UNARY_OP: {
            Value* val = visitUnaryExp(node->unaryExp);
            
            switch (node->unaryOp) {
                case UnaryOp::PLUS:
                    return val;
                case UnaryOp::MINUS: {
                    if (val->get_type()->is_float_type()) {
                        return builder->create_fsub(ConstantFP::get(0.0f, module), val);
                    } else {
                        return builder->create_isub(ConstantInt::get(0, module), val);
                    }
                }
                case UnaryOp::NOT: {
                    // !x 等价于 x == 0
                    return builder->create_icmp_eq(val, ConstantInt::get(0, module));
                }
            }
        }
    }
    
    return nullptr;
}

Value* IRGenerator::visitMulExp(std::shared_ptr<MulExpNode> node) {
    if (!node->left) {
        // 只有右操作数（unaryExp）
        return visitUnaryExp(node->right);
    }
    
    // 有左右操作数
    Value* left = visitMulExp(node->left);
    Value* right = visitUnaryExp(node->right);
    
    // 类型提升
    bool isFloat = left->get_type()->is_float_type() || right->get_type()->is_float_type();
    if (isFloat) {
        if (left->get_type()->is_int32_type()) {
            left = builder->create_sitofp(left, module->get_float_type());
        }
        if (right->get_type()->is_int32_type()) {
            right = builder->create_sitofp(right, module->get_float_type());
        }
    }

    switch (node->op) {
        case BinaryOp::MUL:
            return isFloat ? builder->create_fmul(left, right) : builder->create_imul(left, right);
        case BinaryOp::DIV:
            return isFloat ? builder->create_fdiv(left, right) : builder->create_isdiv(left, right);
        case BinaryOp::MOD:
            // LLVM IR中没有浮点取模，这里只处理整数
            return builder->create_irem(left, right);
        default:
            return nullptr;
    }
}

Value* IRGenerator::visitAddExp(std::shared_ptr<AddExpNode> node) {
    if (!node->left) {
        // 只有右操作数（mulExp）
        return visitMulExp(node->right);
    }
    
    // 有左右操作数
    Value* left = visitAddExp(node->left);
    Value* right = visitMulExp(node->right);

    // 类型提升
    bool isFloat = left->get_type()->is_float_type() || right->get_type()->is_float_type();
    if (isFloat) {
        if (left->get_type()->is_int32_type()) {
            left = builder->create_sitofp(left, module->get_float_type());
        }
        if (right->get_type()->is_int32_type()) {
            right = builder->create_sitofp(right, module->get_float_type());
        }
    }
    
    switch (node->op) {
        case BinaryOp::ADD:
            return isFloat ? builder->create_fadd(left, right) : builder->create_iadd(left, right);
        case BinaryOp::SUB:
            return isFloat ? builder->create_fsub(left, right) : builder->create_isub(left, right);
        default:
            return nullptr;
    }
}

Value* IRGenerator::visitRelExp(std::shared_ptr<RelExpNode> node) {
    if (!node->left) {
        // 只有右操作数（addExp）
        Value* val = visitAddExp(node->right);
        // 转换为i1类型用于比较
        return val;
    }
    
    // 有左右操作数
    Value* left = visitRelExp(node->left);
    // 如果左边是比较结果（i1），需要扩展为i32
    left = ensureInt32(left);
    Value* right = visitAddExp(node->right);

    // 类型提升
    bool isFloat = left->get_type()->is_float_type() || right->get_type()->is_float_type();
    if (isFloat) {
        if (left->get_type()->is_int32_type()) {
            left = builder->create_sitofp(left, module->get_float_type());
        }
        if (right->get_type()->is_int32_type()) {
            right = builder->create_sitofp(right, module->get_float_type());
        }
    }
    
    switch (node->op) {
        case RelOp::LT:
            return isFloat ? builder->create_fcmp_lt(left, right) : builder->create_icmp_lt(left, right);
        case RelOp::GT:
            return isFloat ? builder->create_fcmp_gt(left, right) : builder->create_icmp_gt(left, right);
        case RelOp::LE:
            return isFloat ? builder->create_fcmp_le(left, right) : builder->create_icmp_le(left, right);
        case RelOp::GE:
            return isFloat ? builder->create_fcmp_ge(left, right) : builder->create_icmp_ge(left, right);
        default:
            return nullptr;
    }
}

Value* IRGenerator::visitEqExp(std::shared_ptr<EqExpNode> node) {
    if (!node->left) {
        // 只有右操作数（relExp）
        return visitRelExp(node->right);
    }
    
    // 有左右操作数
    Value* left = visitEqExp(node->left);
    left = ensureInt32(left);
    Value* right = visitRelExp(node->right);
    right = ensureInt32(right);

    // 类型提升
    bool isFloat = left->get_type()->is_float_type() || right->get_type()->is_float_type();
    if (isFloat) {
        if (left->get_type()->is_int32_type()) {
            left = builder->create_sitofp(left, module->get_float_type());
        }
        if (right->get_type()->is_int32_type()) {
            right = builder->create_sitofp(right, module->get_float_type());
        }
    }
    
    switch (node->op) {
        case EqOp::EQ:
            return isFloat ? builder->create_fcmp_eq(left, right) : builder->create_icmp_eq(left, right);
        case EqOp::NE:
            return isFloat ? builder->create_fcmp_ne(left, right) : builder->create_icmp_ne(left, right);
        default:
            return nullptr;
    }
}

Value* IRGenerator::visitLAndExp(std::shared_ptr<LAndExpNode> node) {
    if (!node->left) {
        // 只有右操作数（eqExp）
        return visitEqExp(node->right);
    }
    
    // 短路求值：左边为假则整体为假
    // 创建基本块
    BasicBlock* rhsBB = BasicBlock::create(module, "", currentFunction);
    BasicBlock* mergeBB = BasicBlock::create(module, "", currentFunction);
    
    // 计算左操作数
    Value* leftVal = visitLAndExp(node->left);
    leftVal = ensureInt1(leftVal);
    
    BasicBlock* leftBB = currentBB;
    
    // 短路：左边为真则计算右边，否则跳到merge
    builder->create_cond_br(leftVal, rhsBB, mergeBB);
    
    // 计算右操作数
    currentBB = rhsBB;
    builder->set_insert_point(rhsBB);
    Value* rightVal = visitEqExp(node->right);
    rightVal = ensureInt1(rightVal);
    builder->create_br(mergeBB);
    BasicBlock* rightEndBB = currentBB;
    
    // merge基本块
    currentBB = mergeBB;
    builder->set_insert_point(mergeBB);
    
    // 创建phi节点
    PhiInst* phi = PhiInst::create_phi(module->get_int1_type(), mergeBB);
    phi->add_phi_pair_operand(ConstantInt::get(false, module), leftBB);
    phi->add_phi_pair_operand(rightVal, rightEndBB);
    
    return phi;
}

Value* IRGenerator::visitLOrExp(std::shared_ptr<LOrExpNode> node) {
    if (!node->left) {
        // 只有右操作数（lAndExp）
        return visitLAndExp(node->right);
    }
    
    // 短路求值：左边为真则整体为真
    // 创建基本块
    BasicBlock* rhsBB = BasicBlock::create(module, "", currentFunction);
    BasicBlock* mergeBB = BasicBlock::create(module, "", currentFunction);
    
    // 计算左操作数
    Value* leftVal = visitLOrExp(node->left);
    leftVal = ensureInt1(leftVal);
    
    BasicBlock* leftBB = currentBB;
    
    // 短路：左边为假则计算右边，否则跳到merge
    builder->create_cond_br(leftVal, mergeBB, rhsBB);
    
    // 计算右操作数
    currentBB = rhsBB;
    builder->set_insert_point(rhsBB);
    Value* rightVal = visitLAndExp(node->right);
    rightVal = ensureInt1(rightVal);
    builder->create_br(mergeBB);
    BasicBlock* rightEndBB = currentBB;
    
    // merge基本块
    currentBB = mergeBB;
    builder->set_insert_point(mergeBB);
    
    // 创建phi节点
    PhiInst* phi = PhiInst::create_phi(module->get_int1_type(), mergeBB);
    phi->add_phi_pair_operand(ConstantInt::get(true, module), leftBB);
    phi->add_phi_pair_operand(rightVal, rightEndBB);
    
    return phi;
}
