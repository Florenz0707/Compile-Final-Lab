/*!
 * @file SLRSubsetConstruction.h
 * @brief Subset Construction algorithm for NFA to DFA conversion (adapted for compiler_ir)
 * @version 1.0.0
 * @date 2025
 */

#ifndef SYSYC_SLRSUBSETCONSTRUCTION_H
#define SYSYC_SLRSUBSETCONSTRUCTION_H

#include <memory>
#include <vector>
#include <map>
#include <set>
#include <queue>
#include <string>
#include "SLRNFA.h"
#include "SLRDFA.h"

class SubsetConstruction {
public:
    std::shared_ptr<SLRDFA> convert(std::shared_ptr<NFA> nfa) { 
        auto dfa = std::make_shared<SLRDFA>();
        std::map<std::set<std::shared_ptr<NFAState>>, std::shared_ptr<SLRDFAState>> stateMap;
        
        std::set<std::shared_ptr<NFAState>> startSet = NFATools::epsilonClosure({nfa->start});
        dfa->start = getOrCreateDFAState(startSet, dfa, stateMap);

        std::queue<std::set<std::shared_ptr<NFAState>>> worklist;
        worklist.push(startSet);

        std::vector<char> alphabet = buildAlphabet(nfa);

        while (!worklist.empty()) {
            auto currentSet = worklist.front();
            worklist.pop();
            auto currentDFAState = stateMap[currentSet];
            
            for (char c : alphabet) {
                std::set<std::shared_ptr<NFAState>> nextSet = NFATools::move(currentSet, c);

                if (!nextSet.empty()) {
                    std::set<std::shared_ptr<NFAState>> closureSet = NFATools::epsilonClosure(nextSet);

                    if (stateMap.find(closureSet) == stateMap.end()) {
                        auto nextDFAState = getOrCreateDFAState(closureSet, dfa, stateMap);
                        currentDFAState->addTransition(c, nextDFAState);
                        worklist.push(closureSet);
                    } else {
                        auto nextDFAState = stateMap[closureSet];
                        currentDFAState->addTransition(c, nextDFAState);
                    }
                }
            }
        }
        return dfa;
    }
    
    std::vector<char> buildAlphabet(std::shared_ptr<NFA> nfa) {
        std::set<char> charset;
        for (auto &state : nfa->states) {
            for (auto &kv : state->transitions) {
                char c = kv.first;
                if (c != '\0') charset.insert(c);
            }
        }
        return std::vector<char>(charset.begin(), charset.end());
    }

    std::shared_ptr<SLRDFAState> getOrCreateDFAState(const std::set<std::shared_ptr<NFAState>>& nfaStates, 
        std::shared_ptr<SLRDFA> dfa, 
        std::map<std::set<std::shared_ptr<NFAState>>, std::shared_ptr<SLRDFAState>>& stateMap) {
        
        if (stateMap.find(nfaStates) != stateMap.end()) {
            return stateMap[nfaStates];
        }

        auto dfaState = dfa->createState(); 
        stateMap[nfaStates] = dfaState; 
    
        setAcceptInfoFromNFAStates(nfaStates, dfaState);
        
        return dfaState;
    }

    void setAcceptInfoFromNFAStates(const std::set<std::shared_ptr<NFAState>>& nfaStates, 
        std::shared_ptr<SLRDFAState> dfaState) {
        
        std::shared_ptr<NFAState> bestAcceptState = nullptr;
        
        for (auto state : nfaStates) {
            if (state->isAccept) { 
                if (bestAcceptState == nullptr || state->priority > bestAcceptState->priority) { 
                    bestAcceptState = state;
                }
            }
        }
        
        if (bestAcceptState != nullptr) {
            dfaState->isAccept = true;
            dfaState->acceptType = bestAcceptState->acceptType;
            dfaState->tokenNumber = bestAcceptState->tokenNumber;
            dfaState->tokenValue = bestAcceptState->tokenValue;
            dfaState->priority = bestAcceptState->priority;
        }
    }
};

#endif // SYSYC_SLRSUBSETCONSTRUCTION_H
