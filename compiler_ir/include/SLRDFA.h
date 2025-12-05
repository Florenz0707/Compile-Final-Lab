/*!
 * @file SLRDFA.h
 * @brief DFA implementation for SLR lexer (adapted for compiler_ir)
 * @version 1.0.0
 * @date 2025
 */

#ifndef SYSYC_SLRDFA_H
#define SYSYC_SLRDFA_H

#include <memory>
#include <vector>
#include <map>
#include "Lexer.h"

class SLRDFAState {
public:
    int id;
    bool isAccept;
    TokenType acceptType;
    int tokenNumber;
    std::string tokenValue;
    int priority;
    std::map<char, std::shared_ptr<SLRDFAState>> transitions;
    
    SLRDFAState(int stateId) : id(stateId), isAccept(false), acceptType(TokenType::ERROR), tokenNumber(0), priority(0) {}

    void addTransition(char c, std::shared_ptr<SLRDFAState> nextState) {
        transitions[c] = nextState;
    }
};

class SLRDFA {
public:
    std::shared_ptr<SLRDFAState> start;
    std::vector<std::shared_ptr<SLRDFAState>> states;
    std::vector<std::shared_ptr<SLRDFAState>> acceptStates;
    int stateCounter;

    SLRDFA() : stateCounter(0) {}

    std::shared_ptr<SLRDFAState> createState() {
        auto state = std::make_shared<SLRDFAState>(stateCounter++);
        states.push_back(state);
        return state;
    }
    
    std::shared_ptr<SLRDFAState> createAcceptState(TokenType type, int number, 
                                         const std::string& value, int priority) {
        auto state = createState();
        state->isAccept = true;
        state->acceptType = type;
        state->tokenNumber = number;
        state->tokenValue = value;
        state->priority = priority;
        acceptStates.push_back(state);
        return state;
    }
};

#endif // SYSYC_SLRDFA_H
