#include "Parser.h"
#include <iostream>

using namespace std;

// =====================================================================
// Parser.cpp - analizador sintactico (Entregable 2, cubre la gramatica
// completa de LP). Cada parseXxx() devuelve el subarbol del no terminal
// que acaba de reconocer. La raiz se guarda en 'root' y se libera en
// el destructor.
//
// Lookahead: 2 tokens (currentToken + nextToken). El segundo se usa
// unicamente para distinguir 'void main' de 'void id' al decidir entre
// PRINCIPAL y DECLARACION_FUNCION dentro de PROGRAMA.
// =====================================================================

// ---------- ciclo de vida ----------

Parser::Parser(Lexer& lex) : lexer(lex), errorCount(0), root(nullptr) {
    // Precargamos 2 tokens para habilitar el lookahead extra.
    currentToken = lexer.getNextToken();
    nextToken    = lexer.getNextToken();
}

Parser::~Parser() {
    delete root;
}

// ---------- utilidades ----------

void Parser::advance() {
    currentToken = nextToken;
    nextToken    = lexer.getNextToken();
}

bool Parser::match(TokenType type) {
    if (currentToken.type == type) {
        advance();
        return true;
    }
    return false;
}

bool Parser::check(TokenType type) const {
    return currentToken.type == type;
}

void Parser::expect(TokenType type, const string& nombreEsperado) {
    if (currentToken.type == type) {
        advance();
    } else {
        error(nombreEsperado);
    }
}

void Parser::error(const string& esperado) {
    cerr << "[Error sintactico] Linea " << currentToken.line
         << ", Col " << currentToken.column
         << ": se esperaba " << esperado
         << " pero se encontro '" << currentToken.lexeme
         << "' (" << currentToken.getTypeString() << ")" << endl;
    errorCount++;
}

// Panic-mode: avanza hasta ';' o '}' para intentar continuar en el
// siguiente comando/bloque y poder reportar mas errores.
void Parser::synchronize() {
    while (currentToken.type != TOKEN_EOF &&
           currentToken.type != TOKEN_PYCOMA &&
           currentToken.type != TOKEN_LLA_DER) {
        advance();
    }
    if (currentToken.type == TOKEN_PYCOMA) advance();
}

bool Parser::isTipoBase() const {
    TokenType t = currentToken.type;
    return t == TOKEN_INT || t == TOKEN_FLOAT ||
           t == TOKEN_CHAR || t == TOKEN_BOOLEAN;
}

bool Parser::isTipoRetornoStart() const {
    return isTipoBase() || currentToken.type == TOKEN_VOID;
}

bool Parser::isComandoStart() const {
    TokenType t = currentToken.type;
    return t == TOKEN_ID     || t == TOKEN_SCANF  || t == TOKEN_PRINTLN ||
           t == TOKEN_IF     || t == TOKEN_WHILE  || t == TOKEN_RETURN  ||
           t == TOKEN_FUNC;
}

// ---------- punto de entrada ----------

bool Parser::parse() {
    root = parsePrograma();

    if (currentToken.type != TOKEN_EOF) {
        error("fin de archivo");
    }

    if (errorCount == 0) {
        cout << "\n[Parser] Programa ACEPTADO sintacticamente." << endl;
        return true;
    } else {
        cout << "\n[Parser] Programa RECHAZADO: "
             << errorCount << " error(es) sintactico(s)." << endl;
        return false;
    }
}

// ---------- no terminales ----------

// PROGRAMA -> LISTA_FUNCIONES PRINCIPAL
// LISTA_FUNCIONES -> DECLARACION_FUNCION LISTA_FUNCIONES | epsilon
// Usamos 2 tokens de lookahead: si vemos 'void main' entonces es PRINCIPAL
// (no una funcion llamada 'main').
Node* Parser::parsePrograma() {
    Node* prog = new Node(N_PROGRAMA, "PROGRAMA");

    while (isTipoRetornoStart()) {
        if (currentToken.type == TOKEN_VOID && nextToken.type == TOKEN_MAIN) break;
        prog->addChild(parseDeclaracionFuncion());
    }

    prog->addChild(parsePrincipal());
    return prog;
}

// PRINCIPAL -> main ( ) BLOQUE
// Aceptamos 'void' opcional al inicio (como aparece en el ejemplo del PDF).
Node* Parser::parsePrincipal() {
    match(TOKEN_VOID);
    Node* n = new Node(N_PRINCIPAL, "PRINCIPAL");
    expect(TOKEN_MAIN,    "'main'");
    expect(TOKEN_PAR_IZQ, "'('");
    expect(TOKEN_PAR_DER, "')'");
    n->addChild(parseBloque());
    return n;
}

