# 📜 Changelog - Nico

Todos los cambios notables en este proyecto serán documentados en este archivo.
El formato se basa en [Keep a Changelog](https://keepachangelog.com/es-ES/1.0.0/).

---
## [2.0.0] - 2026-07-06

### 🏗️ Cambios Arquitectónicos
- **Migración completa a arquitectura AST**: El parser ahora genera un Árbol de Sintaxis Abstracta en lugar de evaluación directa, permitiendo optimizaciones futuras, mejor manejo de errores y análisis estático.
- **Eliminación de alias SQL**: Removidos `TRAER`, `DE`, `DONDE`, `COMO`, `UNIR`, `UNIRIZQ`, `SOBRE`, `AGRUPAR POR`, `ORDENAR POR`. Nico ahora usa SQL estándar directamente (`SELECT`, `FROM`, `WHERE`, etc.) para mayor claridad y compatibilidad.
- **Eliminación de `LEERLINEA`**: Removido comando redundante (idéntico a `LEER`). Limpieza de código muerto en lexer, AST y documentación.

### ✅ Agregado
- **Función `INVERTIRTEXTO`**: Nueva función de cadena para invertir el contenido de variables `TEXTO` y `TEXTO EXTENSO`.
- **Soporte de recursión profunda**: Stack size aumentado a 64MB en Windows (`-Wl,--stack,67108864`), permitiendo ejecutar algoritmos recursivos extremos como Ackermann A(4,1) sin desbordamiento.
- **Documentación de `LEERCARACTER` y `LEERHASTA`**: Comandos de entrada ahora completamente documentados con ejemplos y casos de uso.
- **Documentación completa de PWM**: Nuevo documento `19-pwm.md` con ejemplos de control de LEDs, dimmers y efectos visuales.
- **Eliminación de `TECLAMANTENIDA`**: Removido. Comando redundante. `LEERTECLA` es ahora el método oficial no bloqueante para lectura de teclado, con soporte completo para teclas especiales (flechas 1001-1004, Insert 1007, Supr 1008, Page Up/Down 1009-1010, Home 1005, End 1006).
- **Tip de recursión en Linux**: Script `compile.sh` ahora sugiere `ulimit -s unlimited` para máxima profundidad de recursión.

### 🔧 Corregido
- **Inicialización automática de listas y matrices de texto/lógica**: Corregido el bug donde `LISTA TEXTO` y `MATRIZ TEXTO` sin inicialización explícita fallaban al usar `COPIARTEXTO` (mostraban punteros de memoria en lugar de strings vacíos). Ahora se inicializan automáticamente con valores por defecto: `""` para `TEXTO`/`TEXTO EXTENSO`, `FALSO` para `LOGICA`, `0` para numéricos. Aplica tanto a listas como matrices.
- **Documentación de panel web**: Troubleshooting corregido en `04-panel-web.md`.
- **Documentación de entrada/formato**: Agregados `LEERCARACTER` y `LEERHASTA` en `09-entrada-formato.md`, removido `LEERLINEA`.

### 🔄 Mejorado
- **Scripts de compilación**: `compile_windows.bat` actualizado con stack de 64MB, `compile.sh` con tips de recursión.
- **Verificación de documentación**: Revisados y verificados los siguientes documentos:
  - `05-sistema-consola.md`: Verificado `LEERTECLA` como método oficial no bloqueante.
Documentación actualizada con códigos de teclas especiales.
  - `06-graficos-ascii.md`: Verificadas primitivas de dibujo (`DIBUJARLINEA`, `DIBUJARCIRCULO`, `RELLENARRECTANGULO`).
  - `08-fecha-hora.md`: Verificados `FECHAACTUAL` y `HORAACTUAL`.
  - `17-operaciones-bit.md`: Verificadas todas las funciones bit-a-bit.
- **Eliminación de código muerto**: `LEERLINEA` removido de lexer, AST y documentación, simplificando la base de código.

### 📚 Documentación
- 📝 **`04-panel-web.md`**: Troubleshooting corregido.
- 📝 **`05-sistema-consola.md`**: Verificación de comandos de entrada en tiempo real.
- 📝 **`06-graficos-ascii.md`**: Verificación de primitivas de dibujo.
- 📝 **`08-fecha-hora.md`**: Verificación de comandos de fecha y hora.
- 📝 **`09-entrada-formato.md`**: Agregados `LEERCARACTER` y `LEERHASTA`, removido `LEERLINEA`.
- 📝 **`17-operaciones-bit.md`**: Verificación de todas las funciones bit-a-bit.
- 📝 **`19-pwm.md`**: Nuevo documento con control de LEDs, dimmers y efectos visuales.

### 🖥️ Compatibilidad Verificada
| Plataforma | Compilador | Estado | Notas |
|------------|-----------|--------|-------|
| **Linux** (Ubuntu/Debian) | GCC 9+, x86_64/ARM64 | ✅ Compilación limpia | Stack configurable con `ulimit` |
| **Windows 10/11** | MinGW-w64 (MINGW64), x86_64 | ✅ Compilación limpia | Stack 64MB hardcodeado |
| **Raspberry Pi 4/5** | GCC, Pi OS Bookworm | ✅ GPIO + PWM + recursión profunda | `libgpiod` validado |

### 🧪 Tests de Validación
- ✅ `TestAckermannOptimizado.nico`: Ackermann A(3,9)=4093, A(4,0)=13 ejecutados sin desbordamiento.
- ✅ `TestBits.nico`: Todas las operaciones bit-a-bit funcionando correctamente.
- ✅ `TestGraficos.nico`: Primitivas ASCII renderizando correctamente en terminales UTF-8.
- ✅ `TestPWM.nico`: Control de LEDs con dimmer y efectos de respiración.

### 👥 Créditos
- **Diseño, Arquitectura y Supervisión:** Diego Alejandro Majluff
- **Implementación, Debugging y Optimización:** Qwen (Alibaba Cloud)
- **Licencia:** MIT / Uso Educativo

> 💡 **Nota:** Nico v2.0.0 es un release de madurez arquitectónica que migra a AST, elimina features redundantes (alias SQL, LEERLINEA) y completa la documentación de todos los módulos. La arquitectura AST establece la base para optimizaciones futuras, análisis estático y compilación a bytecode.

---

## [1.1.1] - 2026-06-08

### ✅ Agregado
- **Soporte completo de `TEXTO EXTENSO` en todas las funciones de cadena**: `COPIARTEXTO`, `CONCATENARTEXTO`, `MAYUSCULAS`, `MINUSCULAS`, `RECORTARTEXTO`, `REEMPLAZARTEXTO`, `REPETIRTEXTO`, `EXTRAERTEXTO`, `ENTEROATEXTO`, `DECIMALATEXTO`, `DIVIDIRTEXTO` y `CARACTERATEXTO` ahora operan indistintamente sobre variables `TEXTO` y `TEXTO EXTENSO`.
- **Inicialización de `TEXTO EXTENSO` con valor literal**: Sintaxis `VARIABLE TEXTO EXTENSO $var = "Hola"` ahora asigna correctamente el valor inicial en la declaración, con soporte para caracteres escapados (`\n`, `\t`, `\"`, `\'`, `\\`).
- **Memoria dinámica transparente**: Las operaciones con `TEXTO EXTENSO` usan `malloc`/`free` con tamaños calculados dinámicamente, eliminando límites artificiales de buffer.
- **Conversión ASCII directa en `CARACTERATEXTO`**: Ahora evalúa el primer argumento como expresión numérica (ej: `CARACTERATEXTO(72, $c)` produce `'H'`), con fallback a primer carácter para literales.
- **Encabezados de documentación uniformes**: Todos los archivos `.h` y `.c` ahora incluyen metadata estandarizada (`@file`, `@author`, `@ai_assist`, `@description`) para trazabilidad profesional.

### 🔧 Corregido
- **Desbordamiento de pila de `SI` en recursión profunda**: Implementada limpieza de contexto (`si_runtime_ptr = ctx->si_ptr_inicio`) en `cmd_retornar`, resolviendo el fallo en funciones recursivas como Ackermann y Fibonacci.
- **Parsing de operadores infijos**: Corregida la condición de ruptura de tokens para reconocer el espacio (`' '`) como delimitador válido, permitiendo expresiones como `$a BITY $b` y `$x MOD 10`.
- **Validación de retorno en funciones `nico_*`**: Corregida la validación de `== 0` a `!= -1` en `nico_concatenar_texto`, `nico_reemplazar_texto`, `nico_extraer_texto`, etc., ya que estas funciones retornan la longitud del string resultante, no un código de estado.
- **Extracción robusta de nombres en funciones de cadena**: Reemplazado `limpiar_nombre()` por extracción manual que salta correctamente el `$` y toma solo caracteres alfanuméricos, evitando falsos negativos en búsquedas.
- **Funciones de texto no reconocían `TEXTO EXTENSO`**: Agregada ruta de búsqueda con `buscar_texto_extenso()` como fallback en todos los comandos de cadena cuando `buscar_texto_var()` falla.
- **Inicialización ignorada en `TEXTO EXTENSO`**: `procesar_declaracion_variable_texto_extenso()` ahora procesa la cláusula `= "valor"` y asigna el contenido mediante `asignar_texto_extenso_valor()`.
- **`CARACTERATEXTO` con argumentos numéricos**: Corregido el comportamiento que tomaba el primer carácter del string `"72"` (produciendo `'7'`) en lugar de evaluar el número 72 como código ASCII (produciendo `'H'`).
- **Funciones con espacio antes del paréntesis**: Corregido el parser de expresiones para permitir sintaxis `FUNCION ($var)` mediante técnica de "peek" (mirar hacia adelante) que distingue entre operador infijo y llamada a función.
- **Funciones obsoletas eliminadas**: Removidas `evaluar_termino_desde_ptr`, `push_retorno`, `set_retorno` y `pop_retorno` que generaban warnings de compilación.

### 🔄 Mejorado
- **Arquitectura de búsqueda en funciones de cadena**: Patrón unificado "buscar texto normal → fallback a TEXTO EXTENSO → fallback a scope local" para consistencia en los 12 comandos de cadena.
- **Gestión de memoria dinámica**: Uso de `malloc` con tamaños calculados según la operación (ej: `strlen(valor_actual) + strlen(origen) + 1` para concatenación), evitando buffers de tamaño fijo innecesarios.
- **Limpieza de warnings de compilación**: Eliminados todos los warnings de GCC (`-Wunused-function`, `-Wunused-variable`, `-Wunused-but-set-variable`) en módulos `expressions.c`, `funciones.c` y `declaraciones.c`.
- **Contexto de ejecución en funciones**: Nuevo campo `si_ptr_inicio` en `CtxBloque` para guardar el estado inicial de la pila de `SI` al entrar a una función, permitiendo restauración limpia al retornar.
- **Evaluador de expresiones unificado**: Reemplazados los dos bucles separados de operadores aritméticos por un único bucle recursivo con `evaluar_expresion_completa()`, simplificando la lógica y mejorando la precedencia.

### 📚 Documentación
- 📝 **`CHANGELOG.md`**: Actualizado con entrada detallada de v1.1.1 documentando fixes críticos de recursión, operadores infijos y soporte TEXTO EXTENSO.
- 📝 **Encabezados de archivos**: Estandarizado el formato de metadata en `web.h`, `win_compat.h` y módulos principales con créditos de autoría y descripción funcional.
- 📝 **Notas de arquitectura**: Documentado el patrón de "limpieza de pila de SI en RETORNAR" como solución al desbordamiento en recursión profunda.

### 🖥️ Compatibilidad Verificada
| Plataforma | Compilador | Estado | Notas |
|------------|-----------|--------|-------|
| **Linux** (Ubuntu/Debian) | GCC 9+, x86_64/ARM64 | ✅ Compilación limpia | Sin warnings |
| **Windows 10/11** | MinGW-w64, x86_64 | ✅ Compilación limpia | VT100/WinAPI compatible |
| **Raspberry Pi 4/5** | GCC, Pi OS Bookworm | ✅ En validación | GPIO + TEXTO EXTENSO |

### 🧪 Tests de Validación
- ✅ `TestTorturaExtrema.nico`: Ackermann(3,2)=29, Fibonacci(30)=832040, Factorial(15), operadores binarios, anidamiento 3x3x3.
- ✅ `TestCadenasCompleto.nico`: Todas las 19 funciones de cadena operando correctamente sobre `TEXTO` y `TEXTO EXTENSO`.
- ✅ `TestMiniCadenas.nico`: Validación de `CARACTERATEXTO(72) → 'H'`, `CONCATENARTEXTO` y `LONGITUDTEXTO` sobre `TEXTO EXTENSO`.

### 👥 Créditos
- **Diseño, Arquitectura y Supervisión:** Diego Alejandro Majluff
- **Implementación, Debugging y Optimización:** Qwen (Alibaba Cloud)
- **Licencia:** MIT / Uso Educativo

> 💡 **Nota:** Nico v1.1.1 es un release de estabilización crítica que corrige bugs fundamentales en el motor de ejecución (recursión profunda, operadores infijos) y completa el soporte de memoria dinámica para `TEXTO EXTENSO` en todas las operaciones de cadena. Esta versión establece la base sólida para el desarrollo de features avanzadas en v2.0.0.

---

## [1.1.0] - 2026-06-04

### ✅ Agregado
- **Soporte nativo para GPIO en Raspberry Pi**: Comandos `CONFIGURARPIN`, `ESTADOPIN`, `LEERPIN` con abstracción `libgpiod`.
- **Modos de bias interno**: `PULLUP` y `PULLDOWN` para evitar lecturas flotantes y proteger pines en lectura de switches.
- **Parser GPIO extendido**: Ahora acepta variables (`$pin_led`) además de literales numéricos en todos los comandos GPIO.
- **Sintaxis explícita de configuración**: `CONFIGURARPIN(pin, ENTRADA, PULLUP)` para mayor claridad pedagógica.
- **Shadowing completo para LISTA y MATRIZ** en los 14 tipos de datos: variables locales con mismo nombre que globales se aíslan correctamente al entrar/salir de funciones.
- **Soporte de índices en funciones de texto**: `LONGITUDTEXTO($lista[indice])`, `BUSCARTEXTO($mat[f][c], ...)` con interpolación correcta en `MOSTRAR`/`ESCRIBIR`.
- **Corrección de aspect ratio en gráficos ASCII**: ajuste empírico (`factor_y = 0.5`) para círculos visuales en terminales; paso angular optimizado (`0.10` rad).
- **Formato `DECIMALES()` con matrices**: Respeta precisión especificada cuando se escapan corchetes literales (`d\[0\]\[0\] = $m[0][0]`).
- **Validación de comillas en inicialización de CARACTER**: Error explícito si se omite `' '` en `LISTA CARACTER` o `MATRIZ CARACTER`.
- **Documentación técnica modular**: 19 archivos `/docs/` con índice maestro (`MANUAL.md`) y navegación relativa.

### 🔧 Corregido
- **Parser de expresiones GPIO**: `ESTADOPIN($var, SI/NO)` y `CONFIGURARPIN($var, MODO)` ahora resuelven variables correctamente mediante `parsear_numero_pin_o_variable()`.
- **Constantes `SI`/`NO` restringidas a `ESTADOPIN`**: Para evitar conflictos de evaluación en expresiones lógicas (uso de `0`/`1` en condiciones `SI()`).
- **Búsqueda en scopes locales**: Normalización de nombres (`$var` → `var`) antes de `buscar_*_local()` para evitar falsos negativos.
- **Separación de pools decimales**: `DECIMAL` (tipo `1`) y `DECIMAL SIN SIGNO` (tipo `3`) ahora usan pools independientes, evitando colisiones de asignación.
- **Orden de búsqueda en `COPIARTEXTO`**: Prioriza scope local antes de fallback global, garantizando shadowing correcto para `LISTA TEXTO`.
- **Interpolación en `procesar_escribir()`**: Matrices y listas de texto ahora respetan shadowing al imprimir con `MOSTRAR`.
- **Parsing de índices en funciones de texto**: Soporte completo para `$lista[indice]` y `$mat[f][c]`.
- **Warnings de compilación en GCC/Linux**: Eliminados `-Wunused-parameter`, `-Wmisleading-indentation`, `-Wsequence-point`, `-Wunused-variable` en módulos C.
- **Indentación engañosa en bucles de parsing**: Agregadas llaves `{}` explícitas en `cadenas.c` para claridad y portabilidad.

### 🔄 Mejorado
- **Refactorización de `procesar_gpio_configurar`**: Soporte para sintaxis explícita `CONFIGURARPIN(pin, ENTRADA, PULLUP/PULLDOWN)` con validación de rangos.
- **Compilación condicional con stub** (`nico_gpio_stub.c`): Compatibilidad Windows/Linux sin hardware GPIO (comandos son no-ops seguros).
- **Arquitectura de scopes**: Refactorización de `buscar_*_local()` y `set_*_valor()` para consistencia en los 14 tipos.
- **Manejo de nombres con `$`**: Patrón `clean_name = (nombre[0]=='$') ? nombre+1 : nombre` aplicado uniformemente en búsquedas.
- **Estructura de proyecto**: Carpetas `src/`, `examples/`, `docs/` listas para GitHub; `.gitignore` optimizado para C cross-platform.
- **Ejemplos de validación**: `TestGPIO_Completo.nico`, `TestShadowingTexto.nico`, `TestShadowingMatriz2.nico` incluidos para regression testing.

### 📚 Documentación
- 📝 `19-gpio-raspberry.md`: Referencia completa, diagramas de conexión segura, patrón `PULLUP` para switches y troubleshooting.
- 📝 `18-dibujarcirculo.md`: Parámetros, sistema de coordenadas 1-based, corrección de aspect ratio y equivalencias multiplataforma.
- 📝 `MANUAL.md`: Índice maestro con navegación rápida por categorías y búsqueda por comando.
- 📝 **Nota sobre corchetes en `MOSTRAR` y `ESCRIBIR`**: Los corchetes `[...]` sin escapar se interpretan como placeholders; usar `\[` y `\]` para literales.
- 📝 **Filosofía de asignación**: Escalares requieren `ASIGNAR EN`/`CALCULAR EN`; colecciones permiten asignación directa por pragmatismo.
- 📝 **`DECIMAL SIN SIGNO`**: Restricción de signo es semántica (dominio positivo), almacenamiento interno `double` (IEEE 754).
- 📝 **Límites prácticos**: `MAX_TEXTO_LEN=4096`, `MAX_LISTA=1024`, `MAX_DIMENSION=64` documentados en referencia de tipos.
- 📝 **Notas de seguridad GPIO**: Advertencia explícita sobre cortocircuitos y uso obligatorio de `PULLUP` para switches conectados a masa.

### 🖥️ Compatibilidad Verificada
| Plataforma | Compilador | GPIO | Notas |
|------------|-----------|------|-------|
| **Linux** (Ubuntu/Debian) | GCC 9+, x86_64/ARM64 | ✅ `libgpiod` | Native ANSI terminal |
| **Windows 10/11** | MinGW-w64, x86_64 | ⚪ Stub (no-op) | VT100/WinAPI compatible |
| **Raspberry Pi 4/5** | GCC, Pi OS Bookworm | ✅ `libgpiod ≥1.6` | GPIO validado en hardware |
| **macOS** | Clang, Xcode CLI | ⚪ Stub (no-op) | Homebrew `sqlite3` opcional |
| **Terminales** | — | — | GNOME, Windows Terminal, VS Code Integrated, Pi Framebuffer |

### 👥 Créditos
- **Diseño, Arquitectura y Supervisión:** Diego Alejandro Majluff
- **Implementación, Debugging y Optimización:** Qwen (Alibaba Cloud)
- **Licencia:** MIT / Uso Educativo

> 💡 **Nota:** Nico v1.1.0 estabiliza el sistema de scopes y tipos, añade la primera capa de abstracción de hardware nativa (GPIO) y completa la documentación modular. La próxima versión (`v1.2.0`) planifica errores didácticos, `INCLUIR`, precedencia matemática nativa y tipo `BOOLEANA`.

---

## [1.0.1] - 2026-05-07

### ✅ Agregado
- Flag `-e` para evaluación rápida de expresiones desde terminal (`./nico -e "2+2"`).
- Función nativa `SIGMOIDE(x)` para educación en Machine Learning básico.
- Ejemplo educativo: `ejemplos/ml/TestPerceptronSimple.nico` (Perceptrón aprendiendo puerta AND).
- Restauración automática de eco y configuración de terminal post-ejecución.

### 🔧 Corregido
- Fuga de contador estático en evaluador de expresiones con recursión inline.
- Desalineación de scope/contexto post-`RETORNAR` en funciones anidadas.
- Terminal quedaba en estado "sin eco" al salir abruptamente (Linux/Windows).
- Caracteres UTF-8 (`ñ`, `á`, `é`) cortados en `VARIABLE CARACTER` → solucionado mediante `VARIABLE TEXTO` + `LEER()`.
- Múltiples warnings de compilador (`-Wunused-variable`, `-Wsequence-point`) limpiados.

### 🔄 Mejorado
- Optimización del loop principal de parsing para mayor estabilidad en modo REPL.
- Capa `win_compat.h` refinada para compilación limpia en MinGW-w64 (Windows 10/11).
- Gestión de `TEXTO EXTENSO` validada y documentada (solo scope global, heap dinámico).
- Validador estructural pre-ejecución más robusto contra parámetros malformados.

### 📚 Documentación
- 📝 **Nota crítica de precedencia:** Nico v1.0.1 evalúa expresiones **estrictamente de izquierda a derecha**. Se documentó el uso obligatorio de paréntesis explícitos para orden matemático estándar (ej: `2 + (3 * 4)`).
- Límites del lenguaje (recursión, buffers, scopes) expuestos en modo interactivo y README.
- Créditos de asistencia IA integrados en encabezados de código y metadata del proyecto.

### 🖥️ Compatibilidad Verificada
- ✅ Linux (GCC, x86_64 / ARM64)
- ✅ Windows 10/11 (MinGW-w64, x86_64)
- ✅ Raspberry Pi OS (ARMv7/ARMv8, GPIO opcional)

### 👥 Créditos
- **Diseño, Arquitectura y Supervisión:** Diego Alejandro Majluff
- **Implementación, Debugging y Optimización:** Qwen (Alibaba Cloud)
- **Licencia:** MIT / Uso Educativo

> 💡 **Nota:** Este release prioriza estabilidad, claridad pedagógica y compatibilidad multiplataforma. La precedencia matemática nativa se planificó como feature principal para `v1.1.0`.

---
📚 *Documentación validada con Nico v2.0.0. Ejemplos probados en Linux, Windows y Raspberry Pi OS.*