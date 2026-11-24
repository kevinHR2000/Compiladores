# Compiladores
Evidencias Grupo 6

## Objetivo
Implementar en C++ un compilador para el lenguaje LP, compuesto por:
1. **Analizador léxico (Lexer)** — ya entregado (fases 1 a 3).
2. **Analizador sintáctico (Parser)** — dos entregables (30% + 70%), ambos cubiertos.

## Estructura del proyecto
```
src/     Código fuente (Main, Lexer, SymbolTable, Token, Parser, AST)
tests/   Archivos de entrada de prueba (.lp, .txt, .dot generados)
docs/    Material del curso (enunciado léxico, gramática LL(1), especificaciones)
```

Archivos de referencia en `docs/`:
- `Gramatica_LP_LL1.pdf` — gramática LL(1) de LP.
- `Especificaciones_Proyecto-Analizador_Sintactico_AL.pdf` — reglas de entrega.

## Compilar
```
g++ -std=c++17 src/Main.cpp src/Lexer.cpp src/SymbolTable.cpp src/Parser.cpp src/AST.cpp -o src/Main.exe
```
En VS Code: `Ctrl+Shift+B` ejecuta la tarea "Compilar LexLP".

## Ejecutar

### Análisis sintáctico (modo por defecto)
```
./src/Main.exe tests/valido_e2_completo.lp
```
Imprime si el programa es **ACEPTADO** o **RECHAZADO** y la tabla de símbolos.

### Banderas combinables
| Bandera | Efecto |
|---|---|
| `--tokens` | Imprime la lista de tokens antes del análisis (evidencia del lexer). |
| `--ast` | Imprime el AST con sangría por nivel tras el análisis. |
| `--dot [salida.dot]` | Escribe el AST en formato Graphviz. Si se omite el path, usa `<archivo>.dot`. |

Ejemplos:
```
./src/Main.exe tests/valido_e2_completo.lp --tokens --ast
./src/Main.exe tests/valido_e2_funciones.lp --dot
dot -Tpng tests/valido_e2_funciones.dot -o tests/valido_e2_funciones.png
```

## Código de salida
- `0` → programa aceptado sintácticamente.
- `1` → errores sintácticos o problema al abrir el archivo.

## Archivos de prueba

### Entregable 1 (`src/`)
- `valido1.lp` — ejemplo mínimo del PDF (declaraciones + asignaciones).
- `valido2.lp` — múltiples IDs por declaración, arreglos (`float[10] notas`), varios tipos base.
- `invalido1.lp` — falta `;` al final de una declaración.
- `invalido2.lp` — llave `{` sin cerrar.
- `invalido3.lp` — paréntesis `(` sin cerrar en `main`.

### Entregable 2 (`tests/`)
- `valido_e2_completo.lp` — ejemplo integral (if/else, while, println, return) del enunciado 5.2.
- `valido_e2_expresiones.lp` — jerarquía completa de precedencia (aritmética, unarios, `!`, `&&`, `||`, paréntesis).
- `valido_e2_funciones.lp` — declaraciones de funciones con parámetros, llamadas (`func`), `scanf`, arreglos.
- `invalido_e2_if.lp` — falta `)` en la condición del `if`.
- `invalido_e2_return.lp` — `return;` sin expresión (la gramática exige `return EXPRESION ;`).

### Cobertura del Entregable 1 (sección 4.1 del PDF)
| Requisito | Dónde se cubre |
|---|---|
| Integración lexer→parser | `Main.cpp` + `Parser(Lexer&)` |
| `PROGRAMA` y `PRINCIPAL` | `parsePrograma`, `parsePrincipal` |
| `BLOQUE` con llaves | `parseBloque` |
| Declaraciones (`TIPO`, `TIPO_BASE`, `DIMENSION`, `LISTA_VARIABLES`, `LISTA_ID`) | `parseTipo`, `parseTipoBase`, `parseDimension`, `parseListaVariables`, `parseListaId` |
| Asignaciones (`ASIGNACION`, `COMPLEMENTO`) | `parseAsignacion`, `parseComplemento` |
| Errores: `;` faltante, paréntesis/llaves sin cerrar, token inesperado | `expect()`, `error()` |

### Cobertura del Entregable 2 (sección 5.1 del PDF)
| Requisito | Dónde se cubre |
|---|---|
| Expresiones con precedencia (`||`, `&&`, comp, `+-`, `*/%`) | `parseExpresionO/Y/Relacional/Aditiva/Multiplicativa` |
| Factores y términos (id, constantes, texto, signos unarios, `!`, paréntesis) | `parseFactor`, `parseTermino` |
| `scanf` / `println` con lista de términos | `parseLectura`, `parseEscritura`, `parseTerminoLectura/Escritura` |
| `if` / `else` / `while` | `parseSeleccion`, `parseMientras` |
| Funciones: declaración, parámetros, llamada (`func`), argumentos | `parseDeclaracionFuncion`, `parseParametros`, `parseFuncionCall`, `parseArgumentos` |
| `return EXPRESION ;` | `parseRetorno` |
| AST | `AST.h`/`AST.cpp`, nodos construidos por cada `parseXxx()`, salidas `--ast` y `--dot` |
| Mensajes de error con ubicación y tokens esperado/encontrado | `error()` |
| Recuperación básica para reportar varios errores | `synchronize()` (panic-mode a `;` o `}`) |

## Notas sobre la gramática LP
- **`void` opcional antes de `main`**: la gramática formal pide `main() BLOQUE`, pero el ejemplo del PDF usa `void main()`. El parser acepta ambos.
- **Dimensión de arreglos va con el TIPO**: en LP se escribe `float[10] notas;`, NO `float notas[10];` (producción `TIPO → TIPO_BASE DIMENSION`).
- **`func` es palabra reservada**: añadido al lexer para la producción `FUNCION → func id ( ARGUMENTOS )`.
- **`return` exige expresión**: la gramática no admite `return;` solo.
- **No hay `for`**: la gramática solo define `while`.
- **Lookahead de 2 tokens**: necesario en `PROGRAMA` para distinguir `void main` (es `PRINCIPAL`) de `void foo` (es `DECLARACION_FUNCION`).
- **Asociatividad izquierda** en expresiones: aunque la gramática usa recursión a la derecha (`EXPRESION_ADITIVA2`, etc.), el AST se construye con asociatividad izquierda mediante bucles, lo esperado semánticamente.

## Entregas
### Entregables léxico (ya cerrados)
- 15/09: lectura + NUM_INT + NUM_DEC
- 22/09: ID + TEXTO + palabras reservadas + tabla de símbolos
- 29/09: operadores + integración + lista de tokens + errores

### Entregables sintáctico
- **13/11 — Entregable 1 (30%)**: integración lexer+parser, `main`/bloque/declaraciones/asignaciones + errores básicos.
- **24/11 — Entregable 2 (70%)**: expresiones completas con precedencia, `if`/`else`/`while`, `scanf`/`println`, funciones, `return`, AST (consola + Graphviz), mejor recuperación de errores.