// DECLARACION_FUNCION -> TIPO_RETORNO id ( PARAMETROS ) BLOQUE
Node* Parser::parseDeclaracionFuncion() {
    Node* fn = new Node(N_FUNCION_DECL, "FUNCION_DECL");
    fn->addChild(parseTipoRetorno());

    if (check(TOKEN_ID)) {
        fn->value = currentToken.lexeme;
        advance();
    } else {
        error("identificador (nombre de la funcion)");
    }

    expect(TOKEN_PAR_IZQ, "'('");
    parseParametros(fn);
    expect(TOKEN_PAR_DER, "')'");

    fn->addChild(parseBloque());
    return fn;
}

// TIPO_RETORNO -> TIPO | void
Node* Parser::parseTipoRetorno() {
    if (currentToken.type == TOKEN_VOID) {
        Node* t = new Node(N_TIPO, "TIPO_RETORNO", "void");
        advance();
        return t;
    }
    Node* t = parseTipo();
    t->label = "TIPO_RETORNO";
    return t;
}

// TIPO -> TIPO_BASE DIMENSION
Node* Parser::parseTipo() {
    string base = parseTipoBaseLexema();
    Node* t = new Node(N_TIPO, "TIPO", base);
    parseDimension(t);
    return t;
}

// TIPO_BASE -> char | float | int | boolean
string Parser::parseTipoBaseLexema() {
    string lex = currentToken.lexeme;
    if (match(TOKEN_INT))     return lex;
    if (match(TOKEN_FLOAT))   return lex;
    if (match(TOKEN_CHAR))    return lex;
    if (match(TOKEN_BOOLEAN)) return lex;
    error("tipo basico (int, float, char, boolean)");
    return "<error>";
}

// DIMENSION -> [ num_int ] DIMENSION | epsilon
// Las dimensiones se adjuntan como hijos del nodo TIPO.
void Parser::parseDimension(Node* tipoNode) {
    while (match(TOKEN_COR_IZQ)) {
        string size = currentToken.lexeme;
        if (!match(TOKEN_NUM_INT)) {
            error("numero entero dentro de '[ ]'");
        } else {
            tipoNode->addChild(new Node(N_INDICE, "DIM", size));
        }
        expect(TOKEN_COR_DER, "']'");
    }
}

// PARAMETROS -> TIPO id LISTA_PARAMETROS | epsilon
// LISTA_PARAMETROS -> , TIPO id LISTA_PARAMETROS | epsilon
void Parser::parseParametros(Node* parent) {
    if (!isTipoBase()) return;  // epsilon

    Node* ps = new Node(N_PARAMETROS, "PARAMETROS");

    // primer parametro
    Node* p1 = new Node(N_PARAMETRO, "PARAM");
    p1->addChild(parseTipo());
    if (check(TOKEN_ID)) { p1->value = currentToken.lexeme; advance(); }
    else error("identificador en parametro");
    ps->addChild(p1);

    // resto de parametros
    while (match(TOKEN_COMA)) {
        Node* p = new Node(N_PARAMETRO, "PARAM");
        p->addChild(parseTipo());
        if (check(TOKEN_ID)) { p->value = currentToken.lexeme; advance(); }
        else error("identificador en parametro");
        ps->addChild(p);
    }

    parent->addChild(ps);
}

// BLOQUE -> { LISTA_VARIABLES COMANDOS }
Node* Parser::parseBloque() {
    Node* b = new Node(N_BLOQUE, "BLOQUE");
    expect(TOKEN_LLA_IZQ, "'{'");
    parseListaVariables(b);
    parseComandos(b);
    expect(TOKEN_LLA_DER, "'}'");
    return b;
}

// LISTA_VARIABLES -> TIPO id LISTA_ID ; LISTA_VARIABLES | epsilon
void Parser::parseListaVariables(Node* bloque) {
    Node* lv = new Node(N_LISTA_VARS, "LISTA_VARIABLES");

    while (isTipoBase()) {
        Node* decl = new Node(N_VAR_DECL, "DECLARACION");
        decl->addChild(parseTipo());

        vector<string> ids;
        if (check(TOKEN_ID)) {
            ids.push_back(currentToken.lexeme);
            advance();
        } else {
            error("identificador tras tipo");
        }
        parseListaId(ids);

        for (const string& name : ids) {
            decl->addChild(new Node(N_ID_REF, "id", name));
        }
        expect(TOKEN_PYCOMA, "';' al final de la declaracion");
        lv->addChild(decl);
    }

    if (!lv->children.empty()) bloque->addChild(lv);
    else delete lv;
}

