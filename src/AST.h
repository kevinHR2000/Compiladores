#ifndef AST_H
#define AST_H

#include <string>
#include <vector>
#include <iostream>

// =====================================================================
// AST - Arbol Sintactico Abstracto para LP (Entregable 2)
// ---------------------------------------------------------------------
// Diseno minimalista: UN solo tipo Node con una etiqueta y una lista de
// hijos. Esto evita tener una jerarquia de clases con herencia virtual
// y mantiene la impresion/serializacion muy simple. El 'kind' permite
// diferenciar nodos a futuro (p.ej. para analisis semantico).
// =====================================================================

enum NodeKind {
    N_PROGRAMA,
    N_FUNCION_DECL,
    N_PARAMETROS,
    N_PARAMETRO,
    N_PRINCIPAL,
    N_BLOQUE,
    N_LISTA_VARS,
    N_VAR_DECL,
    N_TIPO,
    N_ASIGNACION,
    N_IF,
    N_WHILE,
    N_SCANF,
    N_PRINTLN,
    N_RETURN,
    N_FUNCION_CALL,
    N_ARGUMENTOS,
    N_BINOP,          // ||, &&, comp, +, -, *, /, %
    N_UNARY,          // !, unario -
    N_ID_REF,         // uso de id en expresion (puede tener accesos de arreglo como hijos)
    N_INDICE,         // expresion dentro de [ ]
    N_NUM_INT,
    N_NUM_DEC,
    N_TEXTO
};

struct Node {
    NodeKind kind;
    std::string label;             // etiqueta legible (ej: "ASIGNACION", "+", "IF")
    std::string value;             // lexema asociado si aplica (ej: "edad", "20")
    std::vector<Node*> children;

    Node(NodeKind k, const std::string& lbl, const std::string& val = "")
        : kind(k), label(lbl), value(val) {}

    ~Node() {
        for (Node* c : children) delete c;
    }

    void addChild(Node* c) {
        if (c != nullptr) children.push_back(c);
    }
};

// Imprime el AST con sangria por nivel (util para evidencia en consola).
void printAST(const Node* root, std::ostream& out, int depth = 0);

// Escribe una representacion del AST en formato DOT (Graphviz) al archivo
// indicado. Luego puede visualizarse con: dot -Tpng ast.dot -o ast.png
void writeDot(const Node* root, const std::string& filepath);

#endif
