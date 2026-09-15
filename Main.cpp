#include "Lexer.cpp"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    Lexer lexer;

    if (argc >= 2) {
        if (!lexer.loadFile(argv[1])) {
            return 1;
        }
    } else {
        std::string input((std::istreambuf_iterator<char>(std::cin)),
                           std::istreambuf_iterator<char>());
        lexer.setSource(input);
    }

    std::cout << "TIPO\t\tLEXEMA\t\tLINEA\tCOLUMNA" << std::endl;
    std::cout << "----------------------------------------------------" << std::endl;

    Token token = lexer.getNextToken();
    while (token.type != TOKEN_EOF) {
        std::cout << token.getTypeString() << "\t\t"
                  << token.lexeme << "\t\t"
                  << token.line << "\t"
                  << token.column << std::endl;
        token = lexer.getNextToken();
    }

    std::cout << "EOF\t\t\t\t" << token.line << "\t" << token.column << std::endl;
    return 0;
}