// LISTA_ID -> , id LISTA_ID | epsilon
void Parser::parseListaId(vector<string>& ids) {
    while (match(TOKEN_COMA)) {
        if (check(TOKEN_ID)) {
            ids.push_back(currentToken.lexeme);
            advance();
        } else {
            error("identificador despues de ','");
        }
    }
}

// COMANDOS -> COMANDO COMANDOS | epsilon
void Parser::parseComandos(Node* bloque) {
    while (isComandoStart()) {
        Node* c = parseComando();
        if (c != nullptr) bloque->addChild(c);
    }
}

// COMANDO -> LECTURA | ESCRITURA | ASIGNACION | FUNCION | SELECCION | MIENTRAS | RETORNO
Node* Parser::parseComando() {
    switch (currentToken.type) {
        case TOKEN_SCANF:   return parseLectura();
        case TOKEN_PRINTLN: return parseEscritura();
        case TOKEN_IF:      return parseSeleccion();
        case TOKEN_WHILE:   return parseMientras();
        case TOKEN_RETURN:  return parseRetorno();
        case TOKEN_FUNC: {
            // FUNCION como COMANDO no exige ';' en la gramatica, pero por
            // comodidad para el usuario aceptamos un ';' opcional.
            Node* call = parseFuncionCall();
            match(TOKEN_PYCOMA);
            return call;
        }
        case TOKEN_ID:      return parseAsignacion();
        default:
            error("inicio de comando (id, scanf, println, if, while, return, func)");
            advance();  // evita loop infinito
            return nullptr;
    }
}

// LECTURA -> scanf ( TERMINO_LECTURA NUEVO_TERMINO_LECTURA ) ;
Node* Parser::parseLectura() {
    expect(TOKEN_SCANF, "'scanf'");
    Node* n = new Node(N_SCANF, "SCANF");
    expect(TOKEN_PAR_IZQ, "'('");

    Node* t = parseTerminoLectura();
    if (t) n->addChild(t);

    while (match(TOKEN_COMA)) {
        Node* ti = parseTerminoLectura();
        if (ti) n->addChild(ti);
    }

    expect(TOKEN_PAR_DER, "')'");
    expect(TOKEN_PYCOMA, "';' al final de scanf");
    return n;
}

// TERMINO_LECTURA -> id DIMENSION2
Node* Parser::parseTerminoLectura() {
    if (!check(TOKEN_ID)) {
        error("identificador en scanf");
        advance();
        return nullptr;
    }
    Node* idNode = new Node(N_ID_REF, "id", currentToken.lexeme);
    advance();
    parseDimension2(idNode);
    return idNode;
}

// DIMENSION2 -> [ EXPRESION_ADITIVA ] DIMENSION2 | epsilon
void Parser::parseDimension2(Node* parent) {
    while (match(TOKEN_COR_IZQ)) {
        Node* idx = new Node(N_INDICE, "INDICE");
        idx->addChild(parseExpresionAditiva());
        expect(TOKEN_COR_DER, "']'");
        parent->addChild(idx);
    }
}

// ESCRITURA -> println ( TERMINO_ESCRITURA NUEVO_TERMINO_ESCRITURA ) ;
Node* Parser::parseEscritura() {
    expect(TOKEN_PRINTLN, "'println'");
    Node* n = new Node(N_PRINTLN, "PRINTLN");
    expect(TOKEN_PAR_IZQ, "'('");

    Node* t = parseTerminoEscritura();
    if (t) n->addChild(t);

    while (match(TOKEN_COMA)) {
        Node* ti = parseTerminoEscritura();
        if (ti) n->addChild(ti);
    }

    expect(TOKEN_PAR_DER, "')'");
    expect(TOKEN_PYCOMA, "';' al final de println");
    return n;
}

// TERMINO_ESCRITURA -> id DIMENSION2 | CONSTANTE | texto
Node* Parser::parseTerminoEscritura() {
    if (currentToken.type == TOKEN_TEXTO) {
        Node* t = new Node(N_TEXTO, "texto", currentToken.lexeme);
        advance();
        return t;
    }
    if (currentToken.type == TOKEN_NUM_INT) {
        Node* n = new Node(N_NUM_INT, "num_int", currentToken.lexeme);
        advance();
        return n;
    }
    if (currentToken.type == TOKEN_NUM_DEC) {
        Node* n = new Node(N_NUM_DEC, "num_dec", currentToken.lexeme);
        advance();
        return n;
    }
    if (currentToken.type == TOKEN_ID) {
        Node* idNode = new Node(N_ID_REF, "id", currentToken.lexeme);
        advance();
        parseDimension2(idNode);
        return idNode;
    }
    error("texto, numero o identificador en println");
    advance();
    return nullptr;
}

