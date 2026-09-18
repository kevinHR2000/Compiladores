#ifndef TOKEN_H
#define TOKEN_H

#include <string>

enum TokenType {
    // Números
    TOKEN_NUM_INT,
    TOKEN_NUM_DEC,

    // Identificadores y texto
    TOKEN_ID,
    TOKEN_TEXTO,

    // Palabras reservadas
    TOKEN_INT,
    TOKEN_FLOAT,
    TOKEN_CHAR,
    TOKEN_BOOLEAN,
    TOKEN_VOID,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_FOR,
    TOKEN_WHILE,
    TOKEN_SCANF,
    TOKEN_PRINTLN,
    TOKEN_MAIN,
    TOKEN_RETURN,

    // Auxiliares
    TOKEN_DESCONOCIDO,
    TOKEN_EOF
};

struct Token {
    TokenType type;
    std::string lexeme;
    int line;
    int column;
    int attribute = -1;

    std::string getTypeString() const {
        switch (type) {
            case TOKEN_NUM_INT:    return "NUM_INT";
            case TOKEN_NUM_DEC:    return "NUM_DEC";
            case TOKEN_ID:         return "ID";
            case TOKEN_TEXTO:      return "TEXTO";
            case TOKEN_INT:        return "INT";
            case TOKEN_FLOAT:      return "FLOAT";
            case TOKEN_CHAR:       return "CHAR";
            case TOKEN_BOOLEAN:    return "BOOLEAN";
            case TOKEN_VOID:       return "VOID";
            case TOKEN_IF:         return "IF";
            case TOKEN_ELSE:       return "ELSE";
            case TOKEN_FOR:        return "FOR";
            case TOKEN_WHILE:      return "WHILE";
            case TOKEN_SCANF:      return "SCANF";
            case TOKEN_PRINTLN:    return "PRINTLN";
            case TOKEN_MAIN:       return "MAIN";
            case TOKEN_RETURN:     return "RETURN";
            case TOKEN_DESCONOCIDO:return "DESCONOCIDO";
            case TOKEN_EOF:        return "EOF";
            default:               return "UNKNOWN";
        }
    }
};

#endif
