/*!
 * @file SLRDFAMinimizer.h
 * @brief DFA Minimizer for optimizing DFA states (adapted for compiler_ir)
 * @version 1.0.0
 * @date 2025
 */

#ifndef SYSYC_SLRDFAMINIMIZER_H
#define SYSYC_SLRDFAMINIMIZER_H

#include <memory>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <algorithm>
#include "SLRDFA.h"

class DFAMinimizer {
public:
    static std::shared_ptr<SLRDFA> minimize(std::shared_ptr<SLRDFA> originalDFA) {
        auto minimizedDFA = std::make_shared<SLRDFA>();

        if (!originalDFA || originalDFA->states.empty()) return minimizedDFA;

        std::vector<char> alphabet = buildAlphabet(originalDFA);

        std::vector<std::vector<std::shared_ptr<SLRDFAState>>> partitions;
        partitions.reserve(originalDFA->states.size());
        std::map<std::shared_ptr<SLRDFAState>, int> stateToGroup;

        std::map<std::string, int> keyToGroup;
        auto getKey = [](std::shared_ptr<SLRDFAState> s){
            if (!s->isAccept) return std::string("N");
            return std::string("A_") + std::to_string(static_cast<int>(s->acceptType)) + "_" +
                   std::to_string(s->tokenNumber) + "_" + std::to_string(s->priority);
        };

        for (auto s : originalDFA->states) {
            std::string key = getKey(s);
            if (!keyToGroup.count(key)) {
                keyToGroup[key] = static_cast<int>(partitions.size());
                partitions.push_back({});
            }
            int gid = keyToGroup[key];
            partitions[gid].push_back(s);
            stateToGroup[s] = gid;
        }

        bool changed;
        do {
            changed = false;
            std::vector<std::vector<std::shared_ptr<SLRDFAState>>> newPartitions;
            newPartitions.reserve(partitions.size());

            for (auto &group : partitions) {
                if (group.size() <= 1) {
                    newPartitions.push_back(group);
                    continue;
                }

                std::map<std::string, std::vector<std::shared_ptr<SLRDFAState>>> buckets;
                buckets.clear();

                for (auto s : group) {
                    std::string sig;
                    sig.reserve(alphabet.size() * 3);
                    for (char c : alphabet) {
                        auto it = s->transitions.find(c);
                        int tgt = -1;
                        if (it != s->transitions.end()) {
                            auto t = it->second;
                            auto found = stateToGroup.find(t);
                            tgt = (found == stateToGroup.end()) ? -1 : found->second;
                        }
                        sig.push_back(c);
                        sig.push_back('#');
                        sig += std::to_string(tgt);
                        sig.push_back(';');
                    }
                    buckets[sig].push_back(s);
                }

                if (buckets.size() == 1) {
                    newPartitions.push_back(group);
                } else {
                    changed = true;
                    for (auto &kv : buckets) newPartitions.push_back(kv.second);
                }
            }

            partitions = std::move(newPartitions);
            stateToGroup.clear();
            for (int gid = 0; gid < static_cast<int>(partitions.size()); ++gid) {
                for (auto s : partitions[gid]) stateToGroup[s] = gid;
            }
        } while (changed);

        std::vector<std::shared_ptr<SLRDFAState>> groupToNewState(partitions.size());
        for (int gid = 0; gid < static_cast<int>(partitions.size()); ++gid) {
            auto s0 = partitions[gid][0];
            std::shared_ptr<SLRDFAState> ns;
            if (s0->isAccept) {
                ns = minimizedDFA->createAcceptState(s0->acceptType, s0->tokenNumber, s0->tokenValue, s0->priority);
            } else {
                ns = minimizedDFA->createState();
            }
            groupToNewState[gid] = ns;
            if (s0 == originalDFA->start) minimizedDFA->start = ns;
        }
        if (!minimizedDFA->start) {
            int startGroup = stateToGroup[originalDFA->start];
            minimizedDFA->start = groupToNewState[startGroup];
        }

        for (int gid = 0; gid < static_cast<int>(partitions.size()); ++gid) {
            auto rep = partitions[gid][0];
            auto from = groupToNewState[gid];
            for (auto &kv : rep->transitions) {
                char c = kv.first;
                auto target = kv.second;
                int tgtGroup = stateToGroup[target];
                from->addTransition(c, groupToNewState[tgtGroup]);
            }
        }

        return minimizedDFA;
    }

private:
    static std::vector<char> buildAlphabet(std::shared_ptr<SLRDFA> dfa) {
        std::set<char> cs;
        for (auto s : dfa->states) {
            for (auto &kv : s->transitions) cs.insert(kv.first);
        }
        return std::vector<char>(cs.begin(), cs.end());
    }
};

#endif // SYSYC_SLRDFAMINIMIZER_H
