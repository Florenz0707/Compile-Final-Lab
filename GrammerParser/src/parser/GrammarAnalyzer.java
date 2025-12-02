package parser;

import model.Production;
import model.Symbol;

import java.util.*;

/**
 * 文法分析器：负责计算 FIRST 和 FOLLOW 集
 * 算法基于不动点迭代
 */
public class GrammarAnalyzer {
    private final Grammar grammar;
    private final Map<Symbol, Set<Symbol>> firstSets = new HashMap<>();
    private final Map<Symbol, Set<Symbol>> followSets = new HashMap<>();

    public GrammarAnalyzer(Grammar grammar) {
        this.grammar = grammar;
        initializeSets();
        computeFirstSets();
        computeFollowSets();
    }

    private void initializeSets() {
        // 初始化：所有终结符的 FIRST 集就是它自己
        for (Symbol t : grammar.getTerminals()) {
            firstSets.put(t, new HashSet<>(Collections.singletonList(t)));
        }
        // ε 的 FIRST 集是 {ε}
        firstSets.put(Symbol.EPSILON, new HashSet<>(Collections.singletonList(Symbol.EPSILON)));

        // 非终结符初始化为空集
        for (Symbol nt : grammar.getNonTerminals()) {
            firstSets.put(nt, new HashSet<>());
            followSets.put(nt, new HashSet<>());
        }
    }

    /**
     * 计算 FIRST 集
     * 算法：反复遍历产生式，直到集合不再增大
     */
    private void computeFirstSets() {
        boolean changed = true;
        while (changed) {
            changed = false;
            for (Production p : grammar.getProductions()) {
                Symbol X = p.left;
                // 计算产生式右部符号串的 FIRST 集
                Set<Symbol> rhsFirst = computeSequenceFirst(p.right);

                // 将结果加入左部的 FIRST 集
                if (firstSets.get(X).addAll(rhsFirst)) {
                    changed = true;
                }
            }
        }
    }

    /**
     * 辅助方法：计算符号串 α = Y1 Y2 ... Yn 的 FIRST 集
     * 规则：
     * 1. 加入 FIRST(Y1) - {ε}
     * 2. 如果 Y1 能推出 ε，则继续加入 FIRST(Y2) - {ε}，以此类推
     * 3. 如果所有 Yi 都能推出 ε，则加入 {ε}
     */
    public Set<Symbol> computeSequenceFirst(List<Symbol> sequence) {
        Set<Symbol> result = new HashSet<>();
        boolean allNullable = true;

        for (Symbol Y : sequence) {
            // 获取 Y 的 FIRST 集 (如果 Y 是未初始化的终结符，这里会是 null)
            Set<Symbol> yFirst = firstSets.getOrDefault(Y, new HashSet<>());

            // 将非 ε 元素加入结果
            for (Symbol s : yFirst) {
                if (!s.equals(Symbol.EPSILON)) {
                    result.add(s);
                }
            }

            // 如果 Y 不能推出 ε，则停止向后看
            if (!yFirst.contains(Symbol.EPSILON)) {
                allNullable = false;
                break;
            }
        }

        // 如果所有符号都可能为空，或者序列本身为空(ε)，则整体可为空
        if (allNullable) {
            result.add(Symbol.EPSILON);
        }
        return result;
    }

    /**
     * 计算 FOLLOW 集
     * 算法：
     * 1. 将 # 加入 StartSymbol 的 FOLLOW 集
     * 2. 对 A -> α B β，将 FIRST(β) - {ε} 加入 FOLLOW(B)
     * 3. 对 A -> α B 或 A -> α B β (且 β => ε)，将 FOLLOW(A) 加入 FOLLOW(B)
     */
    private void computeFollowSets() {
        // 1. 初始化 Start Symbol
        Symbol start = grammar.getStartSymbol();
        if (followSets.containsKey(start)) {
            followSets.get(start).add(Symbol.EOF);
        }

        boolean changed = true;
        while (changed) {
            changed = false;
            for (Production p : grammar.getProductions()) {
                Symbol A = p.left;
                List<Symbol> rhs = p.right;

                // 遍历右部的每个符号寻找非终结符
                for (int i = 0; i < rhs.size(); i++) {
                    Symbol B = rhs.get(i);
                    if (B.isTerminal || B.equals(Symbol.EPSILON)) continue;

                    // 找到 B，看它后面的部分 β
                    // β = rhs[i+1 ... end]
                    List<Symbol> beta = rhs.subList(i + 1, rhs.size());
                    Set<Symbol> firstBeta = computeSequenceFirst(beta);

                    // 规则 2: FOLLOW(B) += FIRST(β) - {ε}
                    Set<Symbol> targetFollow = followSets.get(B);
                    int oldSize = targetFollow.size();

                    for (Symbol s : firstBeta) {
                        if (!s.equals(Symbol.EPSILON)) {
                            targetFollow.add(s);
                        }
                    }

                    // 规则 3: 如果 β => ε (包含 β 为空的情况)，FOLLOW(B) += FOLLOW(A)
                    if (firstBeta.contains(Symbol.EPSILON)) {
                        targetFollow.addAll(followSets.get(A));
                    }

                    if (targetFollow.size() > oldSize) {
                        changed = true;
                    }
                }
            }
        }
    }

    public Set<Symbol> getFirst(Symbol s) {
        return firstSets.get(s);
    }

    public Set<Symbol> getFollow(Symbol s) {
        return followSets.get(s);
    }

    public Map<Symbol, Set<Symbol>> getAllFirstSets() { return firstSets; }
    public Map<Symbol, Set<Symbol>> getAllFollowSets() { return followSets; }


    public Grammar getGrammar() { return grammar; }
}