// ASIGNACION -> id = COMPLEMENTO ;
Node* Parser::parseAsignacion() {
    Node* asig = new Node(N_ASIGNACION, "ASIGNACION", "=");

    if (check(TOKEN_ID)) {
        asig->addChild(new Node(N_ID_REF, "id", currentToken.lexeme));
        advance();
    } else {
        error("identificador");
    }

    expect(TOKEN_ASIGN, "'=' en la asignacion");
    Node* rhs = parseComplemento();
    if (rhs) asig->addChild(rhs);
    expect(TOKEN_PYCOMA, "';' al final de la asignacion");
    return asig;
}

// COMPLEMENTO -> EXPRESION | FUNCION
// Decidimos por el token actual: si es 'func' -> FUNCION, si no -> EXPRESION.
Node* Parser::parseComplemento() {
    if (currentToken.type == TOKEN_FUNC) {
        return parseFuncionCall();
    }
    return parseExpresion();
}

// SELECCION -> if ( EXPRESION ) BLOQUE SINO
// SINO -> else BLOQUE | epsilon
Node* Parser::parseSeleccion() {
    expect(TOKEN_IF, "'if'");
    Node* n = new Node(N_IF, "IF");
    expect(TOKEN_PAR_IZQ, "'('");
    Node* cond = parseExpresion();
    expect(TOKEN_PAR_DER, "')'");
    Node* thenB = parseBloque();
    if (cond)  n->addChild(cond);
    if (thenB) n->addChild(thenB);
    if (match(TOKEN_ELSE)) {
        Node* elseB = parseBloque();
        if (elseB) n->addChild(elseB);
    }
    return n;
}

// MIENTRAS -> while ( EXPRESION ) BLOQUE
Node* Parser::parseMientras() {
    expect(TOKEN_WHILE, "'while'");
    Node* n = new Node(N_WHILE, "WHILE");
    expect(TOKEN_PAR_IZQ, "'('");
    Node* cond = parseExpresion();
    expect(TOKEN_PAR_DER, "')'");
    Node* body = parseBloque();
    if (cond) n->addChild(cond);
    if (body) n->addChild(body);
    return n;
}

// RETORNO -> return EXPRESION ;
Node* Parser::parseRetorno() {
    expect(TOKEN_RETURN, "'return'");
    Node* n = new Node(N_RETURN, "RETURN");
    Node* e = parseExpresion();
    if (e) n->addChild(e);
    expect(TOKEN_PYCOMA, "';' al final de return");
    return n;
}

// FUNCION -> func id ( ARGUMENTOS )
Node* Parser::parseFuncionCall() {
    expect(TOKEN_FUNC, "'func'");
    Node* call = new Node(N_FUNCION_CALL, "FUNCION_CALL");
    if (check(TOKEN_ID)) {
        call->value = currentToken.lexeme;
        advance();
    } else {
        error("identificador tras 'func'");
    }
    expect(TOKEN_PAR_IZQ, "'('");
    parseArgumentos(call);
    expect(TOKEN_PAR_DER, "')'");
    return call;
}

// ARGUMENTOS -> EXPRESION NUEVO_ARGUMENTO | epsilon
// NUEVO_ARGUMENTO -> , EXPRESION NUEVO_ARGUMENTO | epsilon
void Parser::parseArgumentos(Node* parent) {
    if (currentToken.type == TOKEN_PAR_DER) return;  // epsilon
    Node* args = new Node(N_ARGUMENTOS, "ARGUMENTOS");
    Node* e = parseExpresion();
    if (e) args->addChild(e);
    while (match(TOKEN_COMA)) {
        Node* ei = parseExpresion();
        if (ei) args->addChild(ei);
    }
    parent->addChild(args);
}

// ---------- Expresiones (precedencia de menor a mayor) ----------
// Aunque la gramatica usa recursion a la derecha (EXP_ADITIVA2, etc.),
// construimos el AST con asociatividad IZQUIERDA via un bucle. Esto es
// lo esperado semanticamente para aritmetica y logica.

// EXPRESION -> EXPRESION_O
Node* Parser::parseExpresion() {
    return parseExpresionO();
}

