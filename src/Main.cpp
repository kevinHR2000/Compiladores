#include "Lexer.h"
#include "Parser.h"
#include "AST.h"
#include <iostream>
#include <string>
#include <vector>

using namespace std;

// =====================================================================
// Main: punto de entrada del compilador LP (Entregable 2).
// Flujo:
//   archivo.lp -> Lexer -> tokens -> Parser -> AST -> Graphviz (.dot)
//
// Uso:
//   Main.exe                                   -> MENU interactivo (lista los .lp)
//   Main.exe archivo.lp                        -> solo analisis sintactico
//   Main.exe archivo.lp --tokens               -> imprime lista de tokens
//   Main.exe archivo.lp --ast                  -> imprime AST por consola
//   Main.exe archivo.lp --dot [salida.dot]     -> genera AST en Graphviz
// Las banderas son combinables. Si se omite el nombre del .dot, se usa
// el mismo nombre del archivo fuente con extension .dot.
// =====================================================================

// Lista de archivos .lp validos que ofrece el menu interactivo.
// Si agregas o renombras un test, actualiza esta tabla para que aparezca.
static const vector<string>& menuFiles() {
    static const vector<string> files = {
        "tests/valido_e2_completo.lp",
        "tests/valido_e2_expresiones.lp",
        "tests/valido_e2_funciones.lp",
        "src/valido1.lp",
        "src/valido2.lp"
    };
    return files;
}

static bool hasFlag(int argc, char* argv[], const string& flag) {
    for (int i = 2; i < argc; ++i) {
        if (string(argv[i]) == flag) return true;
    }
    return false;
}

// Devuelve el argumento que sigue a --dot, o cadena vacia si no hay.
static string dotOutputPath(int argc, char* argv[]) {
    for (int i = 2; i < argc; ++i) {
        if (string(argv[i]) == "--dot" && i + 1 < argc) {
            string next = argv[i + 1];
            // Si el siguiente arg empieza con '--' no es un path.
            if (next.size() >= 2 && next.substr(0, 2) == "--") return "";
            return next;
        }
    }
    return "";
}

// Deriva el nombre por defecto del .dot a partir del archivo fuente:
// "tests/foo.lp" -> "tests/foo.dot"
static string defaultDotPath(const string& source) {
    size_t dot = source.find_last_of('.');
    if (dot == string::npos) return source + ".dot";
    return source.substr(0, dot) + ".dot";
}

// Nucleo del flujo: lexer -> parser -> AST -> (opcional) consola/.dot.
// Se usa tanto desde el modo CLI como desde el menu interactivo.
static int runAnalysis(const string& sourcePath,
                       bool showTokens, bool showAst, bool genDot,
                       const string& customDotPath) {
    // ---------- Fase opcional: imprimir lista de tokens ----------
    // Lexer separado porque imprimir consume todos los tokens.
    if (showTokens) {
        Lexer lexerLex;
        if (!lexerLex.loadFile(sourcePath)) return 1;

        cout << "===== LISTA DE TOKENS =====" << endl;
        cout << "TIPO\t\tLEXEMA\t\tLINEA\tCOL\tTOKEN" << endl;
        cout << "---------------------------------------------------------" << endl;

        Token token = lexerLex.getNextToken();
        while (token.type != TOKEN_EOF) {
            cout << token.getTypeString() << "\t\t"
                 << token.lexeme << "\t\t"
                 << token.line << "\t"
                 << token.column << "\t";
            if (token.type == TOKEN_ID) {
                cout << "<ID," << token.attribute << ">";
            } else {
                cout << "<" << token.getTypeString() << ">";
            }
            cout << endl;
            token = lexerLex.getNextToken();
        }
        cout << "EOF\t\t\t\t" << token.line << "\t" << token.column << endl;
    }

    // ---------- Fase principal: analisis sintactico ----------
    Lexer lexer;
    if (!lexer.loadFile(sourcePath)) return 1;

    cout << "\n===== ANALISIS SINTACTICO =====" << endl;
    Parser parser(lexer);
    bool ok = parser.parse();

    // ---------- Fase opcional: imprimir AST en consola ----------
    if (showAst) {
        cout << "\n===== ARBOL SINTACTICO (AST) =====" << endl;
        printAST(parser.getAST(), cout);
    }

    // ---------- Fase opcional: generar Graphviz (.dot) ----------
    if (genDot) {
        string outPath = customDotPath.empty() ? defaultDotPath(sourcePath)
                                               : customDotPath;
        writeDot(parser.getAST(), outPath);
        cout << "\n[AST] Grafo Graphviz generado en: " << outPath << endl;
        cout << "      Visualizar con: dot -Tpng " << outPath
             << " -o " << outPath.substr(0, outPath.find_last_of('.'))
             << ".png" << endl;
    }

    // Tabla de simbolos: evidencia heredada del lexer.
    lexer.getSymbolTable().print();

    return ok ? 0 : 1;
}

// Modo interactivo: muestra la lista de .lp validos, pide al usuario
// elegir uno y corre analisis + AST + .dot sobre esa seleccion. Pensado
// para la sustentacion: sin recordar rutas ni banderas.
static int runMenu() {
    const vector<string>& files = menuFiles();

    cout << "===== Compilador LP - Modo interactivo =====" << endl;
    cout << "Selecciona un archivo .lp valido para analizar:" << endl;
    for (size_t i = 0; i < files.size(); ++i) {
        cout << "  " << (i + 1) << ") " << files[i] << endl;
    }
    cout << "  0) Salir" << endl;
    cout << "Opcion: " << flush;

    int choice = -1;
    if (!(cin >> choice)) {
        cerr << "Entrada invalida." << endl;
        return 1;
    }
    if (choice == 0) {
        cout << "Hasta luego." << endl;
        return 0;
    }
    if (choice < 1 || static_cast<size_t>(choice) > files.size()) {
        cerr << "Opcion fuera de rango." << endl;
        return 1;
    }

    const string& sourcePath = files[choice - 1];
    cout << "\n>>> Analizando: " << sourcePath << " <<<" << endl;

    // En el menu generamos AST en consola y .dot por defecto: es la
    // evidencia tipica que se pide para la sustentacion del Entregable 2.
    return runAnalysis(sourcePath,
                       /*showTokens*/ false,
                       /*showAst*/    true,
                       /*genDot*/     true,
                       /*customDot*/  "");
}

int main(int argc, char* argv[]) {
    // Sin argumentos -> modo interactivo (eleccion de archivo por menu).
    if (argc < 2) {
        return runMenu();
    }

    string sourcePath = argv[1];
    bool showTokens = hasFlag(argc, argv, "--tokens");
    bool showAst    = hasFlag(argc, argv, "--ast");
    bool genDot     = hasFlag(argc, argv, "--dot");
    string customDot = dotOutputPath(argc, argv);

    return runAnalysis(sourcePath, showTokens, showAst, genDot, customDot);
}
