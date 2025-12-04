/*!
 * @file SLRNFA.h
 * @brief NFA implementation for SLR lexer (adapted for compiler_ir)
 * @version 1.0.0
 * @date 2024
 */

#ifndef SYSYC_SLRNFA_H
#define SYSYC_SLRNFA_H

#include <memory>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <cctype>
#include "Lexer.h"

class NFAState {
public:
    int id;
    bool isAccept;
    TokenType acceptType;
    int tokenNumber;
    std::string tokenValue;
    int priority;
    std::map<char, std::vector<std::shared_ptr<NFAState>>> transitions;
    
    NFAState(int stateId) : id(stateId), isAccept(false), acceptType(TokenType::ERROR), tokenNumber(0), priority(0) {}

    void addTransition(char c, std::shared_ptr<NFAState> nextState) {
        transitions[c].push_back(nextState);
    }
    
    void addEpsilonTransition(std::shared_ptr<NFAState> nextState) {
        transitions['\0'].push_back(nextState);
    }
};

class NFA {
public:
    std::shared_ptr<NFAState> start;
    std::vector<std::shared_ptr<NFAState>> states;
    std::vector<std::shared_ptr<NFAState>> acceptStates;
    int stateCounter;

    NFA() : stateCounter(0) {}
    
    std::shared_ptr<NFAState> createNFAState() {
        auto state = std::make_shared<NFAState>(stateCounter++);
        states.push_back(state);
        return state;
    }

    std::shared_ptr<NFAState> createAcceptState(TokenType type, int num, const std::string& value, int prio) {
        auto state = createNFAState();
        state->isAccept = true;
        state->acceptType = type;
        state->tokenNumber = num;
        state->tokenValue = value;
        state->priority = prio;
        acceptStates.push_back(state);
        return state;
    }

    std::shared_ptr<NFA> IntegerNFA() {
        auto nfa = std::make_shared<NFA>();
        nfa->start = nfa->createNFAState();
        auto acceptState = nfa->createAcceptState(TokenType::INT, 0, "", 10); 
        
        for (char c = '0'; c <= '9'; c++) {
            nfa->start->addTransition(c, acceptState);
            acceptState->addTransition(c, acceptState);
        }
        return nfa;
    }

    std::shared_ptr<NFA> FloatNFA() {
        auto nfa = std::make_shared<NFA>();
        nfa->start = nfa->createNFAState();
        auto intPart = nfa->createNFAState();
        auto dotState = nfa->createNFAState();
        auto fracPart = nfa->createAcceptState(TokenType::FLOAT, 0, "", 15);
        
        for (char c = '0'; c <= '9'; c++) {
            nfa->start->addTransition(c, intPart);
            intPart->addTransition(c, intPart);
        }
        
        intPart->addTransition('.', dotState);
        
        for (char c = '0'; c <= '9'; c++) {
            dotState->addTransition(c, fracPart);
            fracPart->addTransition(c, fracPart);
        }
        
        return nfa;
    }

    std::shared_ptr<NFA> SENFA() {
        auto nfa = std::make_shared<NFA>();
        nfa->start = nfa->createNFAState();
        
        std::vector<std::pair<char, TokenType>> allSE = {
            {'(', TokenType::SE_LPAREN}, 
            {')', TokenType::SE_RPAREN}, 
            {'{', TokenType::SE_LBRACE}, 
            {'}', TokenType::SE_RBRACE}, 
            {';', TokenType::SE_SEMI}, 
            {',', TokenType::SE_COMMA}
        };
        
        for (auto& se_pair : allSE) {
            char se_char = se_pair.first;
            TokenType type = se_pair.second;
            auto end = nfa->createAcceptState(type, static_cast<int>(type), std::string(1, se_char), 5);
            nfa->start->addTransition(se_char, end);
        }
        return nfa;  
    }

    std::shared_ptr<NFA> OPNFA() {
        auto nfa = std::make_shared<NFA>();
        nfa->start = nfa->createNFAState();
        
        std::vector<std::pair<std::string, TokenType>> singleOps = {
            {"+", TokenType::OP_PLUS}, 
            {"-", TokenType::OP_MINUS}, 
            {"*", TokenType::OP_MUL}, 
            {"/", TokenType::OP_DIV}, 
            {"%", TokenType::OP_MOD},
            {"!", TokenType::OP_NOT}
        };
        
        std::vector<std::pair<std::string, TokenType>> doubleOps = {
            {"==", TokenType::OP_EQ}, 
            {"<=", TokenType::OP_LE}, 
            {">=", TokenType::OP_GE}, 
            {"!=", TokenType::OP_NE}, 
            {"&&", TokenType::OP_AND}, 
            {"||", TokenType::OP_OR}
        };
        
        std::vector<std::pair<std::string, TokenType>> specialOps = {
            {"=", TokenType::OP_ASSIGN}, 
            {">", TokenType::OP_GT}, 
            {"<", TokenType::OP_LT}
        };
        
        for (auto& op : singleOps) {
            auto acceptState = nfa->createAcceptState(op.second, static_cast<int>(op.second), op.first, 10);
            nfa->start->addTransition(op.first[0], acceptState);
        }
        
        for (auto& op : specialOps) {
            auto acceptState = nfa->createAcceptState(op.second, static_cast<int>(op.second), op.first, 10);
            nfa->start->addTransition(op.first[0], acceptState);
        }

        for (auto& op : doubleOps) {
            std::string val = op.first;
            TokenType type = op.second;
            
            auto mid = nfa->createNFAState();
            auto end = nfa->createAcceptState(type, static_cast<int>(type), val, 15);
            
            nfa->start->addTransition(val[0], mid);
            mid->addTransition(val[1], end);
        }
        
        return nfa;
    }

