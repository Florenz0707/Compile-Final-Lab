package parser.slr;

import model.Production;
import model.Symbol;
import parser.Grammar;
import parser.GrammarAnalyzer;

import java.util.*;

public class SLRTable {
    public enum ActionType { SHIFT, REDUCE, ACCEPT, ERROR }

    public static class Action {
        public ActionType type;
        public int id; // 状态ID (Shift) 或 产生式ID (Reduce)

        public Action(ActionType type, int id) {
            this.type = type;
            this.id = id;
        }
        @Override public String toString() { return type + " " + id; }
    }

    // 状态集规范族 (Canonical Collection of LR(0) items)
    // List 的索引即为状态 ID
    private final List<Set<LR0Item>> states = new ArrayList<>();

    // 两个核心表
    private final Map<Integer, Map<Symbol, Action>> actionTable = new HashMap<>();
    private final Map<Integer, Map<Symbol, Integer>> gotoTable = new HashMap<>();

    private final Grammar grammar;
    private final GrammarAnalyzer analyzer;

    public SLRTable(Grammar grammar, GrammarAnalyzer analyzer) {
        this.grammar = grammar;
        this.analyzer = analyzer;
        buildCanonicalCollection();
        buildParsingTable();
    }

    // --- 1. 构造规范项集族 (DFA) ---

    private void buildCanonicalCollection() {
        // 初始状态 I0 = Closure({S' -> · S})
        Production startProd = grammar.getProductions().get(0); // 假设第一个是增广产生式 S' -> Program
        Set<LR0Item> startKernel = new HashSet<>();
        startKernel.add(new LR0Item(startProd, 0));

        Set<LR0Item> I0 = closure(startKernel);
        states.add(I0);

        // 广度优先遍历构造所有状态
        int processedStateCount = 0;
        while (processedStateCount < states.size()) {
            Set<LR0Item> I = states.get(processedStateCount);
            int currentInfoId = processedStateCount;
            processedStateCount++;

            // 找出所有可能的后继符号 X (Symbol after dot)
            Set<Symbol> transitionSymbols = new HashSet<>();
            for (LR0Item item : I) {
                Symbol sym = item.getSymbolAfterDot();
                if (sym != null) transitionSymbols.add(sym);
            }

            for (Symbol X : transitionSymbols) {
                Set<LR0Item> J = gotoSet(I, X);
                if (J.isEmpty()) continue;

                // 检查 J 是否已存在
                int existingStateId = -1;
                for (int i = 0; i < states.size(); i++) {
                    if (states.get(i).equals(J)) {
                        existingStateId = i;
                        break;
                    }
                }

                if (existingStateId == -1) {
                    states.add(J);
                    existingStateId = states.size() - 1;
                }

                // 记录转移: GOTO(I, X) = J
                // 如果 X 是终结符，这是 Shift 动作的基础；如果是非终结符，是 Goto 表
                if (X.isTerminal) {
                    // 暂不填表，只记录关系，填表在 buildParsingTable 统一做
                }
                recordTransition(currentInfoId, X, existingStateId);
            }
        }
    }

    // Closure 算法
    private Set<LR0Item> closure(Set<LR0Item> items) {
        Set<LR0Item> closureSet = new HashSet<>(items);
        boolean changed = true;
        while (changed) {
            changed = false;
            Set<LR0Item> temp = new HashSet<>();
            for (LR0Item item : closureSet) {
                Symbol B = item.getSymbolAfterDot();
                // 若 A -> α · B β，且 B 是非终结符
                if (B != null && !B.isTerminal) {
                    // 将所有 B -> · γ 加入
                    for (Production p : grammar.getProductions()) {
                        if (p.left.equals(B)) {
                            LR0Item newItem = new LR0Item(p, 0);
                            if (!closureSet.contains(newItem) && !temp.contains(newItem)) {
                                temp.add(newItem);
                                changed = true;
                            }
                        }
                    }
                }
            }
            closureSet.addAll(temp);
        }
        return closureSet;
    }

