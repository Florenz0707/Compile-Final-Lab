package generator;

import model.NFA;
import model.State;

import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import java.util.Stack;

public class NfaConstructor {

    private String regex;
    private int index;

    public NFA construct(String regex, String tokenType, int tokenValue) {
        this.regex = regex;
        this.index = 0;

        NFA nfa = parseRegex();

        nfa.endState.tokenType = tokenType;
        nfa.endState.tokenValue = tokenValue;

        return nfa;
    }

    private NFA parseRegex() {
        NFA left = parseTerm();
        while (index < regex.length() && regex.charAt(index) == '|') {
            index++;
            NFA right = parseTerm();
            left = createUnionNFA(left, right);
        }
        return left;
    }

    private NFA parseTerm() {
        NFA left = parseFactor();

        while (index < regex.length() && regex.charAt(index) != ')' && regex.charAt(index) != '|') {
            NFA right = parseFactor();
            left = createConcatNFA(left, right);
        }

        return left;
    }

    private NFA parseFactor() {
        NFA atomNfa = parseAtom();

        if (index < regex.length()) {
            char op = regex.charAt(index);

            if (op == '*') {
                index++;
                return createClosureNFA(atomNfa);

            } else if (op == '+') {
                index++;

                State start = new State();
                State end = new State();

                atomNfa.endState.isAccept = false;

                start.addNfaTransition(null, atomNfa.startState);

                atomNfa.endState.addNfaTransition(null, atomNfa.startState);

                atomNfa.endState.addNfaTransition(null, end);

                return new NFA(start, end);
            }
        }

        return atomNfa;
    }

    private NFA parseAtom() {
        if (regex.charAt(index) == '(') {
            index++;
            NFA nfa = parseRegex();

            if (index >= regex.length() || regex.charAt(index) != ')') {
                throw new RuntimeException("Regex Error: Missing ')' for regex: " + this.regex);
            }
            index++;
            return nfa;

        } else if (regex.charAt(index) == '\\') {
            index++;

            if (index >= regex.length()) {
                throw new RuntimeException("Regex Error: dangling escape character for regex: " + this.regex);
            }

            char c = regex.charAt(index++);

            switch (c) {
                case 't':
                    return createBasicNFA('\t');
                case 'n':
                    return createBasicNFA('\n');
                case 'r':
                    return createBasicNFA('\r');
                default:
                    return createBasicNFA(c);
            }

        } else {
            char c = regex.charAt(index++);
            return createBasicNFA(c);
        }
    }

    private NFA createBasicNFA(char c) {
        State start = new State();
        State end = new State();
        start.addNfaTransition(c, end);
        return new NFA(start, end);
    }

    private NFA createUnionNFA(NFA nfa1, NFA nfa2) {
        State start = new State();
        start.addNfaTransition(null, nfa1.startState);
        start.addNfaTransition(null, nfa2.startState);

        State end = new State();
        nfa1.endState.isAccept = false;
        nfa2.endState.isAccept = false;
        nfa1.endState.addNfaTransition(null, end);
        nfa2.endState.addNfaTransition(null, end);

        return new NFA(start, end);
    }

    private NFA createConcatNFA(NFA nfa1, NFA nfa2) {
        nfa1.endState.isAccept = false;
        nfa1.endState.addNfaTransition(null, nfa2.startState);

        return new NFA(nfa1.startState, nfa2.endState);
    }

    private NFA createClosureNFA(NFA nfa) {
        State start = new State();
        State end = new State();

        nfa.endState.isAccept = false;

        nfa.endState.addNfaTransition(null, nfa.startState);

        start.addNfaTransition(null, end);

        start.addNfaTransition(null, nfa.startState);
        nfa.endState.addNfaTransition(null, end);

        return new NFA(start, end);
    }
}