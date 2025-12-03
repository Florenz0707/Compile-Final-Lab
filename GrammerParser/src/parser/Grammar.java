package parser;

import model.Production;
import model.Symbol;

import java.util.*;

/**
 * C-- 文法定义类 (SLR/LR 修正版)
 * 修复了 bType 和 funcType 的 Reduce-Reduce 冲突
 */
public class Grammar {
    private final List<Production> productions = new ArrayList<>();
    private final Set<Symbol> nonTerminals = new HashSet<>();
    private final Set<Symbol> terminals = new HashSet<>();
    private Symbol startSymbol;

    private int currentId = 0;

    public Grammar() {
        initGrammar();
    }

    private void initGrammar() {
        // 0. 增广文法
        this.startSymbol = new Symbol("S'", false);
        addRule("S'", "Program");

        // 1. Program
        addRule("Program", "CompUnit");

        // 2. CompUnit
        addRule("CompUnit", "CompUnit", "Unit");
        addRule("CompUnit", "ε");

        // 辅助: Unit
        addRule("Unit", "decl");
        addRule("Unit", "funcDef");

        // 3. decl
        addRule("decl", "constDecl");
        addRule("decl", "varDecl");

        // 4. constDecl
        addRule("constDecl", "const", "bType", "ConstDefList", ";");

        // ConstDefList
        addRule("ConstDefList", "constDef");
        addRule("ConstDefList", "ConstDefList", ",", "constDef");

        // 5. bType (保留 int/float)
        addRule("bType", "int");
        addRule("bType", "float");

        // 6. constDef
        addRule("constDef", "Ident", "=", "constInitVal");

        // 7. constInitVal
        addRule("constInitVal", "constExp");

        // 8. varDecl
        addRule("varDecl", "bType", "VarDefList", ";");

        // VarDefList
        addRule("VarDefList", "varDef");
        addRule("VarDefList", "VarDefList", ",", "varDef");

        // 9. varDef
        addRule("varDef", "Ident");
        addRule("varDef", "Ident", "=", "initVal");

        // 10. initVal
        addRule("initVal", "exp");

        // 11. funcDef
        // ★★★ 关键修改：移除 funcType，直接展开 ★★★
        // 情况A: void 开头的函数
        addRule("funcDef", "void", "Ident", "(", "FuncFParamsOpt", ")", "block");
        // 情况B: int/float 开头的函数 (复用 bType，解决 Reduce-Reduce 冲突)
        addRule("funcDef", "bType", "Ident", "(", "FuncFParamsOpt", ")", "block");

        // 12. funcType (已废弃，无需定义)

        // 13. funcFParams
        addRule("FuncFParamsOpt", "funcFParams");
        addRule("FuncFParamsOpt", "ε");

        addRule("funcFParams", "funcFParam");
        addRule("funcFParams", "funcFParams", ",", "funcFParam");

        // 14. funcFParam
        addRule("funcFParam", "bType", "Ident");

        // 15. block
        addRule("block", "{", "BlockItemList", "}");

        // BlockItemList
        addRule("BlockItemList", "BlockItemList", "blockItem");
        addRule("BlockItemList", "ε");

        // 16. blockItem
        addRule("blockItem", "decl");
        addRule("blockItem", "stmt");

        // 17. stmt
        addRule("stmt", "LVal", "=", "exp", ";");
        addRule("stmt", "exp", ";");
        addRule("stmt", ";");
        addRule("stmt", "block");
        // 解决 if-else 移进-归约冲突 (依赖 Table 的 Shift 优先策略)
        addRule("stmt", "if", "(", "cond", ")", "stmt");
        addRule("stmt", "if", "(", "cond", ")", "stmt", "else", "stmt");
        addRule("stmt", "return", "exp", ";");
        addRule("stmt", "return", ";");
        addRule("stmt", "while", "(", "cond", ")", "stmt");
        addRule("stmt", "break", ";");
        addRule("stmt", "continue", ";");

        // 18. exp
        addRule("exp", "addExp");

        // 19. cond
        addRule("cond", "lOrExp");

        // 20. LVal
        addRule("LVal", "Ident");

        // 21. primaryExp
        addRule("primaryExp", "(", "exp", ")");
        addRule("primaryExp", "LVal");
        addRule("primaryExp", "number");

        // 22. number
        addRule("number", "IntConst");
        addRule("number", "floatConst");

        // 23. unaryExp
        addRule("unaryExp", "primaryExp");
        addRule("unaryExp", "Ident", "(", "FuncRParamsOpt", ")");
        addRule("unaryExp", "unaryOp", "unaryExp");

        // 24. unaryOp
        addRule("unaryOp", "+");
        addRule("unaryOp", "-");
        addRule("unaryOp", "!");

        // 25. funcRParams
        addRule("FuncRParamsOpt", "funcRParams");
        addRule("FuncRParamsOpt", "ε");

        addRule("funcRParams", "exp");
        addRule("funcRParams", "funcRParams", ",", "exp");

        // 27. mulExp
        addRule("mulExp", "unaryExp");
        addRule("mulExp", "mulExp", "*", "unaryExp");
        addRule("mulExp", "mulExp", "/", "unaryExp");
        addRule("mulExp", "mulExp", "%", "unaryExp");

        // 28. addExp
        addRule("addExp", "mulExp");
        addRule("addExp", "addExp", "+", "mulExp");
        addRule("addExp", "addExp", "-", "mulExp");

        // 29. relExp
        addRule("relExp", "addExp");
        String[] relOps = {"<", ">", "<=", ">="};
        for (String op : relOps) {
            addRule("relExp", "relExp", op, "addExp");
        }

        // 30. eqExp
        addRule("eqExp", "relExp");
        addRule("eqExp", "eqExp", "==", "relExp");
        addRule("eqExp", "eqExp", "!=", "relExp");

        // 31. lAndExp
        addRule("lAndExp", "eqExp");
        addRule("lAndExp", "lAndExp", "&&", "eqExp");

        // 32. lOrExp
        addRule("lOrExp", "lAndExp");
        addRule("lOrExp", "lOrExp", "||", "lAndExp");

        // 33. constExp
        addRule("constExp", "addExp");
    }

