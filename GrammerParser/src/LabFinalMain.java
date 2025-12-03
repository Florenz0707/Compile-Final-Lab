import generator.DfaConstructor;
import generator.MinDfaConstructor;
import generator.NfaConstructor;
import lexer.Lexer;
import lexer.Token;
import model.NFA;
import model.State;
import parser.Grammar;
import parser.GrammarAnalyzer;
import parser.slr.SLRParser;
import parser.slr.SLRTable;

import java.util.*;

/**
 * 编译原理大作业主入口 (LabFinal)
 * 流程：正则定义 -> NFA/DFA构造 -> 词法分析 -> SLR语法分析
 */
public class LabFinalMain {

    public static void main(String[] args) {
        // =============================================================
        // 1. 词法分析器初始化 (复用 Lab3)
        // =============================================================
        System.err.println(">>> [Phase 1] 初始化词法分析器 (Lexer)...");

        // 1.1 定义 C-- 词法规则 (正则)
        // 使用 LinkedHashMap 保证匹配优先级 (关键字/运算符 > 标识符)
        Map<String, String> regexMap = new LinkedHashMap<>();

        // 基础字符集定义
        String letter = "(a|b|c|d|e|f|g|h|i|j|k|l|m|n|o|p|q|r|s|t|u|v|w|x|y|z|A|B|C|D|E|F|G|H|I|J|K|L|M|N|O|P|Q|R|S|T|U|V|W|X|Y|Z|_)";
        String digit = "(0|1|2|3|4|5|6|7|8|9)";

        // 关键字 (C-- subset) [cite: 2740]
        // 注意：正则引擎可能需要转义，这里假设 NfaConstructor 能处理基础串
        regexMap.put("int", "int");
        regexMap.put("void", "void");
        regexMap.put("return", "return");
        regexMap.put("const", "const");
        regexMap.put("float", "float");
        regexMap.put("if", "if");
        regexMap.put("else", "else");
        regexMap.put("while", "while");
        regexMap.put("break", "break");
        regexMap.put("continue", "continue");

        // 运算符 [cite: 2741]
        regexMap.put("OP_EQ", "==");
        regexMap.put("OP_LE", "<=");
        regexMap.put("OP_GE", ">=");
        regexMap.put("OP_NE", "!=");
        regexMap.put("OP_AND", "&&");
        regexMap.put("OP_OR", "\\|\\|"); // 需要转义
        regexMap.put("OP_PLUS", "\\+");
        regexMap.put("OP_MINUS", "-");
        regexMap.put("OP_MUL", "\\*");
        regexMap.put("OP_DIV", "/");
        regexMap.put("OP_MOD", "%");
        regexMap.put("OP_ASSIGN", "=");
        regexMap.put("OP_GT", ">");
        regexMap.put("OP_LT", "<");
        regexMap.put("OP_NOT", "!");

        // 界符 [cite: 2742]
        regexMap.put("SE_LPAREN", "\\(");
        regexMap.put("SE_RPAREN", "\\)");
        regexMap.put("SE_LBRACE", "\\{");
        regexMap.put("SE_RBRACE", "\\}");
        regexMap.put("SE_SEMICOLON", ";");
        regexMap.put("SE_COMMA", ",");

        // 标识符与常量 [cite: 2743-2745]
        // 放在关键字之后，确保最长匹配优先或顺序优先
        regexMap.put("IDN", letter + "(" + letter + "|" + digit + ")*");
        regexMap.put("FLOAT", digit + "+" + "\\." + digit + "+"); // 浮点数要在整数前匹配
        regexMap.put("INT", digit + "+");

        // 空白符 (用于过滤)
        regexMap.put("WHITESPACE", "( |\\t|\\n|\\r)+");

        // 1.2 定义 Token 种别码 (可选，主要用于 debug 或特定输出格式)
        Map<String, Integer> tokenValues = new HashMap<>();
        int code = 1;
        for (String key : regexMap.keySet()) {
            tokenValues.put(key, code++);
        }

        // 1.3 构建自动机: Regex -> NFA -> DFA -> MinDFA
        NfaConstructor thompson = new NfaConstructor();
        List<State> nfaStartStates = new ArrayList<>();

        for (Map.Entry<String, String> entry : regexMap.entrySet()) {
            String ruleName = entry.getKey();
            // 提取类型前缀 (如 OP_PLUS -> OP)
            String tokenType = ruleName.contains("_") ? ruleName.split("_")[0] :
                    (ruleName.equals("IDN") || ruleName.equals("INT") || ruleName.equals("FLOAT") || ruleName.equals("WHITESPACE")) ? ruleName : "KW";

            int tokenValue = tokenValues.getOrDefault(ruleName, 0);

            // 构造该规则的 NFA
            NFA nfa = thompson.construct(entry.getValue(), tokenType, tokenValue);
            nfaStartStates.add(nfa.startState);
        }

        // 合并所有 NFA
        State giantNfaStart = new State();
        for (State start : nfaStartStates) {
            giantNfaStart.addNfaTransition(null, start);
        }

        // 子集构造法 NFA -> DFA
        DfaConstructor subset = new DfaConstructor();
        State dfaStart = subset.NfaToDfa(giantNfaStart);

        // DFA 最小化
        MinDfaConstructor minimizer = new MinDfaConstructor();
        State minDfaStart = minimizer.minimize(dfaStart);

        System.err.println(">>> [Phase 1] 词法分析器构建完成。");

        // =============================================================
        // 2. SLR 语法分析器初始化 (大作业核心)
        // =============================================================
        System.err.println(">>> [Phase 2] 初始化 SLR 语法分析器...");

        // 2.1 加载原始 C-- 文法 (保留左递归) [cite: 2763]
        Grammar grammar = new Grammar();

        // 2.2 计算 FOLLOW 集 [cite: 2764]
        GrammarAnalyzer analyzer = new GrammarAnalyzer(grammar);

        // 2.3 构造 SLR 分析表 (Action / Goto) [cite: 2765]
        SLRTable slrTable = new SLRTable(grammar, analyzer);

        System.err.println(">>> [Phase 2] SLR 分析表构造完成。");

        // =============================================================
        // 3. 执行编译流程 (输入 -> 词法 -> 语法)
        // =============================================================
        System.err.println(">>> [Phase 3] 读取输入并开始分析...");

        // 3.1 读取源代码
        String sourceCode = "";
        try (Scanner scanner = new Scanner(System.in, "UTF-8")) {
            if (scanner.hasNext()) {
                sourceCode = scanner.useDelimiter("\\A").next();
            }
        }

        if (sourceCode.isEmpty()) {
            System.err.println("错误: 输入为空。请使用输入重定向: java LabFinalMain < test.sy");
            return;
        }

        // 3.2 执行词法分析
        Lexer lexer = new Lexer(sourceCode, minDfaStart);
        List<Token> rawTokens = lexer.scanTokens();

        // 3.3 过滤空白符
        List<Token> tokens = new ArrayList<>();
        for (Token t : rawTokens) {
            if (!"WHITESPACE".equals(t.type)) {
                tokens.add(t);
            }
        }

        System.err.println(">>> 词法分析完成，有效 Token 数: " + tokens.size());

        // 3.4 执行 SLR 语法分析
        System.out.println("序号\t栈顶符号#面临符号\t执行动作");
        SLRParser parser = new SLRParser(slrTable, tokens, grammar);
        parser.parse();

        System.err.println(">>> 分析结束。");
    }
}