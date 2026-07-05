# 🔤 Sintaxis y Estructura

## 📐 Estructura Base

PROGRAMA NombrePrograma
    // Recomendado para declaraciones de variables globales, funciones y
    // subprogramas.
BLOQUE PRINCIPAL
    // Código aquí
FIN PRINCIPAL
FINAL

## 📦 Variables y Tipos

// Declaración automática
VARIABLE TEXTO $nombre = "Diego"

// Enteros (admiten SIN SIGNO para rango 0–4.294.967.295)
VARIABLE ENTERA $edad = 30
VARIABLE ENTERA SIN SIGNO $id_maxima = 4294967295

// Decimales (admiten SIN SIGNO si solo usás positivos)
VARIABLE DECIMAL $pi = 3.14

// Caracteres (1 byte, 0–255 con SIN SIGNO)
VARIABLE CARACTER $letra = 'A'
VARIABLE CARACTER SIN SIGNO $byte = 200

// Listas
LISTA TEXTO $frutas[2] = {"Manzana", "Pera"}
// Acceso: $frutas[0]

// Matrices
MATRIZ ENTERA $tabla[3][3] = {{10, 20, 30}, {25, 35, 45}, {115, 120, 125}}
// Acceso: $tabla[fila][columna] → $tabla[1][2] devuelve 45

### ⚠️ Nota sobre `CARACTER SIN SIGNO` y Salida en Terminal
`VARIABLE CARACTER SIN SIGNO` almacena un **byte sin interpretar** (`0`–`255`). 
Al imprimirlo con `ESCRIBIR`, Nico envía el byte crudo a `stdout`. En terminales modernas, el renderizador gráfico espera secuencias `UTF-8` válidas; 
al recibir un byte `>127` aislado, lo muestra como `` o "basura" visual. 
**Esto no indica un fallo en Nico**: la variable guarda, compara y procesa el 
valor correctamente (como validaste con `SI`).

✅ **Práctica recomendada:**
- Usá `CARACTER SIN SIGNO` para **lógica interna, flags, buffers o comunicación de bajo nivel**.
- Para verificar su valor, imprimilo como número: `ESCRIBIR("$var") SALTO`
- Para caracteres visibles en consola o web, usá `VARIABLE TEXTO $var = "ñ"`.

## 📝 Texto

### TEXTO (memoria dinámica)
Para cadenas de cualquier longitud con soporte UTF-8 completo (acentos, ñ, emojis):

    VARIABLE TEXTO $saludo = "¡Hola, Diego! ¿Cómo estás?"
    VARIABLE TEXTO $emoji = "🚀"
    ESCRIBIR($saludo) SALTO

**Características:**
- Memoria dinámica (se ajusta automáticamente al contenido)
- Soporte completo UTF-8
- Límite: solo la RAM disponible
- Compatible con todas las funciones de texto (LONGITUDTEXTO, COPIARTEXTO, etc.)

### TEXTO EXTENSO (alias)
`TEXTO EXTENSO` es un alias de `TEXTO`. Funcionalmente son idénticos, 
pero podés usar `TEXTO EXTENSO` para indicar semánticamente que el texto 
será largo (archivos, JSON, HTML):

    VARIABLE TEXTO EXTENSO $documento = "Contenido muy largo..."