    // Goto 算法
    private Set<LR0Item> gotoSet(Set<LR0Item> I, Symbol X) {
        Set<LR0Item> J = new HashSet<>();
        for (LR0Item item : I) {
            Symbol sym = item.getSymbolAfterDot();
            if (sym != null && sym.equals(X)) {
                J.add(item.shift());
            }
        }
        return closure(J);
    }

    private void recordTransition(int fromState, Symbol symbol, int toState) {
        if (symbol.isTerminal) {
            // Action 表暂存，类型为 SHIFT
            // 稍后在 buildParsingTable 中处理，这里也可以直接存
        } else {
            gotoTable.computeIfAbsent(fromState, k -> new HashMap<>()).put(symbol, toState);
        }
    }

    // --- 2. 构造 SLR 分析表 ---

    private void buildParsingTable() {
        for (int i = 0; i < states.size(); i++) {
            Set<LR0Item> I = states.get(i);

            // 处理 Shift 和 Goto (基于状态机边)
            // 这里我们需要重新扫描一遍 Goto 关系吗？不需要，可以直接根据闭包项推导

            // 遍历该状态所有项
            for (LR0Item item : I) {
                Symbol next = item.getSymbolAfterDot();

                // Rule 1: Shift
                // 若 A -> α · a β，a 为终结符，且 GOTO(I, a) = Ij
                if (next != null && next.isTerminal) {
                    Set<LR0Item> nextStateSet = gotoSet(I, next);
                    int nextStateId = getStateId(nextStateSet);
                    addAction(i, next, new Action(ActionType.SHIFT, nextStateId));
                }

                // Rule 2: Reduce
                // 若 A -> α · (点在最后)，对 FOLLOW(A) 中所有符号 a，置 Action[i, a] = Reduce A -> α
                if (next == null) {
                    if (item.production.left.name.equals("S'")) {
                        // S' -> S · , Accept
                        addAction(i, Symbol.EOF, new Action(ActionType.ACCEPT, 0));
                    } else {
                        // 归约
                        Set<Symbol> followA = analyzer.getFollow(item.production.left);
                        for (Symbol a : followA) {
                            addAction(i, a, new Action(ActionType.REDUCE, item.production.id));
                        }
                    }
                }
            }
        }
    }

    private int getStateId(Set<LR0Item> stateSet) {
        for (int i = 0; i < states.size(); i++) {
            if (states.get(i).equals(stateSet)) return i;
        }
        throw new RuntimeException("State not found!");
    }

    // 在 SLRTable.java 中
    private void addAction(int state, Symbol sym, Action action) {
        actionTable.computeIfAbsent(state, k -> new HashMap<>());
        Map<Symbol, Action> row = actionTable.get(state);

        if (row.containsKey(sym)) {
            Action existing = row.get(sym);
            if (existing.type != action.type || existing.id != action.id) {
                // 解决 if-else 冲突：Shift 优先
                if (existing.type == ActionType.SHIFT && action.type == ActionType.REDUCE) {
                    // 已经有 Shift，忽略当前的 Reduce，保留 Shift
                    return;
                }
                if (existing.type == ActionType.REDUCE && action.type == ActionType.SHIFT) {
                    // 现有是 Reduce，新来的是 Shift -> 覆盖为 Shift
                    row.put(sym, action);
                    return;
                }

                // 其他冲突打印警告
                System.err.println("SLR Conflict in State " + state + " on symbol " + sym +
                        " Existing: " + existing + " New: " + action);
            }
        } else {
            row.put(sym, action);
        }
    }

    public Action getAction(int state, Symbol sym) {
        if (actionTable.containsKey(state)) {
            return actionTable.get(state).get(sym);
        }
        return new Action(ActionType.ERROR, -1);
    }

    public int getGoto(int state, Symbol nonTerminal) {
        if (gotoTable.containsKey(state) && gotoTable.get(state).containsKey(nonTerminal)) {
            return gotoTable.get(state).get(nonTerminal);
        }
        return -1; // Error
    }
}