    std::shared_ptr<NFA> IDNNFA() {
        auto nfa = std::make_shared<NFA>();
        nfa->start = nfa->createNFAState();
        auto acceptState = nfa->createAcceptState(TokenType::IDN, 0, "", 10);

        for (char c = 'a'; c <= 'z'; c++) {
            nfa->start->addTransition(c, acceptState);
            acceptState->addTransition(c, acceptState);
        }

        for (char c = 'A'; c <= 'Z'; c++) {
            nfa->start->addTransition(c, acceptState);
            acceptState->addTransition(c, acceptState);
        }

        nfa->start->addTransition('_', acceptState);     
        acceptState->addTransition('_', acceptState);
        
        for (char c = '0'; c <= '9'; c++) {
            acceptState->addTransition(c, acceptState);     
        }
        return nfa;
    }

    std::shared_ptr<NFA> buildKeywordNFA(const std::string& keyword, TokenType type) {
        auto nfa = std::make_shared<NFA>();
        auto current = nfa->start = nfa->createNFAState();
        
        for (size_t i = 0; i < keyword.length(); i++) {
            auto next = (i == keyword.length() - 1) 
                       ? nfa->createAcceptState(type, static_cast<int>(type), keyword, 20)
                       : nfa->createNFAState();
            
            char ch = keyword[i];
            if (std::isalpha(static_cast<unsigned char>(ch))) {
                char lower = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
                char upper = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
                current->addTransition(lower, next);
                current->addTransition(upper, next);
            } else {
                current->addTransition(ch, next);
            }
            current = next;
        }
        
        return nfa;
    }

    std::shared_ptr<NFA> KWNFA() {
        auto nfa = std::make_shared<NFA>();
        nfa->start = nfa->createNFAState();
        
        // Note: "main" is NOT a keyword, it's an identifier
        std::vector<std::pair<std::string, TokenType>> keywords = {
            {"int", TokenType::KW_INT}, 
            {"void", TokenType::KW_VOID}, 
            {"return", TokenType::KW_RETURN}, 
            {"const", TokenType::KW_CONST}, 
            {"float", TokenType::KW_FLOAT}, 
            {"if", TokenType::KW_IF}, 
            {"else", TokenType::KW_ELSE}
        };
        
        for (auto& kw : keywords) {
            auto keywordNFA = buildKeywordNFA(kw.first, kw.second);
            nfa->start->addEpsilonTransition(keywordNFA->start);
            
            for (auto state : keywordNFA->states) {
                nfa->states.push_back(state);
            }
            for (auto state : keywordNFA->acceptStates) {
                nfa->acceptStates.push_back(state);
            }
        }
        
        return nfa;
    }
    
    std::shared_ptr<NFA> combineNFA(std::vector<std::shared_ptr<NFA>> nfas) {
        auto combined = std::make_shared<NFA>();
        combined->start = combined->createNFAState();
        for (auto nfa : nfas) {
            combined->start->addEpsilonTransition(nfa->start);
            for (auto state : nfa->states) {
                combined->states.push_back(state);
            }
            for (auto state : nfa->acceptStates) {
                combined->acceptStates.push_back(state);
            }
        }
        return combined;
    }
};

// NFA tools for epsilon closure and move operations
class NFATools {
public:
    static std::set<std::shared_ptr<NFAState>> epsilonClosure(const std::set<std::shared_ptr<NFAState>>& states) {
        std::set<std::shared_ptr<NFAState>> closure = states;
        std::vector<std::shared_ptr<NFAState>> stack(states.begin(), states.end());
        
        while (!stack.empty()) {
            auto current = stack.back();
            stack.pop_back();
            
            if (current->transitions.count('\0')) { 
                for (auto nextState : current->transitions['\0']) {
                    if (closure.find(nextState) == closure.end()) {
                        closure.insert(nextState);
                        stack.push_back(nextState);
                    }
                }
            }
        }
        
        return closure;
    }

    static std::set<std::shared_ptr<NFAState>> move(const std::set<std::shared_ptr<NFAState>>& states, char c) {
        std::set<std::shared_ptr<NFAState>> result;
        
        for (auto state : states) {
            if (state->transitions.count(c)) {
                for (auto target : state->transitions[c]) {
                    result.insert(target);
                }
            }
        }
            
        return result;
    }
};

#endif // SYSYC_SLRNFA_H
