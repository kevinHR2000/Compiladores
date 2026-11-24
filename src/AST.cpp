#include "AST.h"
#include <fstream>

using namespace std;

// Escapa caracteres problematicos para la etiqueta DOT entre comillas dobles.
static string escape(const string& s) {
    string out;
    out.reserve(s.size());
    for (char c : s) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n";  break;
            default:   out += c;
        }
    }
    return out;
}

void printAST(const Node* root, ostream& out, int depth) {
    if (root == nullptr) return;
    for (int i = 0; i < depth; ++i) out << "  ";
    out << root->label;
    if (!root->value.empty()) out << " [" << root->value << "]";
    out << "\n";
    for (const Node* c : root->children) {
        printAST(c, out, depth + 1);
    }
}

// Recorrido DFS que asigna ids enteros a los nodos y emite las aristas
// padre->hijo. Devuelve el id asignado al subarbol raiz pasado.
static int writeNode(const Node* root, ofstream& out, int& counter) {
    int id = counter++;

    // Construimos el label escapando cada parte por separado, de modo que
    // el "\n" que insertamos para el salto de linea NO sea doble-escapado
    // por la funcion escape(). Graphviz interpreta \n (dos caracteres:
    // backslash + n) dentro de un label entre comillas como nueva linea.
    string lbl = escape(root->label);
    if (!root->value.empty()) {
        lbl += "\\n";
        lbl += escape(root->value);
    }

    out << "  n" << id << " [label=\"" << lbl << "\"];\n";
    for (const Node* c : root->children) {
        int childId = writeNode(c, out, counter);
        out << "  n" << id << " -> n" << childId << ";\n";
    }
    return id;
}

void writeDot(const Node* root, const string& filepath) {
    ofstream out(filepath);
    if (!out.is_open()) {
        cerr << "[AST] No se pudo abrir '" << filepath << "' para escritura." << endl;
        return;
    }
    out << "digraph AST {\n";
    out << "  node [shape=box, fontname=\"Consolas\"];\n";
    out << "  rankdir=TB;\n";

    if (root != nullptr) {
        int counter = 0;
        writeNode(root, out, counter);
    }

    out << "}\n";
    out.close();
}
