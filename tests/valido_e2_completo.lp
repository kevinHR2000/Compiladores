// Ejemplo integral del Entregable 2 (seccion 5.2 del PDF):
// declaraciones, asignaciones, if/else con && y comp, while, println y return.
void main() {
    int edad;
    float promedio;
    edad = 20;
    promedio = 15.5;

    if (edad >= 18 && edad <= 60) {
        println("Edad valida");
    } else {
        println("Edad no valida");
    }

    while (edad > 0) {
        edad = edad - 1;
    }

    return edad;
}
