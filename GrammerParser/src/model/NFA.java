package model;

public class NFA {
    public State startState;
    public State endState;

    public NFA(State start, State end) {
        this.startState = start;
        this.endState = end;
        this.endState.isAccept = true;
    }
}
