// Cobertura de la jerarquia de expresiones: precedencia aritmetica,
// signos unarios, negacion logica, parentesis y operadores logicos.
void main() {
    int a, b, c;
    boolean ok;

    a = 2 + 3 * 4;              // 14 (precedencia *)
    b = (2 + 3) * 4;            // 20 (parentesis)
    c = -a + b % 7;             // signo unario + modulo
    ok = !(a == b) && a < b || c >= 0;
}
