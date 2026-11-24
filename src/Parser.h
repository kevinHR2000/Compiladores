#ifndef PARSER_H
#define PARSER_H

#include "Lexer.h"
#include "Token.h"
#include "AST.h"
#include <string>
#include <vector>

// =====================================================================
// Analizador Sintactico para LP (Entregable 2 - 70%)
// ---------------------------------------------------------------------
// Tecnica: descenso recursivo con 2 tokens de lookahead (necesario para
// distinguir PRINCIPAL ('void main') de DECLARACION_FUNCION ('void id')).
//
// Cada parseXxx() implementa UN no terminal de la gramatica LP y
// devuelve el nodo del AST correspondiente. El root final queda
// accesible mediante getAST() tras llamar a parse().
//
// Alcance completo (seccion 5.1 del PDF):
//   - Expresiones con jerarquia completa de precedencia
//     (||, &&, comp, +-, */%, factor unario, parentesis).
//   - LECTURA (scanf), ESCRITURA (println).
//   - SELECCION (if/else), MIENTRAS (while).
//   - FUNCION (declaracion + llamada) con PARAMETROS y ARGUMENTOS.
//   - RETORNO (return EXPRESION ;).
//   - AST serializable a texto y a Graphviz (.dot).
//   - Recuperacion basica ante errores (panic-mode a ; o }).
// =====================================================================
class Parser {
private:
    Lexer& lexer;
    Token currentToken;   // Token "bajo el cursor".
    Token nextToken;      // Lookahead extra (necesario para 'void main').
    int errorCount;
    Node* root;           // Raiz del AST construido durante parse().

    // ----- Utilidades -----
    void advance();
    bool match(TokenType type);
    bool check(TokenType type) const;        // mira sin consumir
    void expect(TokenType type, const std::string& nombreEsperado);
    void error(const std::string& esperado);
    // Panic-mode: avanza hasta encontrar un token de sincronizacion
    // (por defecto ';' o '}' para reanudar en el siguiente comando/bloque).
    void synchronize();
    // Devuelve true si currentToken puede iniciar un COMANDO.
    bool isComandoStart() const;
    // Devuelve true si currentToken es un TIPO_BASE (int/float/char/boolean).
    bool isTipoBase() const;
    // Devuelve true si currentToken puede iniciar un TIPO_RETORNO
    // (TIPO_BASE o 'void').
    bool isTipoRetornoStart() const;

    // ----- No terminales -----
    Node* parsePrograma();
    Node* parseDeclaracionFuncion();     // consume TIPO_RETORNO id (...) BLOQUE
    Node* parsePrincipal();              // main () BLOQUE  (acepta 'void main' opcional)
    Node* parseTipoRetorno();            // TIPO | void
    Node* parseTipo();                   // TIPO_BASE DIMENSION
    std::string parseTipoBaseLexema();   // devuelve el lexema del tipo
    void   parseDimension(Node* tipoNode);
    void   parseParametros(Node* parent);
    Node*  parseBloque();
    void   parseListaVariables(Node* bloque);
    void   parseListaId(std::vector<std::string>& ids);
    void   parseComandos(Node* bloque);
    Node*  parseComando();
    Node*  parseLectura();               // scanf
    Node*  parseTerminoLectura();        // id DIMENSION2
    void   parseDimension2(Node* parent);
    Node*  parseEscritura();             // println
    Node*  parseTerminoEscritura();
    Node*  parseAsignacion();
    Node*  parseComplemento();           // EXPRESION | FUNCION
    Node*  parseSeleccion();             // if ... else ...
    Node*  parseMientras();              // while
    Node*  parseRetorno();               // return EXPRESION ;
    Node*  parseFuncionCall();           // func id ( ARGUMENTOS )
    void   parseArgumentos(Node* parent);

    // Expresiones con precedencia (de menor a mayor):
    Node* parseExpresion();              // === EXPRESION_O
    Node* parseExpresionO();             // ||
    Node* parseExpresionY();             // &&
    Node* parseExpresionRelacional();    // comp  (no asociativo)
    Node* parseExpresionAditiva();       // + -
    Node* parseExpresionMultiplicativa();// * / %
    Node* parseFactor();                 // SIGNO TERMINO | texto | ! FACTOR | ( EXPRESION )
    Node* parseTermino();                // id DIMENSION2 | CONSTANTE

public:
    explicit Parser(Lexer& lex);
    ~Parser();

    // Ejecuta el analisis. Devuelve true si no se encontraron errores.
    bool parse();

    // Acceso al AST construido (dueno sigue siendo el parser; se libera en ~Parser).
    Node* getAST() const { return root; }
};

#endif
