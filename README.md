# 🐧 Nico v2.0.0 - Intérprete Educativo de Scripting en Español

Nico es un lenguaje de programación interpretado y 100% en español, diseñado para aprender conceptos fundamentales de programación sin barreras de idioma. Con un solo binario y sin dependencias externas, incluye soporte nativo para hardware (GPIO/PWM), bases de datos SQLite, servidor web integrado y gráficos ASCII.

## ✨ Características Principales

- **Sintaxis en español**: `SI`, `MIENTRAS`, `FUNCION`, `BLOQUE PRINCIPAL`, `ESCRIBIR`, `CALCULAR EN`, etc.
- **Arquitectura AST**: Parser y evaluador basado en árbol de sintaxis abstracta para mejor rendimiento y mantenibilidad.
- **Funciones y subprogramas**: Soporte completo para funciones con retorno y subprogramas sin retorno, con scopes locales aislados.
- **Sistema de inclusión**: `INCLUIR "archivo.nico"` para modularizar código y reutilizar funciones.
- **15 tipos de datos nativos**: Enteros, decimales, caracteres, texto, logica y variantes `SIN SIGNO`, en `VARIABLE`, `LISTA` y `MATRIZ`.
- **Seguridad de tipos**: Pools separados, validación de índices y gestión automática de memoria para colecciones.
- **Optimización de memoria**: Pool de tablas de símbolos reutilizables para reducir overhead en llamadas recursivas.
- **Recursión profunda**: Soporte hasta 7000 niveles de recursión con stack de 64MB en Windows y configurable en Linux.
- **GPIO y PWM para Raspberry Pi**: Control nativo de pines digitales y señales PWM (máx 500Hz) para servos, LEDs y sensores.
- **Base de datos SQLite**: Consultas SQL estándar directamente desde Nico, sin configuración adicional.
- **Servidor web integrado**: API REST para consultar bases de datos desde navegadores o aplicaciones externas.
- **Gráficos ASCII**: Primitivas de dibujo (`DIBUJARLINEA`, `DIBUJARCIRCULO`, `RELLENARRECTANGULO`) para UIs en consola.
- **Operaciones bit-a-bit**: Funciones completas para manipulación de bits (`BITY`, `BITO`, `BITXOR`, `LEERBIT`, etc.).
- **E/S completa**: `ESCRIBIR`, `LEER`, `LEERCARACTER`, `LEERHASTA`, manejo de archivos, colores, cursor y efectos de terminal.
- **Biblioteca nativa**: Funciones matemáticas (`SENO`, `RAIZ`, `POTENCIA`) y de texto (`LONGITUDTEXTO`, `COPIARTEXTO`, `INVERTIRTEXTO`, etc.).
- **Cross-platform**: Compila y corre en Linux (incluyendo Raspberry Pi) y Windows sin dependencias externas.

## 🚀 Inicio Rápido

### Compilar

**Linux / macOS:**
    chmod +x compile.sh && ./compile.sh

**Windows:** compile_windows.bat

**Raspberry Pi:** ./compile.sh (detecta y habilita GPIO automáticamente)

### Ejecutar

    ./nico ejemplo.nico

**Modo interactivo:** ./nico

## 📖 Ejemplo de Uso

    PROGRAMA HolaNico
        VARIABLE TEXTO $nombre = "Mundo"
        VARIABLE DECIMAL $precio = 19.99
        VARIABLE ENTERA $cantidad = 3

    BLOQUE PRINCIPAL
        CALCULAR EN $total = $precio * $cantidad
        ESCRIBIR("¡Hola, $nombre!") SALTO
        ESCRIBIR("Total: $total") SALTO
    FIN PRINCIPAL
    FINAL

## 🧠 Sistema de Tipos

Nico usa 15 tipos explícitos en 3 categorías:

**Escalares:** ENTERA, DECIMAL, CARACTER, LOGICA (VERDADERO/FALSO o 1/0), TEXTO, y sus variantes SIN SIGNO.

**Listas:** LISTA ENTERA, LISTA DECIMAL, LISTA LOGICA, LISTA TEXTO, etc. (arrays 1D).

**Matrices:** MATRIZ ENTERA, MATRIZ DECIMAL, MATRIZ LOGICA, etc. (arrays 2D).

**Mapas de bits:** Usá LISTA LOGICA o MATRIZ LOGICA para representar sets, flags o estados:

    // Lista de flags
    LISTA LOGICA $flags[100]
    $flags[0] = VERDADERO
    $flags[1] = 1
    
    // Matriz para tablero/mapa de bits
    MATRIZ LOGICA $tablero[3][3]
    $tablero[0][0] = VERDADERO
    $tablero[1][1] = 0

## 🔒 Scopes y Shadowing

Cada función crea un entorno local que oculta variables globales con el mismo nombre. Al retornar, el contexto global se restaura intacto.

    PROGRAMA Test
        VARIABLE ENTERA $x = 100
        VARIABLE ENTERA $resultado = 0

        FUNCION ENTERA demo()
            VARIABLE ENTERA $x = 50
            ESCRIBIR("Local: $x") SALTO
            RESULTADO EN $x
        FIN FUNCION

    BLOQUE PRINCIPAL
        ESCRIBIR("Global: $x") SALTO
        CALCULAR EN $resultado = demo()
        ESCRIBIR("Global después: $x") SALTO
    FIN PRINCIPAL
    FINAL

