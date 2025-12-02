package model;

import java.util.List;

public class Production {
    public final int id;
    public final Symbol left;
    public final List<Symbol> right;

    public Production(int id, Symbol left, List<Symbol> right) {
        this.id = id;
        this.left = left;
        this.right = right;
    }

    @Override
    public String toString() {
        return String.format("%d. %s -> %s", id, left.name, right.toString());
    }
}