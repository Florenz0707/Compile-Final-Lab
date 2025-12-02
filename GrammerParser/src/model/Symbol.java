package model;

import java.util.Objects;

public class Symbol {
    public final String name;
    public final boolean isTerminal;

    public Symbol(String name, boolean isTerminal) {
        this.name = name;
        this.isTerminal = isTerminal;
    }

    public static final Symbol EPSILON = new Symbol("ε", true);
    public static final Symbol EOF = new Symbol("#", true);

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        Symbol symbol = (Symbol) o;
        return isTerminal == symbol.isTerminal && Objects.equals(name, symbol.name);
    }

    @Override
    public int hashCode() {
        return Objects.hash(name, isTerminal);
    }

    @Override
    public String toString() {
        return name;
    }
}