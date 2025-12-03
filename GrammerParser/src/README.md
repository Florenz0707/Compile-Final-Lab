## 1. 项目简介

本项目实现了一个针对 **C-- 语言**（C 语言子集）的编译器前端。系统完全**手工实现**，未借助 Lex/Yacc 或 ANTLR 等编译器生成工具 。



项目主要包含两个核心模块：

1. **词法分析器 (Lexer)**：基于正则表达式构建 NFA，通过子集构造法生成 DFA，并使用 Hopcroft 算法进行最小化，最终输出 Token 流。

   

   

2. **语法分析器 (Parser)**：基于 **SLR(1)** 算法（自底向上分析），自动构造 LR(0) 项目集规范族、计算 FIRST/FOLLOW 集、生成 Action/Goto 表，并实现基于状态栈的移进-归约（Shift-Reduce）驱动引擎。

   

   

## 2. 系统架构

项目采用严格的分层架构设计，核心代码位于 `src` 目录下：

Plaintext

```
src/
├── LabFinalMain.java         // [入口] 程序主入口，串联词法与语法分析
├── model/                    // [模型层] 核心数据结构
│   ├── Symbol.java           // 文法符号 (区分终结符/非终结符)
│   ├── Production.java       // 产生式规则
│   ├── State.java            // 自动机状态节点
│   └── NFA.java              // NFA 图结构
├── lexer/                    // [词法层] 词法分析运行时
│   ├── Lexer.java            // DFA 驱动器
│   └── Token.java            // 词法单元实体
├── generator/                // [生成器] 词法分析器生成逻辑
│   ├── NfaConstructor.java   // 正则 -> NFA (Thompson算法)
│   ├── DfaConstructor.java   // NFA -> DFA (子集构造法)
│   └── MinDfaConstructor.java// DFA 最小化 (Hopcroft算法)
└── parser/                   // [语法层] SLR 语法分析核心
    ├── Grammar.java          // C-- 文法定义 (保留左递归)
    ├── GrammarAnalyzer.java  // FIRST/FOLLOW 集计算引擎
    └── slr/                  // SLR 算法实现
        ├── LR0Item.java      // LR(0) 项目定义 (A -> α·β)
        ├── SLRTable.java     // 预测表构造器 (Closure, Goto, 填表)
        └── SLRParser.java    // 移进-归约驱动引擎
```

## 3. 核心特性

### 3.1 文法设计 (Grammar Design)

与 LL(1) 分析不同，本项目采用了 **SLR(1)** 分析法，因此文法设计更加自然：

- **保留左递归**：表达式文法（如 `addExp -> addExp + mulExp`）保留了直接左递归，使得语法树能够自然体现左结合性，无需引入复杂的辅助非终结符 。

  

- **冲突解决**：针对 `int a` (变量) 与 `int main` (函数) 的 Reduce-Reduce 冲突，通过提取左公因子（将 `funcType` 展开到 `funcDef` 中）进行了文法重构。

### 3.2 算法实现 (Algorithms)

- **自动机理论**：实现了完整的正则引擎流水线。
- **不动点迭代**：用于计算 FIRST 和 FOLLOW 集合。
- **SLR 表构造**：
  - 实现了 `Closure` (闭包) 和 `Goto` (状态转移) 算法。
  - 实现了规范项集族（DFA）的自动构建。
  - **冲突处理**：在填表时实现了 **Shift-Over-Reduce** 策略，有效解决了悬挂 `else` (Dangling Else) 的 Shift-Reduce 冲突。
- **错误恢复**：实现了**恐慌模式 (Panic Mode)**，在遇到语法错误时能够跳过非法 Token 直到遇到同步符号，从而发现后续错误而不直接崩溃。

## 4. 编译与运行

### 环境要求

- JDK 11 或更高版本。
- 无第三方依赖。