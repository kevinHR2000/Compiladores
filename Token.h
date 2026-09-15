#ifndef TOKEN_H
#define TOKEN_H

#include <string>

enum TokenType {
    TOKEN_NUM_INT,
    TOKEN_NUM_DEC,
    TOKEN_DESCONOCIDO, // Para caracteres que aún no se procesan en la Semana 1
    TOKEN_EOF
};

struct Token {
    TokenType type;
    std::string lexeme;
    int line;
    int column;

    std::string getTypeString() const {
        switch (type) {
            case TOKEN_NUM_INT: return "NUM_INT";
            case TOKEN_NUM_DEC: return "NUM_DEC";
            case TOKEN_DESCONOCIDO: return "DESCONOCIDO";
            case TOKEN_EOF: return "EOF";
            default: return "UNKNOWN";
        }
    }
};

#endif