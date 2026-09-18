#include "Lexer.h"
#include <cctype>

using namespace std;

Lexer::Lexer() : source(""), cursor(0), currentLine(1), currentColumn(1) {
    initKeywords();
}

void Lexer::initKeywords() {
    keywords["int"]     = TOKEN_INT;
    keywords["float"]   = TOKEN_FLOAT;
    keywords["char"]    = TOKEN_CHAR;
    keywords["boolean"] = TOKEN_BOOLEAN;
    keywords["void"]    = TOKEN_VOID;
    keywords["if"]      = TOKEN_IF;
    keywords["else"]    = TOKEN_ELSE;
    keywords["for"]     = TOKEN_FOR;
    keywords["while"]   = TOKEN_WHILE;
    keywords["scanf"]   = TOKEN_SCANF;
    keywords["println"] = TOKEN_PRINTLN;
    keywords["main"]    = TOKEN_MAIN;
    keywords["return"]  = TOKEN_RETURN;
}

bool Lexer::loadFile(const string& filepath) {
    ifstream file(filepath);
    if (!file.is_open()) {
        cerr << "Error al abrir el archivo: " << filepath << endl;
        return false;
    }
    string content((istreambuf_iterator<char>(file)),
                    istreambuf_iterator<char>());
    source = content;
    cursor = 0;
    currentLine = 1;
    currentColumn = 1;
    file.close();
    return true;
}

void Lexer::setSource(const string& input) {
    source = input;
    cursor = 0;
    currentLine = 1;
    currentColumn = 1;
}

char Lexer::peek() const {
    if (cursor >= source.length()) return '\0';
    return source[cursor];
}

char Lexer::advance() {
    if (cursor >= source.length()) return '\0';
    char c = source[cursor++];
    currentColumn++;
    return c;
}

const SymbolTable& Lexer::getSymbolTable() const {
    return symbols;
}

Token Lexer::getNextToken() {
    while (peek() != '\0') {
        char c = peek();

        // Saltos de linea y espacios
        if (c == '\n') {
            currentLine++;
            currentColumn = 1;
            cursor++;
            continue;
        }
        if (c == ' ' || c == '\t' || c == '\r') {
            advance();
            continue;
        }

        int startColumn = currentColumn;

        // NUM_INT (D+) y NUM_DEC (D+.D+)
        if (isdigit(static_cast<unsigned char>(c))) {
            size_t start = cursor;
            while (isdigit(static_cast<unsigned char>(peek()))) advance();

            bool isDecimal = false;
            if (peek() == '.' && (cursor + 1 < source.length())
                && isdigit(static_cast<unsigned char>(source[cursor + 1]))) {
                isDecimal = true;
                advance();
                while (isdigit(static_cast<unsigned char>(peek()))) advance();
            }

            string lexeme = source.substr(start, cursor - start);
            return {isDecimal ? TOKEN_NUM_DEC : TOKEN_NUM_INT,
                    lexeme, currentLine, startColumn, -1};
        }

        // ID (L(L|D)*) y palabras reservadas -- L = [a-zA-Z_]
        if (isalpha(static_cast<unsigned char>(c)) || c == '_') {
            size_t start = cursor;
            while (isalnum(static_cast<unsigned char>(peek())) || peek() == '_') {
                advance();
            }
            string lexeme = source.substr(start, cursor - start);

            auto it = keywords.find(lexeme);
            if (it != keywords.end()) {
                return {it->second, lexeme, currentLine, startColumn, -1};
            }

            int pos = symbols.insert(lexeme);
            return {TOKEN_ID, lexeme, currentLine, startColumn, pos};
        }

        // TEXTO ".*"
        if (c == '"') {
            size_t start = cursor;
            advance(); // consumir "
            while (peek() != '"' && peek() != '\n' && peek() != '\0') {
                advance();
            }
            if (peek() == '"') {
                advance(); // consumir " de cierre
                string lexeme = source.substr(start, cursor - start);
                return {TOKEN_TEXTO, lexeme, currentLine, startColumn, -1};
            }
            // cadena sin cierre -> error lexico
            string lexeme = source.substr(start, cursor - start);
            return {TOKEN_DESCONOCIDO, lexeme, currentLine, startColumn, -1};
        }

        // Cualquier otro caracter: aun no procesado en esta fase
        string lexeme(1, advance());
        return {TOKEN_DESCONOCIDO, lexeme, currentLine, startColumn, -1};
    }

    return {TOKEN_EOF, "", currentLine, currentColumn, -1};
}
