package parser.slr;

import model.Production;
import model.Symbol;

import java.util.Objects;

/**
 * LR(0) 项目：产生式 + 圆点位置
 * 形如: A -> α · β
 */
public class LR0Item {
    public final Production production;
    public final int dotPosition;

    public LR0Item(Production production, int dotPosition) {
        this.production = production;
        this.dotPosition = dotPosition;
    }

    // 获取圆点后面的符号 (用于 Goto)
    // 如果圆点在最后，或者遇到 ε 产生式且圆点在 0，返回 null (表示归约)
    public Symbol getSymbolAfterDot() {
        if (production.right.get(0).equals(Symbol.EPSILON)) return null; // ε 产生式视为 A -> · (可直接归约)

        if (dotPosition < production.right.size()) {
            return production.right.get(dotPosition);
        }
        return null;
    }

    public LR0Item shift() {
        return new LR0Item(production, dotPosition + 1);
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        LR0Item lr0Item = (LR0Item) o;
        return dotPosition == lr0Item.dotPosition && Objects.equals(production, lr0Item.production);
    }

    @Override
    public int hashCode() {
        return Objects.hash(production, dotPosition);
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        sb.append(production.left.name).append(" -> ");
        for (int i = 0; i < production.right.size(); i++) {
            if (i == dotPosition) sb.append("· ");
            sb.append(production.right.get(i).name).append(" ");
        }
        if (dotPosition == production.right.size()) sb.append("·");
        return sb.toString();
    }
}