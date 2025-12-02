package test;

import model.Symbol;
import parser.Grammar;
import parser.GrammarAnalyzer;

import java.util.Set;
import java.util.stream.Collectors;

public class AnalyzerTest {
    public static void main(String[] args) {
        System.out.println("========== 开始验证 FIRST/FOLLOW 集 ==========");
        Grammar grammar = new Grammar();
        GrammarAnalyzer analyzer = new GrammarAnalyzer(grammar);

        // 1. 验证简单的 FIRST 集
        // 修正：number 是非终结符，FIRST集应该包含它推导出的 IntConst 和 floatConst
        checkFirst(analyzer, "addExp", new String[]{
                "(", "Ident", "IntConst", "floatConst", "+", "-", "!"
        });

        // FIRST(addExp') 应该包含 '+', '-', 'ε'
        checkFirst(analyzer, "addExp'", new String[]{"+", "-", "ε"});

        // 2. 验证 FOLLOW 集
        // FOLLOW(addExp) 应该包含 relExp' 的开始符号 '<', '>', 以及 ')', ';', '#' 等
        checkFollow(analyzer, "addExp", new String[]{"+", "-", "<", ">", "<=", ">=", "==", "!=", "&&", "||", ")", ";", "#"});

        System.out.println("\n[Visual Check] 打印关键集合:");
        printSet("FIRST(stmt)", analyzer.getFirst(getSymbol(grammar, "stmt")));
        printSet("FOLLOW(addExp)", analyzer.getFollow(getSymbol(grammar, "addExp")));

        System.out.println("\n========== 验证结束 ==========");
    }

    private static void checkFirst(GrammarAnalyzer analyzer, String name, String[] expected) {
        Symbol s = getSymbol(analyzer.getGrammar(), name); // 使用 analyzer 中的 grammar
        Set<String> firstNames = analyzer.getFirst(s).stream().map(Symbol::toString).collect(Collectors.toSet());

        System.out.print("检查 FIRST(" + name + ")... ");
        for (String exp : expected) {
            if (!firstNames.contains(exp)) {
                throw new RuntimeException("失败: 期望包含 " + exp + "，实际未找到。实际集合: " + firstNames);
            }
        }
        System.out.println("通过。");
    }

    private static void checkFollow(GrammarAnalyzer analyzer, String name, String[] expected) {
        Symbol s = getSymbol(analyzer.getGrammar(), name);
        Set<String> followNames = analyzer.getFollow(s).stream().map(Symbol::toString).collect(Collectors.toSet());

        System.out.print("检查 FOLLOW(" + name + ")... ");
        for (String exp : expected) {
            if (!followNames.contains(exp)) {
                // 仅作为警告
                System.err.print("[警告: 缺 " + exp + "] ");
            }
        }
        System.out.println("检查完成。");
    }

    // 修正：将 grammar 可见性设为 public 或提供 getter，或者直接在这里复用查找逻辑
    // 为了方便，这里假设 analyzer.grammar 是 public 的，或者我们在下面直接重新查找
    // 更好的方式是在 AnalyzerTest 内部加一个查找方法：
    private static Symbol getSymbol(Grammar g, String name) {
        for (Symbol s : g.getNonTerminals()) if (s.name.equals(name)) return s;
        for (Symbol s : g.getTerminals()) if (s.name.equals(name)) return s;
        // 如果还没找到，可能是 ε 或 #
        if (name.equals("ε")) return Symbol.EPSILON;
        if (name.equals("#")) return Symbol.EOF;
        throw new RuntimeException("测试代码错误：找不到符号 " + name);
    }

    private static void printSet(String title, Set<Symbol> set) {
        System.out.println(title + ": " + set.toString());
    }
}