// EXPRESION_O -> EXPRESION_Y ( || EXPRESION_Y )*
Node* Parser::parseExpresionO() {
    Node* left = parseExpresionY();
    while (currentToken.type == TOKEN_OR) {
        string op = currentToken.lexeme;
        advance();
        Node* right = parseExpresionY();
        Node* bin = new Node(N_BINOP, op);
        bin->addChild(left);
        bin->addChild(right);
        left = bin;
    }
    return left;
}

// EXPRESION_Y -> EXPRESION_RELACIONAL ( && EXPRESION_RELACIONAL )*
Node* Parser::parseExpresionY() {
    Node* left = parseExpresionRelacional();
    while (currentToken.type == TOKEN_AND) {
        string op = currentToken.lexeme;
        advance();
        Node* right = parseExpresionRelacional();
        Node* bin = new Node(N_BINOP, op);
        bin->addChild(left);
        bin->addChild(right);
        left = bin;
    }
    return left;
}

// EXPRESION_RELACIONAL -> EXPRESION_ADITIVA ( comp EXPRESION_ADITIVA )?
// La gramatica permite a lo mas UN operador relacional (no asociativo).
Node* Parser::parseExpresionRelacional() {
    Node* left = parseExpresionAditiva();
    if (currentToken.type == TOKEN_COMP) {
        string op = currentToken.lexeme;
        advance();
        Node* right = parseExpresionAditiva();
        Node* bin = new Node(N_BINOP, op);
        bin->addChild(left);
        bin->addChild(right);
        return bin;
    }
    return left;
}

// EXPRESION_ADITIVA -> EXPRESION_MULTIPLICATIVA ( (+|-) EXPRESION_MULTIPLICATIVA )*
Node* Parser::parseExpresionAditiva() {
    Node* left = parseExpresionMultiplicativa();
    while (currentToken.type == TOKEN_MAS || currentToken.type == TOKEN_MENOS) {
        string op = currentToken.lexeme;
        advance();
        Node* right = parseExpresionMultiplicativa();
        Node* bin = new Node(N_BINOP, op);
        bin->addChild(left);
        bin->addChild(right);
        left = bin;
    }
    return left;
}

// EXPRESION_MULTIPLICATIVA -> FACTOR ( (*|/|%) FACTOR )*
Node* Parser::parseExpresionMultiplicativa() {
    Node* left = parseFactor();
    while (currentToken.type == TOKEN_POR ||
           currentToken.type == TOKEN_DIV ||
           currentToken.type == TOKEN_MOD) {
        string op = currentToken.lexeme;
        advance();
        Node* right = parseFactor();
        Node* bin = new Node(N_BINOP, op);
        bin->addChild(left);
        bin->addChild(right);
        left = bin;
    }
    return left;
}

// FACTOR -> SIGNO TERMINO | texto | ! FACTOR | ( EXPRESION )
// SIGNO  -> + | - | epsilon
Node* Parser::parseFactor() {
    if (currentToken.type == TOKEN_TEXTO) {
        Node* t = new Node(N_TEXTO, "texto", currentToken.lexeme);
        advance();
        return t;
    }
    if (currentToken.type == TOKEN_NOT) {
        advance();
        Node* u = new Node(N_UNARY, "!");
        u->addChild(parseFactor());
        return u;
    }
    if (currentToken.type == TOKEN_PAR_IZQ) {
        advance();
        Node* e = parseExpresion();
        expect(TOKEN_PAR_DER, "')' para cerrar expresion");
        return e;
    }

    // SIGNO opcional + TERMINO
    string signo;
    if (currentToken.type == TOKEN_MAS || currentToken.type == TOKEN_MENOS) {
        signo = currentToken.lexeme;
        advance();
    }
    Node* term = parseTermino();
    if (!signo.empty() && term != nullptr) {
        Node* u = new Node(N_UNARY, signo);
        u->addChild(term);
        return u;
    }
    return term;
}

// TERMINO -> id DIMENSION2 | CONSTANTE
// CONSTANTE -> num_int | num_dec
Node* Parser::parseTermino() {
    if (currentToken.type == TOKEN_ID) {
        Node* idNode = new Node(N_ID_REF, "id", currentToken.lexeme);
        advance();
        parseDimension2(idNode);
        return idNode;
    }
    if (currentToken.type == TOKEN_NUM_INT) {
        Node* n = new Node(N_NUM_INT, "num_int", currentToken.lexeme);
        advance();
        return n;
    }
    if (currentToken.type == TOKEN_NUM_DEC) {
        Node* n = new Node(N_NUM_DEC, "num_dec", currentToken.lexeme);
        advance();
        return n;
    }
    error("identificador o numero");
    advance();  // evita loop infinito
    return nullptr;
}
