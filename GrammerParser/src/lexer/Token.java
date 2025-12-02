package lexer;

public class Token {
    public final String type;
    public final String lexeme;
    public final Object value;
    public final int line;
    public final int column;

    public Token(String type, String lexeme, Object value, int line, int column) {
        this.type = type;
        this.lexeme = lexeme;
        this.value = value;
        this.line = line;
        this.column = column;
    }

    public String toTestFormat() {
        String valueStr;
        if ("ERROR".equals(type)) {
            valueStr = String.format("<ERROR,%d,%d>", line, column);
        } else if ("IDN".equals(type) || "INT".equals(type) || "FLOAT".equals(type)) {
            valueStr = String.format("<%s,%s>", type, value);
        } else {
            valueStr = String.format("<%s,%s>", type, value);
        }
        return lexeme + "\t" + valueStr;
    }
}
