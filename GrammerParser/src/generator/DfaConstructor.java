package generator;

import model.*;
import java.util.*;

public class DfaConstructor {
    private final Set<State> allNfaStates = new HashSet<>();
    private final Set<Character> alphabet = new HashSet<>();

    private Set<State> eClosure(Set<State> states) {
        Set<State> closure = new HashSet<>(states);
        Stack<State> stack = new Stack<>();
        stack.addAll(states);

        while (!stack.isEmpty()) {
            State s = stack.pop();
            for (State next : s.getNfaTransitions(null)) {
                if (closure.add(next)) {
                    stack.push(next);
                }
            }
        }
        return closure;
    }

    private Set<State> move(Set<State> states, char c) {
        Set<State> moveSet = new HashSet<>();
        for (State s : states) {
            moveSet.addAll(s.getNfaTransitions(c));
        }
        return moveSet;
    }

    private void collectNfaInfo(State start) {
        Set<State> visited = new HashSet<>();
        Queue<State> queue = new LinkedList<>();
        queue.add(start);
        visited.add(start);

        while (!queue.isEmpty()) {
            State s = queue.poll();
            allNfaStates.add(s);

            for (Map.Entry<Character, Set<State>> entry : s.getAllNfaTransitions().entrySet()) {
                Character symbol = entry.getKey();
                if (symbol != null) {
                    alphabet.add(symbol);
                }
                for (State next : entry.getValue()) {
                    if (visited.add(next)) {
                        queue.add(next);
                    }
                }
            }
        }
    }

    private void updateDfaAcceptState(State dfaState, Set<State> nfaSet) {
        for (State nfaState : nfaSet) {
            if (nfaState.isAccept) {
                if (!dfaState.isAccept) {
                    dfaState.isAccept = true;
                    dfaState.tokenType = nfaState.tokenType;
                    dfaState.tokenValue = nfaState.tokenValue;
                }
            }
        }
    }

    public State NfaToDfa(State nfaStart) {
        collectNfaInfo(nfaStart);

        Map<Set<State>, State> dfaStateMap = new HashMap<>();
        Queue<Set<State>> workList = new LinkedList<>();

        Set<State> nfaStartSet = eClosure(Collections.singleton(nfaStart));
        State dfaStart = new State();
        dfaStateMap.put(nfaStartSet, dfaStart);
        workList.add(nfaStartSet);

        while (!workList.isEmpty()) {
            Set<State> currentNfaSet = workList.poll();
            State currentDfaState = dfaStateMap.get(currentNfaSet);

            updateDfaAcceptState(currentDfaState, currentNfaSet);

            for (char c : alphabet) {
                Set<State> moveSet = move(currentNfaSet, c);
                if (moveSet.isEmpty()) continue;

                Set<State> nextNfaSet = eClosure(moveSet);
                if (nextNfaSet.isEmpty()) continue;

                State nextDfaState = dfaStateMap.get(nextNfaSet);
                if (nextDfaState == null) {
                    nextDfaState = new State();
                    dfaStateMap.put(nextNfaSet, nextDfaState);
                    workList.add(nextNfaSet);
                }

                currentDfaState.setDfaTransition(c, nextDfaState);
            }
        }
        return dfaStart;
    }
}
