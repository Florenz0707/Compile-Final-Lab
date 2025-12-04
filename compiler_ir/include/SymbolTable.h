/*!
 * @file SymbolTable.h
 * @brief 符号表管理头文件
 * @version 1.0.0
 * @date 2024
 */

#ifndef SYSYC_SYMBOLTABLE_H
#define SYSYC_SYMBOLTABLE_H

#include <string>
#include <map>
#include <vector>
#include <memory>
#include "Value.h"
#include "Type.h"

/**
 * @brief 符号信息结构
 */
struct SymbolInfo {
    Value* value;           // IR中的Value指针
    Type* type;             // 类型
    bool isConst;           // 是否为常量
    bool isGlobal;          // 是否为全局变量
    
    SymbolInfo() : value(nullptr), type(nullptr), isConst(false), isGlobal(false) {}
    SymbolInfo(Value* v, Type* t, bool c, bool g) 
        : value(v), type(t), isConst(c), isGlobal(g) {}
};

/**
 * @brief 作用域类
 */
class Scope {
public:
    std::map<std::string, SymbolInfo> symbols;  // 符号表
    
    Scope() = default;
    
    /**
     * @brief 在当前作用域中查找符号
     * @param name 符号名称
     * @return SymbolInfo* 符号信息指针，未找到返回nullptr
     */
    SymbolInfo* lookup(const std::string& name) {
        auto it = symbols.find(name);
        if (it != symbols.end()) {
            return &it->second;
        }
        return nullptr;
    }
    
    /**
     * @brief 在当前作用域中插入符号
     * @param name 符号名称
     * @param info 符号信息
     * @return bool 成功返回true，重复定义返回false
     */
    bool insert(const std::string& name, const SymbolInfo& info) {
        if (symbols.find(name) != symbols.end()) {
            return false;  // 重复定义
        }
        symbols[name] = info;
        return true;
    }
};

/**
 * @brief 符号表类 - 管理多层作用域
 */
class SymbolTable {
private:
    std::vector<std::shared_ptr<Scope>> scopes;  // 作用域栈

public:
    SymbolTable() {
        // 创建全局作用域
        enterScope();
    }
    
    /**
     * @brief 进入新的作用域
     */
    void enterScope() {
        scopes.push_back(std::make_shared<Scope>());
    }
    
    /**
     * @brief 退出当前作用域
     */
    void exitScope() {
        if (scopes.size() > 1) {  // 保留全局作用域
            scopes.pop_back();
        }
    }
    
    /**
     * @brief 判断当前是否在全局作用域
     * @return bool 在全局作用域返回true
     */
    bool isGlobalScope() const {
        return scopes.size() == 1;
    }
    
    /**
     * @brief 获取当前作用域深度
     * @return size_t 作用域深度
     */
    size_t getScopeDepth() const {
        return scopes.size();
    }
    
    /**
     * @brief 在当前作用域中插入符号
     * @param name 符号名称
     * @param value IR Value指针
     * @param type 类型
     * @param isConst 是否为常量
     * @return bool 成功返回true，重复定义返回false
     */
    bool insert(const std::string& name, Value* value, Type* type, bool isConst) {
        if (scopes.empty()) return false;
        SymbolInfo info(value, type, isConst, isGlobalScope());
        return scopes.back()->insert(name, info);
    }
    
    /**
     * @brief 在当前作用域中插入符号（简化版）
     * @param name 符号名称
     * @param value IR Value指针
     * @return bool 成功返回true
     */
    bool put(const std::string& name, Value* value) {
        if (scopes.empty()) return false;
        SymbolInfo info(value, nullptr, false, isGlobalScope());
        return scopes.back()->insert(name, info);
    }
    
    /**
     * @brief 从当前作用域向外查找符号
     * @param name 符号名称
     * @return SymbolInfo* 符号信息指针，未找到返回nullptr
     */
    SymbolInfo* lookup(const std::string& name) {
        // 从内层作用域向外查找
        for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
            SymbolInfo* info = (*it)->lookup(name);
            if (info != nullptr) {
                return info;
            }
        }
        return nullptr;
    }
    
    /**
     * @brief 仅在当前作用域中查找符号
     * @param name 符号名称
     * @return SymbolInfo* 符号信息指针，未找到返回nullptr
     */
    SymbolInfo* lookupCurrentScope(const std::string& name) {
        if (scopes.empty()) return nullptr;
        return scopes.back()->lookup(name);
    }
    
    /**
     * @brief 获取符号对应的Value
     * @param name 符号名称
     * @return Value* Value指针，未找到返回nullptr
     */
    Value* getValue(const std::string& name) {
        SymbolInfo* info = lookup(name);
        return info ? info->value : nullptr;
    }
    
    /**
     * @brief 获取所有符号的变量映射（兼容示例代码）
     * @return std::map<std::string, Value*> 变量名到Value的映射
     */
    std::map<std::string, Value*> variable() {
        std::map<std::string, Value*> result;
        for (const auto& scope : scopes) {
            for (const auto& pair : scope->symbols) {
                result[pair.first] = pair.second.value;
            }
        }
        return result;
    }
};

#endif // SYSYC_SYMBOLTABLE_H
