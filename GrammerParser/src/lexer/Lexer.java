package lexer;

import model.State;
import java.util.*;

public class Lexer {
    private final String source;
    private final State dfaStart;

    private final Map<String, Integer> keywords = new HashMap<>();

    private final List<Token> tokens = new ArrayList<>();
    private int current = 0;
    private int line = 1;
    private int column = 1;

    public Lexer(String source, State dfaStart) {
        this.source = source;
        this.dfaStart = dfaStart;

        keywords.put("int", 1);
        keywords.put("void", 2);
        keywords.put("return", 3);
        keywords.put("const", 4);
        keywords.put("main", 5);
        keywords.put("float", 6);
        keywords.put("if", 7);
        keywords.put("else", 8);
    }

    public List<Token> scanTokens() {
        while (current < source.length()) {
            scanToken();
        }
        return tokens;
    }

    private void scanToken() {
        State currentState = dfaStart;
        State lastAcceptState = null;
        int lastAcceptPosition = -1;
        int forward = current;

        while (forward < source.length()) {
            char c = source.charAt(forward);
            State nextState = currentState.getDfaTransition(c);

            if (nextState == null) {
                break;
            }

            currentState = nextState;

            if (currentState.isAccept) {
                lastAcceptState = currentState;
                lastAcceptPosition = forward;
            }

            forward++;
        }

        if (lastAcceptState == null) {
            char c = source.charAt(current);
            if (Character.isWhitespace(c)) {
                if (c == '\n') {
                    line++;
                    column = 1;
                } else {
                    column++;
                }
                current++;
                return;
            }

            addToken("ERROR", String.valueOf(c), null);
            current++;

        } else {
            String lexeme = source.substring(current, lastAcceptPosition + 1);
            String tokenType = lastAcceptState.tokenType;
            Object tokenValue = lastAcceptState.tokenValue;

            if ("IDN".equals(tokenType)) {
                String lowerLexeme = lexeme.toLowerCase();
                if (keywords.containsKey(lowerLexeme)) {
                    tokenType = "KW";
                    tokenValue = keywords.get(lowerLexeme);
                } else {
                    tokenValue = lexeme;
                }
            }

            if ("INT".equals(tokenType)) {
                tokenValue = lexeme;
            }
            if ("FLOAT".equals(tokenType)) {
                tokenValue = lexeme;
            }

            if ("WHITESPACE".equals(tokenType)) {
                for (char c : lexeme.toCharArray()) {
                    if (c == '\n') {
                        line++;
                        column = 1;
                    } else {
                        column++;
                    }
                }
                current = lastAcceptPosition + 1;
                return;
            }

            addToken(tokenType, lexeme, tokenValue);

            current = lastAcceptPosition + 1;
        }
    }

    private void addToken(String type, String lexeme, Object value) {
        int startLine = line;
        int startColumn = column;

        for (char c : lexeme.toCharArray()) {
            if (c == '\n') {
                line++;
                column = 1;
            } else {
                column++;
            }
        }

        tokens.add(new Token(type, lexeme, value, startLine, startColumn));
    }
}
