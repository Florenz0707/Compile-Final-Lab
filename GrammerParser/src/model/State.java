package model;

import java.util.*;

public class State {
    private static int idCounter = 0;
    public final int id;

    private final Map<Character, Set<State>> nfaTransitions = new HashMap<>();

    private final Map<Character, State> dfaTransitions = new HashMap<>();

    public boolean isAccept = false;

    public String tokenType = null;

    public int tokenValue = -1;

    public State() {
        this.id = idCounter++;
    }

    public void addNfaTransition(Character symbol, State nextState) {
        nfaTransitions.computeIfAbsent(symbol, k -> new HashSet<>()).add(nextState);
    }

    public Set<State> getNfaTransitions(Character symbol) {
        return nfaTransitions.getOrDefault(symbol, Collections.emptySet());
    }

    public Map<Character, Set<State>> getAllNfaTransitions() {
        return nfaTransitions;
    }

    public void setDfaTransition(Character symbol, State nextState) {
        dfaTransitions.put(symbol, nextState);
    }

    public State getDfaTransition(Character symbol) {
        return dfaTransitions.get(symbol);
    }

    public Map<Character, State> getAllDfaTransitions() {
        return dfaTransitions;
    }

    @Override
    public String toString() {
        return "S" + id;
    }

    @Override
    public int hashCode() {
        return id;
    }

    @Override
    public boolean equals(Object obj) {
        if (this == obj) return true;
        if (obj == null || getClass() != obj.getClass()) return false;
        return this.id == ((State) obj).id;
    }



}
