#include "Lexer.h"
#include <cctype>

Lexer::Lexer() : source(""), cursor(0), currentLine(1), currentColumn(1) {}

bool Lexer::loadFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Error al abrir el archivo: " << filepath << std::endl;
        return false;
    }
    std::string content((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());
    source = content;
    cursor = 0;
    currentLine = 1;
    currentColumn = 1;
    file.close();
    return true;
}

void Lexer::setSource(const std::string& input) {
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

Token Lexer::getNextToken() {
    while (peek() != '\0') {
        char c = peek();

        // Manejo de saltos de línea y espacios en blanco
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

        // Reconocimiento de NUM_INT (D+) y NUM_DEC (D+.D+) según ER
        if (std::isdigit(c)) {
            size_t start = cursor;

            while (std::isdigit(peek())) {
                advance();
            }

            bool isDecimal = false;
            // Verificar si hay un punto seguido de al menos un dígito
            if (peek() == '.' && (cursor + 1 < source.length()) && std::isdigit(source[cursor + 1])) {
                isDecimal = true;
                advance(); // Consumir '.'
                while (std::isdigit(peek())) {
                    advance();
                }
            }

            std::string lexeme = source.substr(start, cursor - start);
            return {isDecimal ? TOKEN_NUM_DEC : TOKEN_NUM_INT, lexeme, currentLine, startColumn};
        }

        // Para la fase del 15/09, cualquier otro carácter se avanza y marca como desconocido
        std::string lexeme(1, advance());
        return {TOKEN_DESCONOCIDO, lexeme, currentLine, startColumn};
    }

    return {TOKEN_EOF, "", currentLine, currentColumn};
}