package generator;

import model.*;
import java.util.*;

public class MinDfaConstructor {

    private List<State> allDfaStates = new ArrayList<>();
    private Set<Character> alphabet = new HashSet<>();

    public State minimize(State dfaStart) {
        collectDfaInfo(dfaStart);


        Map<String, Set<State>> initialGroups = new HashMap<>();
        for (State s : allDfaStates) {
            String groupKey = s.isAccept ? s.tokenType + s.tokenValue : "NON_ACCEPT";
            initialGroups.computeIfAbsent(groupKey, k -> new HashSet<>()).add(s);
        }

        List<Set<State>> partition = new ArrayList<>(initialGroups.values());
        
        boolean changed = true;
        while (changed) {
            changed = false;
            List<Set<State>> newPartition = new ArrayList<>();

            for (Set<State> group : partition) {
                if (group.size() <= 1) {
                    newPartition.add(group);
                    continue;
                }

                List<Set<State>> splitGroups = splitGroup(group, partition);
                newPartition.addAll(splitGroups);

                if (splitGroups.size() > 1) {
                    changed = true;
                }
            }
            partition = newPartition;
        }

        return reconstructMinimizedDfa(dfaStart, partition);
    }

    private List<Set<State>> splitGroup(Set<State> group, List<Set<State>> currentPartition) {
        Map<List<Integer>, Set<State>> subgroups = new HashMap<>();

        for (State s : group) {
            List<Integer> transitionSignature = new ArrayList<>();
            for (char c : alphabet) {
                State next = s.getDfaTransition(c);
                transitionSignature.add(findGroupIndex(next, currentPartition));
            }
            subgroups.computeIfAbsent(transitionSignature, k -> new HashSet<>()).add(s);
        }
        return new ArrayList<>(subgroups.values());
    }

    private int findGroupIndex(State s, List<Set<State>> partition) {
        if (s == null) return -1;
        for (int i = 0; i < partition.size(); i++) {
            if (partition.get(i).contains(s)) {
                return i;
            }
        }
        return -1;
    }

    private void collectDfaInfo(State start) {
        Set<State> visited = new HashSet<>();
        Queue<State> queue = new LinkedList<>();
        queue.add(start);
        visited.add(start);

        while (!queue.isEmpty()) {
            State s = queue.poll();
            allDfaStates.add(s);
            for (Map.Entry<Character, State> entry : s.getAllDfaTransitions().entrySet()) {
                alphabet.add(entry.getKey());
                if (visited.add(entry.getValue())) {
                    queue.add(entry.getValue());
                }
            }
        }
    }

    private State reconstructMinimizedDfa(State oldDfaStart, List<Set<State>> partition) {
        Map<Set<State>, State> newStatesMap = new HashMap<>();
        Map<State, State> oldToNewState = new HashMap<>();

        for (Set<State> group : partition) {
            State newState = new State();
            State representative = group.iterator().next();
            newState.isAccept = representative.isAccept;
            newState.tokenType = representative.tokenType;
            newState.tokenValue = representative.tokenValue;

            newStatesMap.put(group, newState);
            for (State oldState : group) {
                oldToNewState.put(oldState, newState);
            }
        }

        for (Set<State> group : partition) {
            State newState = newStatesMap.get(group);
            State representative = group.iterator().next();

            for (char c : alphabet) {
                State oldNextState = representative.getDfaTransition(c);
                if (oldNextState != null) {
                    State newNextState = oldToNewState.get(oldNextState);
                    newState.setDfaTransition(c, newNextState);
                }
            }
        }

        return oldToNewState.get(oldDfaStart);
    }
}
