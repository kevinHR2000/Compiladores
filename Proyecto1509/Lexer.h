#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <fstream>
#include <iostream>
#include "Token.h"

class Lexer {
private:
    std::string source;
    size_t cursor;
    int currentLine;
    int currentColumn;

    char peek() const;
    char advance();

public:
    Lexer();
    bool loadFile(const std::string& filepath);
    void setSource(const std::string& input);
    Token getNextToken();
};

#endif