    private void addRule(String left, String... rights) {
        Symbol l = getOrCreateNonTerminal(left);
        List<Symbol> r = new ArrayList<>();
        for (String s : rights) {
            if (s.equals("ε")) {
                r.add(Symbol.EPSILON);
            } else {
                boolean isTerm = isTerminalStr(s);
                Symbol sym = isTerm ? getOrCreateTerminal(s) : getOrCreateNonTerminal(s);
                r.add(sym);
            }
        }
        productions.add(new Production(currentId++, l, r));
    }

    private Symbol getOrCreateTerminal(String name) {
        for (Symbol s : terminals) if (s.name.equals(name)) return s;
        Symbol s = new Symbol(name, true);
        terminals.add(s);
        return s;
    }

    private Symbol getOrCreateNonTerminal(String name) {
        for (Symbol s : nonTerminals) if (s.name.equals(name)) return s;
        Symbol s = new Symbol(name, false);
        nonTerminals.add(s);
        return s;
    }

    private boolean isTerminalStr(String s) {
        if (s.equals("Ident") || s.equals("IntConst") || s.equals("floatConst")) return true;
        if (!Character.isLetter(s.charAt(0))) return true;
        if (Set.of("int", "float", "void", "const", "if", "else", "return", "while", "break", "continue").contains(s)) return true;
        return false;
    }

    public List<Production> getProductions() { return productions; }
    public Set<Symbol> getNonTerminals() { return nonTerminals; }
    public Set<Symbol> getTerminals() { return terminals; }
    public Symbol getStartSymbol() { return startSymbol; }
}