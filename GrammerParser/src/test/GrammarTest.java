package test;

import parser.Grammar;
import model.Production; // 修改这里
import model.Symbol;     // 修改这里

import java.util.List;
import java.util.Set;
import java.util.stream.Collectors;

public class GrammarTest {

    public static void main(String[] args) {
        System.out.println("========== 开始验证 Grammar.java ==========");
        Grammar grammar = new Grammar();

        testSymbolClassification(grammar);
        testLeftRecursionElimination(grammar);
        testLeftFactoring(grammar);
        printAllProductions(grammar);

        System.out.println("\n========== 验证结束：请检查是否有报错 ==========");
    }

    private static void testSymbolClassification(Grammar g) {
        System.out.println("\n[Test 1] 验证符号分类...");
        Set<String> terminals = g.getTerminals().stream().map(s -> s.name).collect(Collectors.toSet());
        Set<String> nonTerminals = g.getNonTerminals().stream().map(s -> s.name).collect(Collectors.toSet());

        assertIsTerminal(terminals, "int");
        assertIsTerminal(terminals, "+");
        assertIsTerminal(terminals, "{");
        assertIsTerminal(terminals, "Ident");

        assertIsNonTerminal(nonTerminals, "exp");
        assertIsNonTerminal(nonTerminals, "stmt");
        assertIsNonTerminal(nonTerminals, "addExp");
        assertIsNonTerminal(nonTerminals, "addExp'");

        System.out.println(">> 符号分类验证通过。");
    }

    private static void testLeftRecursionElimination(Grammar g) {
        System.out.println("\n[Test 2] 验证左递归消除 (Target: addExp)...");
        List<Production> adds = getRulesFor(g, "addExp");
        boolean hasRightRecursion = false;
        boolean hasLeftRecursion = false;

        for (Production p : adds) {
            if (p.right.get(0).name.equals("addExp")) {
                hasLeftRecursion = true;
            }
            if (p.right.size() > 1 && p.right.get(1).name.equals("addExp'")) {
                hasRightRecursion = true;
            }
        }

        if (hasLeftRecursion) throw new RuntimeException("失败: addExp 仍包含左递归!");
        if (!hasRightRecursion) throw new RuntimeException("失败: addExp 未正确转换为右递归形式!");

        System.out.println(">> addExp 左递归消除验证通过。");
    }

    private static void testLeftFactoring(Grammar g) {
        System.out.println("\n[Test 3] 验证左公因子提取 (Target: stmt)...");
        List<Production> stmts = getRulesFor(g, "stmt");
        boolean hasIdentPrefix = false;

        for (Production p : stmts) {
            if (p.right.get(0).name.equals("Ident")) {
                if (p.right.size() > 1 && p.right.get(1).name.equals("stmt_ident_tail")) {
                    hasIdentPrefix = true;
                } else {
                    throw new RuntimeException("失败: stmt -> Ident 后未接辅助符号!");
                }
            }
        }

        if (!hasIdentPrefix) throw new RuntimeException("失败: stmt 缺少以 Ident 开头的产生式!");
        System.out.println(">> stmt 左公因子提取验证通过。");
    }

    private static void printAllProductions(Grammar g) {
        System.out.println("\n[Visual Check] 打印所有产生式列表 (共 " + g.getProductions().size() + " 条):");
        for (Production p : g.getProductions()) {
            System.out.println(p.toString());
        }
    }

    private static void assertIsTerminal(Set<String> set, String name) {
        if (!set.contains(name)) throw new RuntimeException("错误: " + name + " 应该被识别为终结符。");
    }
    private static void assertIsNonTerminal(Set<String> set, String name) {
        if (!set.contains(name)) throw new RuntimeException("错误: " + name + " 应该被识别为非终结符。");
    }
    private static List<Production> getRulesFor(Grammar g, String leftName) {
        return g.getProductions().stream()
                .filter(p -> p.left.name.equals(leftName))
                .collect(Collectors.toList());
    }
}