## 📝 Filosofía de Asignación

| Tipo | Sintaxis | Razón |
|------|----------|-------|
| Escalares | CALCULAR EN $var = expresión | Claridad semántica para cálculos |
| Resultado | RESULTADO EN $var = valor | Retorno explícito de funciones |
| Colecciones | $lista[indice] = valor | La indexación ya implica mutación |
| Texto | COPIARTEXTO($txt, "..." | Copia explícita para evitar efectos secundarios |

## 📌 Funciones y Subprogramas

### Funciones con retorno

Las funciones retornan un valor usando RESULTADO EN:

    FUNCION ENTERA sumar($a, $b)
        VARIABLE ENTERA $suma = 0
        CALCULAR EN $suma = $a + $b
        RESULTADO EN $suma
    FIN FUNCION

### Subprogramas sin retorno

Los subprogramas ejecutan acciones sin retornar valor (funciones vacías):

    SUBPROGRAMA saludar($nombre)
        ESCRIBIR("Hola, $nombre") SALTO
    FIN SUBPROGRAMA

También podés usar FUNCION sin tipo de retorno:

    FUNCION limpiar_pantalla()
        LIMPIARPANTALLA()
    FIN FUNCION

### Llamada a funciones y subprogramas

    BLOQUE PRINCIPAL
        VARIABLE ENTERA $resultado = 0
        CALCULAR EN $resultado = sumar(5, 3)
        ESCRIBIR("Suma: $resultado") SALTO
        
        LLAMAR A saludar("Diego")
    FIN PRINCIPAL

## 📦 Sistema de Inclusión

Modularizá tu código incluyendo archivos externos:

    PROGRAMA Principal
        INCLUIR "Operaciones.nico"

    BLOQUE PRINCIPAL
        VARIABLE ENTERA $resultado = 0
        CALCULAR EN $resultado = sumar(5, 3)
        ESCRIBIR("Resultado: $resultado") SALTO
    FIN PRINCIPAL
    FINAL

El archivo Operaciones.nico contendría:

    FUNCION ENTERA sumar($a, $b)
        VARIABLE ENTERA $suma = 0
        CALCULAR EN $suma = $a + $b
        RESULTADO EN $suma
    FIN FUNCION

## 🔄 Recursión y Optimización

Nico soporta recursión profunda con optimizaciones automáticas:

- **Pool de tablas de símbolos**: Reutiliza memoria en lugar de hacer malloc en cada llamada recursiva
- **Stack automático**: Aumenta el stack a 256MB para soportar recursión intensa (Linux/macOS)
- **Límite de seguridad**: 7000 niveles máximos para prevenir desbordamiento de stack

**Ejemplo: Función de Ackermann**

    FUNCION ENTERA ackermann($m, $n)
        VARIABLE ENTERA $res = 0
        SI($m IGUAL 0) ENTONCES
            RESULTADO EN $res = $n + 1
        SINO
            SI($n IGUAL 0) ENTONCES
                RESULTADO EN $res = ackermann($m - 1, 1)
            SINO
                VARIABLE ENTERA $temp = 0
                CALCULAR EN $temp = ackermann($m, $n - 1)
                RESULTADO EN $res = ackermann($m - 1, $temp)
            FIN SI
        FIN SI
        RESULTADO EN $res
    FIN FUNCION

## 📌 Interpolación

Las variables se interpolan automáticamente dentro de textos:

    ESCRIBIR("Precio: $precio") SALTO
    ESCRIBIR("A y B: $a y $b") SALTO

     nico/
    ├── src/                   # Código fuente C
    │   ├── main.c             # Punto de entrada
    │   ├── lexer.c/h          # Analizador léxico
    │   ├── parser.c/h         # Parser AST
    │   ├── evaluator.c/h      # Evaluador AST
    │   ├── ast.c/h            # Nodos del árbol de sintaxis
    │   ├── nico_gpio.c/h      # Control GPIO (Raspberry Pi)
    │   ├── nico_pwm.c/h       # Control PWM (Raspberry Pi)
    │   ├── web.c/h            # Servidor web integrado
    │   └── win_compat.h       # Compatibilidad Windows

## 🔌 Hardware: GPIO y PWM (Raspberry Pi)

Nico incluye soporte nativo para control de hardware en Raspberry Pi:

### GPIO Digital
```
// Configurar pin como entrada con pull-up (por defecto)
CONFIGURARPIN(17, ENTRADA)

// Leer estado del pin
VARIABLE ENTERA $estado = 0
ESTADOPIN(17, $estado)

// Configurar pin como salida
CONFIGURARPIN(18, SALIDA)

```
### PWM (Control de LEDs)
```
// Generar señal PWM a 100Hz con 50% duty cycle
GENERARPWM(18, 100, 50)
// Detener PWM
DETENERPWM(18)
```

> 💡 **Nota**: PWM está limitado a 500Hz máximo y funciona en pines 12, 13, 18, 19 (hardware) o cualquier pin (software).

## 🗄️ Base de Datos SQLite

Nico incluye SQLite embebido. Podés crear tablas, insertar datos y ejecutar consultas SQL estándar:

```
PROGRAMA test_bd
BLOQUE PRINCIPAL
    CONECTARBD ("test.db")

    // Crear tabla de prueba (solo la primera vez)
    EJECUTARBD ("CREATE TABLE IF NOT EXISTS usuarios (id INTEGER PRIMARY KEY, nombre TEXT, activo INTEGER)")
    
    // Insertar datos de ejemplo (si está vacía)
    EJECUTARBD ("INSERT OR IGNORE INTO usuarios (id, nombre, activo) VALUES (1, 'Diego', 1), (2, 'Ana', 1), (3, 'Inactivo', 0)")

    // Consulta con alias en inglés
    CONSULTARBD ("SELECT id, nombre FROM usuarios AS u WHERE u.activo = 1", 1)

    // Iterar: SIGUIENTEFILABD() con paréntesis obligatorios
    ESCRIBIR("") SALTO
    MIENTRAS(SIGUIENTEFILABD()) HACER
        ESCRIBIR ("Usuario: $BDCOL2") SALTO
    FIN MIENTRAS
    
    CERRARCONSULTABD
    CERRARBD
    ESCRIBIR("") SALTO
FIN PRINCIPAL
FINAL

```
## 🌐 Servidor Web Integrado

Nico puede exponer una API REST para consultar bases de datos desde navegadores:

```
PROGRAMA ServidorWeb
BLOQUE PRINCIPAL
    // Iniciar servidor en puerto 8080
    INICIAR SERVIDOR WEB EN 8080
    
    ESCRIBIR("Servidor corriendo en http://localhost:8080") SALTO
    ESCRIBIR("Presioná Ctrl+C para detener") SALTO
    
    MIENTRAS(VERDADERO) HACER
        ESPERAR(1, SEGUNDOS)
    FIN MIENTRAS
FIN PRINCIPAL
FINAL
```
## 🎨 Gráficos ASCII

Dibujá formas geométricas directamente en la consola:

```
PROGRAMA DemoGraficos
    VARIABLE ENTERA $ancho = 0
    VARIABLE ENTERA $alto = 0
BLOQUE PRINCIPAL
    ANCHOTERMINAL($ancho)
    ALTOTERMINAL($alto)
    
    // Dibujar línea horizontal
    DIBUJARLINEA(1, 1, $ancho, 1, "═")

    // Dibujar círculo
    DIBUJARCIRCULO($ancho/2, $alto/2, 10, "○")
    
    // Rellenar rectángulo
    RELLENARRECTANGULO(10, 5, 30, 15, "░")
FIN PRINCIPAL
FINAL
```
## 🔢 Operaciones Bit-a-Bit

Manipulación directa de bits para flags, máscaras y optimizaciones:

```
VARIABLE ENTERA SIN SIGNO $x = 0

// Operaciones lógicas
CALCULAR EN $x = BITY(255, 15)      // AND: resultado = 15
CALCULAR EN $x = BITO(240, 15)      // OR: resultado = 255
CALCULAR EN $x = BITXOR(255, 128)   // XOR: resultado = 127

// Desplazamientos
CALCULAR EN $x = DESPLAZARIZQUIERDA(1, 8)   // 1 << 8 = 256
CALCULAR EN $x = DESPLAZARDERECHA(256, 4)   // 256 >> 4 = 16

// Rotaciones
CALCULAR EN $x = ROTARIZQUIERDA(255, 4)

// Manipulación de bits individuales
CALCULAR EN $x = LEERBIT(255, 3)            // Lee bit 3
CALCULAR EN $x = ACTIVARBIT(0, 5)           // Activa bit 5
CALCULAR EN $x = DESACTIVARBIT(255, 5)      // Desactiva bit 5

```
## 📝 Filosofía de Asignación

| Tipo | Sintaxis | Razón |
|------|----------|-------|
| Escalares | CALCULAR EN $var = expresión | Claridad semántica para cálculos |
| Resultado | RESULTADO EN $var = valor | Retorno explícito de funciones |
| Colecciones | $lista[indice] = valor | La indexación ya implica mutación |
| Texto | COPIARTEXTO($txt, "...") | Copia explícita para evitar efectos secundarios |

## 📄 Licencia y Contribuciones

MIT License. Contribuciones bienvenidas vía Issues/PRs.

**Créditos:**
- Diseño/Arquitectura/Supervisión: Diego Alejandro Majluff
- Implementación: Qwen (Alibaba Cloud)

---

**Versión:** 2.0.0 (Julio 2026)  

**Cross-platform**: Compila y corre en Linux (incluyendo Raspberry Pi) y Windows. En Linux/RPi es un binario único; en Windows requiere DLLs incluidas (sqlite3, pthread).

**Distribución:** Linux: Binario único | Windows: Ejecutable + DLLs requeridas (incluidas)