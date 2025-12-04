/*!
 * @file SLRLexer.h
 * @brief SLR-based Lexer wrapper (adapted for compiler_ir)
 * @version 1.0.0
 * @date 2024
 */

#ifndef SYSYC_SLRLEXER_H
#define SYSYC_SLRLEXER_H

#include <string>
#include <vector>
#include <memory>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cctype>
#include "Lexer.h"
#include "SLRNFA.h"
#include "SLRDFA.h"
#include "SLRSubsetConstruction.h"
#include "SLRDFAMinimizer.h"

class SLRLexer {
private:
    std::shared_ptr<SLRDFA> dfa;
    int line;
    int column;
    
public:
    SLRLexer() : line(1), column(1) {
        buildDFA();
    }
    
    void buildDFA() {
        auto nfaBuilder = std::make_shared<NFA>();
        
        auto kwNFA = nfaBuilder->KWNFA();
        auto idnNFA = nfaBuilder->IDNNFA();
        auto intNFA = nfaBuilder->IntegerNFA();
        auto floatNFA = nfaBuilder->FloatNFA();
        auto opNFA = nfaBuilder->OPNFA();
        auto seNFA = nfaBuilder->SENFA();
        
        std::vector<std::shared_ptr<NFA>> nfas = {kwNFA, idnNFA, intNFA, floatNFA, opNFA, seNFA};
        auto combinedNFA = nfaBuilder->combineNFA(nfas);
        
        SubsetConstruction constructor;
        auto rawDfa = constructor.convert(combinedNFA);
        dfa = DFAMinimizer::minimize(rawDfa);
    }
    
    std::vector<Token> analyze(const std::string& sourceCode) {
        std::vector<Token> tokens;
        int pos = 0;
        int length = sourceCode.length();
        line = 1;
        column = 1;
        
        while (pos < length) {
            char currentChar = sourceCode[pos];
            
            // Skip whitespace
            if (std::isspace(currentChar)) {
                if (currentChar == '\n') {
                    line++;
                    column = 1;
                } else if (currentChar == '\t') {
                    column += 4;
                } else {
                    column++;
                }
                pos++;
                continue;
            }
            
            // Skip comments
            if (currentChar == '/' && pos + 1 < length) {
                if (sourceCode[pos + 1] == '/') {
                    // Line comment
                    while (pos < length && sourceCode[pos] != '\n') {
                        pos++;
                        column++;
                    }
                    continue;
                } else if (sourceCode[pos + 1] == '*') {
                    // Block comment
                    pos += 2;
                    column += 2;
                    while (pos + 1 < length) {
                        if (sourceCode[pos] == '*' && sourceCode[pos + 1] == '/') {
                            pos += 2;
                            column += 2;
                            break;
                        }
                        if (sourceCode[pos] == '\n') {
                            line++;
                            column = 1;
                        } else {
                            column++;
                        }
                        pos++;
                    }
                    continue;
                }
            }
            
            auto currentState = dfa->start;
            int startPos = pos;
            int startLine = line;
            int startColumn = column;
            std::shared_ptr<SLRDFAState> lastAcceptState = nullptr;
            int lastAcceptPos = -1;
            int currentPos = pos;
            
            // Longest match
            while (currentPos < length) {
                char c = sourceCode[currentPos];
                auto it = currentState->transitions.find(c);
                if (it == currentState->transitions.end()) {
                    break;
                }
                currentState = it->second;
                currentPos++;
                if (currentState->isAccept) {
                    lastAcceptState = currentState;
                    lastAcceptPos = currentPos;
                }
            }
            
            if (lastAcceptState != nullptr && lastAcceptPos > startPos) {
                std::string tokenValue = sourceCode.substr(startPos, lastAcceptPos - startPos);
                Token token(lastAcceptState->acceptType, tokenValue, startLine, startColumn);
                tokens.push_back(token);
                
                // Update line and column
                for (int i = startPos; i < lastAcceptPos; ++i) {
                    if (sourceCode[i] == '\n') {
                        line++;
                        column = 1;
                    } else if (sourceCode[i] == '\t') {
                        column += 4;
                    } else {
                        column++;
                    }
                }
                pos = lastAcceptPos;
            } else {
                // Unrecognized character
                std::string errorChar(1, sourceCode[startPos]);
                Token token(TokenType::ERROR, errorChar, startLine, startColumn);
                tokens.push_back(token);
                pos = startPos + 1;
                column++;
            }
        }
        
        // Add EOF token
        tokens.push_back(Token(TokenType::END_OF_FILE, "$", line, column));
        
        return tokens;
    }
};

#endif // SYSYC_SLRLEXER_H
