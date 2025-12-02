package parser.slr;

import lexer.Token;
import model.Production;
import model.Symbol;
import parser.Grammar;

import java.util.List;
import java.util.Stack;

public class SLRParser {
    private final SLRTable table;
    private final List<Token> tokens;
    private final Grammar grammar;

    private final Stack<Integer> stateStack = new Stack<>();
    private final Stack<Symbol> symbolStack = new Stack<>();

    public SLRParser(SLRTable table, List<Token> tokens, Grammar grammar) {
        this.table = table;
        this.tokens = tokens;
        this.grammar = grammar;
    }

    public void parse() {
        stateStack.push(0);
        // 栈底符号，虽然LR主要靠状态栈，但为了输出“栈顶符号”，我们需要维护符号栈
        // 初始状态对应空或者 S'，任务书示例第一行是 program#int reduction
        // 说明一开始栈里可能已经有了 program (如果是自底向上，这是最后的归约结果)
        // 等等，任务书示例第一行: 1 program#int reduction
        // 这非常奇怪。自底向上分析，program 是最后才归约出来的。
        // 除非...任务书展示的是【预测分析】(自顶向下) 的输出？
        // 图片标题写着：“语法分析输出示例（以预测分析为例）” [cite: 2878]
        // 而大作业要求的是 SLR (自底向上)。

        // 既然你是做 SLR，你的输出应该是 SLR 的归约过程。
        // SLR 的 Move 输出： 栈顶符号(终结符)#输入 move
        // SLR 的 Reduce 输出：产生式左部(归约出的非终结符)#输入 reduction

        // 我们按 SLR 的逻辑来输出：
        // Move: 显示移进的符号
        // Reduce: 显示归约出的符号

        symbolStack.push(Symbol.EOF); // 对应 ##

        int ip = 0;
        int step = 1;

        while (true) {
            int currentState = stateStack.peek();
            Token currentToken = (ip < tokens.size()) ? tokens.get(ip) : new Token("EOF", "#", null, -1, -1);
            Symbol a = convertTokenToSymbol(currentToken);

            SLRTable.Action action = table.getAction(currentState, a);

            if (action == null || action.type == SLRTable.ActionType.ERROR) {
                System.err.println("Syntax Error at " + currentToken.lexeme + " (State " + currentState + ")");
                return;
            }

            if (action.type == SLRTable.ActionType.SHIFT) {
                // Move 动作
                // 栈顶符号：当前面临的输入符号 a (即将被移进)
                // 任务书示例: 6 int#int move
                // 这里的栈顶符号实际上是操作发生【后】的栈顶，或者是当前匹配的符号。
                // 为了格式整齐，我们打印：当前匹配符号#输入符号 move
                System.out.println(step++ + "\t" + a.name + "#" + currentToken.lexeme + "\tmove");

                stateStack.push(action.id);
                symbolStack.push(a);
                ip++;
            }
            else if (action.type == SLRTable.ActionType.REDUCE) {
                Production p = getProductionById(action.id);

                // Reduction 动作
                // 任务书示例: 5 btype#int reduction
                // 这里的 "btype" 是产生式左部。
                // 输出格式：[产生式左部]#[面临输入符号] reduction
                System.out.println(step++ + "\t" + p.left.name + "#" + a.name + "\treduction");

                int len = p.right.size();
                if (p.right.get(0).equals(Symbol.EPSILON)) len = 0;

                for (int i = 0; i < len; i++) {
                    stateStack.pop();
                    symbolStack.pop();
                }

                int t = stateStack.peek();
                int nextState = table.getGoto(t, p.left);
                if (nextState == -1) {
                    System.err.println("GOTO Error!");
                    return;
                }

                stateStack.push(nextState);
                symbolStack.push(p.left);
            }
            else if (action.type == SLRTable.ActionType.ACCEPT) {
                // Accept 动作
                // 输出：Program#EOF accept
                System.out.println(step++ + "\t" + "Program" + "#" + a.name + "\taccept");
                return;
            }
        }
    }

    private Production getProductionById(int id) {
        for (Production p : grammar.getProductions()) {
            if (p.id == id) return p;
        }
        return null;
    }

    private Symbol convertTokenToSymbol(Token t) {
        String lexeme = t.lexeme.toLowerCase(); // 适配关键字大小写不敏感
        if ("IDN".equals(t.type) || lexeme.equals("main")) return new Symbol("Ident", true);
        if ("INT".equals(t.type)) return new Symbol("IntConst", true);
        if ("FLOAT".equals(t.type)) return new Symbol("floatConst", true);
        if ("EOF".equals(t.type)) return Symbol.EOF;
        return new Symbol(lexeme, true);
    }
}