// Ejercita la declaracion de funciones, parametros, llamada con 'func',
// scanf, expresiones aritmeticas con precedencia y arreglos.
int suma(int a, int b) {
    int r;
    r = a + b;
    return r;
}

float promediar(int x, int y) {
    float p;
    p = (x + y) / 2;
    return p;
}

void main() {
    int[10] datos;
    int i, total;
    float media;

    i = 0;
    total = 0;

    scanf(datos[0], datos[1]);

    while (i < 10) {
        total = total + datos[i];
        i = i + 1;
    }

    total = func suma(total, 100);
    media = func promediar(total, i);

    println("Total:", total, "Media:", media);
}
