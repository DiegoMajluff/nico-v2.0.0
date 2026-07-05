# 🔤 Funciones de Cadenas (TEXTO)

## 📏 Información y Consulta
| Función | Sintaxis | Ejemplo | Retorna / Nota |
|---------|----------|---------|---------------|
| LONGITUDTEXTO | LONGITUDTEXTO($texto) | $len = LONGITUDTEXTO($nombre) | ENTERA: cantidad de caracteres |
| BUSCARTEXTO | BUSCARTEXTO($texto, $busqueda) | $pos = BUSCARTEXTO($completo, "Nico") | ENTERA: posición (0-based) o -1. Case-insensitive. |
| BUSCARCARACTER | BUSCARCARACTER($texto, 'c') | $pos = BUSCARCARACTER($nombre, 'i') | ENTERA: posición del carácter o -1. |
| COMPARARTEXTO | COMPARARTEXTO($t1, $t2) | $res = COMPARARTEXTO($a, $b) | ENTERA: 0 si iguales, !=0 si difieren. Case-sensitive. |
| TEXTOVACIO | TEXTOVACIO($texto) | $es_vacio = TEXTOVACIO($var) | ENTERA: 1 si vacío, 0 si tiene contenido. |

## ✏️ Modificación In-Place
| Procedimiento | Sintaxis | Ejemplo | Nota |
|--------------|----------|---------|------|
| COPIARTEXTO | COPIARTEXTO($destino, $origen) | COPIARTEXTO($completo, $nombre) | Copia contenido. $destino declarado. |
| CONCATENARTEXTO | CONCATENARTEXTO($texto, $agregar) | CONCATENARTEXTO($completo, " Mundo") | Agrega al final. |
| MAYUSCULAS | MAYUSCULAS($texto) | MAYUSCULAS($completo) | Convierte in-place. |
| MINUSCULAS | MINUSCULAS($texto) | MINUSCULAS($completo) | Convierte in-place. |
| RECORTARTEXTO | RECORTARTEXTO($texto) | RECORTARTEXTO($con_espacios) | Elimina espacios inicio/final. |
| REEMPLAZARTEXTO | REEMPLAZARTEXTO($texto, $buscar, $reemplazar) | REEMPLAZARTEXTO($completo, "mundo", "Nico") | Case-insensitive. Reemplaza todas. |
| INVERTIRTEXTO | INVERTIRTEXTO($texto, $destino) | INVERTIRTEXTO($original, $invertida) | Invierte byte por byte. $destino declarado. |

## 🔀 Extracción y División
| Procedimiento | Sintaxis | Ejemplo | Nota |
|--------------|----------|---------|------|
| EXTRAERTEXTO | EXTRAERTEXTO($origen, $inicio, $longitud, $destino) | EXTRAERTEXTO($completo, 0, 4, $sub) | Substring. Inicio 0-based. |
| DIVIDIRTEXTO | DIVIDIRTEXTO($texto, $delimitador, $indice, $destino) | DIVIDIRTEXTO($csv, ',', 2, $parte) | Obtiene parte $indice. |
| REPETIRTEXTO | REPETIRTEXTO($texto, $veces, $destino) | REPETIRTEXTO("AB", 3, $res) | Repite y guarda en $destino. |

## 🔢 Conversión de Tipos
| Función / Proc. | Sintaxis | Ejemplo | Retorna / Nota |
|----------------|----------|---------|--------------|
| TEXTOAENTERO | TEXTOAENTERO($texto) | $n = TEXTOAENTERO("123") | ENTERA. Si falla, 0. |
| ENTEROATEXTO | ENTEROATEXTO($numero, $destino) | ENTEROATEXTO(133, $txt) | void en $destino. |
| TEXTOADECIMAL | TEXTOADECIMAL($texto) | $d = TEXTOADECIMAL("3.14") | DECIMAL. Si falla, 0.0. |
| DECIMALATEXTO | DECIMALATEXTO($decimal, $destino) | DECIMALATEXTO(3.14, $txt) | void en $destino. |
| TEXTOACARACTER | TEXTOACARACTER($texto) | $ascii = TEXTOACARACTER("A") | ENTERA: ASCII primer carácter. |
| CARACTERATEXTO | CARACTERATEXTO($codigo, $destino) | CARACTERATEXTO(65, $txt) | void: 1 carácter en $destino. |

📌 Notas: In-Place vs Retorno, Case-Insensitivity en búsqueda/reemplazo, índices 0-based. INVERTIRTEXTO invierte byte por byte (cuidado con UTF-8).

---


# 🔢 Funciones Matemáticas

## 📐 Constantes
| Función | Sintaxis | Retorna |
|---------|----------|---------|
| NUMEROPI | NUMEROPI() | DECIMAL: π ≈ 3.14159 |
| NUMEROEULER | NUMEROEULER() | DECIMAL: e ≈ 2.71828 |
| RAIZDEUNMEDIO | RAIZDEUNMEDIO() | DECIMAL: √0.5 ≈ 0.7071 |
| LOGNATURALDE2 | LOGNATURALDE2() | DECIMAL: ln(2) ≈ 0.6931 |
| LOGNATURALDE10 | LOGNATURALDE10() | DECIMAL: ln(10) ≈ 2.3025 |

## 🧮 Básicas
| Función | Sintaxis | Retorna / Nota |
|---------|----------|---------------|
| RAIZ | RAIZ($x) | Raíz cuadrada |
| RAIZCUBICA | RAIZCUBICA($x) | Raíz cúbica |
| POTENCIA | POTENCIA($base, $expo) | $base ^ $expo |
| MODULO | MODULO($a, $b) | Resto de $a / $b |
| ABSOLUTO | ABSOLUTO($x) | Valor absoluto |

## 📐 Trigonométricas (radianes)
| Función | Sintaxis | Retorna |
|---------|----------|---------|
| SENO | SENO($rad) | Seno |
| COSENO | COSENO($rad) | Coseno |
| TANGENTE | TANGENTE($rad) | Tangente |
| ARCOSENO | ARCOSENO($x) | Arcoseno [-1,1] |
| ARCOCOSENO | ARCOCOSENO($x) | Arcocoseno [-1,1] |
| ARCOTANGENTE | ARCOTANGENTE($x) | Arcotangente |

## 📊 Logarítmicas y Exponenciales
| Función | Sintaxis | Retorna |
|---------|----------|---------|
| LOGNATURAL | LOGNATURAL($x) | ln(x) |
| LOGBASE10 | LOGBASE10($x) | log₁₀(x) |
| LOGBASE2 | LOGBASE2($x) | log₂(x) |
| LOGARITMO | LOGARITMO($x, $base) | log_base(x) |
| EXPONENCIAL | EXPONENCIAL($x) | e^x |
| DOSALAX | DOSALAX($x) | 2^x |

## 🔢 Redondeo, Comparación y Operadores
| Función | Sintaxis | Retorna / Nota |
|---------|----------|---------------|
| REDONDEAR | REDONDEAR($modo, $valor) | ARRIBA, ABAJO, ENTERO |
| QUITARDECIMAL | QUITARDECIMAL($x) | Trunca hacia cero |
| MAXIMO | MAXIMO($a, $b) | Mayor valor |
| MINIMO | MINIMO($a, $b) | Menor valor |
| + - * / ^ | $a op $b | Operadores aritméticos estándar |

📌 Notas: Radianes obligatorios en trig. CALCULAR EN y RESULTADO EN son alias. ESCRIBIR(..., DECIMALES(n)) formatea salida.

---


# 🔀 Control de Flujo

## 🔀 SEGUN CASO

    SEGUN CASO ($variable)
        CASO 1
            // código
            CORTE
        CASO 2
            // código
            CORTE
        POR DEFECTO
            // fallback
            CORTE
    FIN SEGUN

- CORTE obligatorio por caso (evita fall-through).
- POR DEFECTO opcional.
- Soporta anidamiento ilimitado. Cada nivel requiere su FIN SEGUN.

## 🔁 REALIZAR...MIENTRAS (Do-While)

    REALIZAR
        // se ejecuta al menos 1 vez
    MIENTRAS ($condición)

- Evalúa condición al final.
- No lleva FIN MIENTRAS (el MIENTRAS cierra el bloque).
- Ideal para menús y validación de entrada.

---


# 🖥️ Sistema y Control de Consola

## ⏱️ Temporización y Shell
| Comando | Sintaxis | Ejemplo | Nota |
|---------|----------|---------|------|
| ESPERAR | ESPERAR(valor, UNIDAD) | ESPERAR(500, MILISEGUNDOS) | Unidades: MICROS/US, MILISEGUNDOS/MS, SEGUNDOS/S, MINUTOS/MIN. Case-insensitive. |
| SISTEMA | SISTEMA("comando") | SISTEMA("ls -l") | Ejecuta comando del SO. Captura stdout en terminal. Comillas obligatorias. |

## 📐 Información y Cursor
| Comando | Sintaxis | Retorna / Acción |
|---------|----------|-----------------|
| ANCHOTERMINAL | ANCHOTERMINAL($destino) | Columnas visibles (ENTERA) |
| ALTOTERMINAL | ALTOTERMINAL($destino) | Filas visibles (ENTERA) |
| CURSOR | CURSOR($fila, $col) | Posiciona cursor (1-based) |
| LIMPIARPANTALLA | LIMPIARPANTALLA | Borra pantalla + cursor a (1,1) |
| OCULTARCURSOR | OCULTARCURSOR | Oculta cursor (sin args) |
| MOSTRARCURSOR | MOSTRARCURSOR | Muestra cursor (sin args) |
| LEERTECLA | LEERTECLA($destino) | Captura 1 tecla sin Enter. Retorna código ASCII/byte. |

⚠️ LEERTECLA configura terminal en modo raw/cbreak automáticamente. En bucles, agregar ESPERAR(10-50, MS) para evitar CPU al 100%.

---


# 🎨 Colores y Estilos de Texto

## 🌈 Colores
| Comando | Sintaxis | Colores válidos (minúsculas, sin comillas) |
|---------|----------|-------------------------------------------|
| COLORTEXTO | COLORTEXTO(color) | negro, rojo, verde, amarillo, azul, magenta, cyan, blanco |
| COLORFONDO | COLORFONDO(color) | Mismos que COLORTEXTO |
| RESETCOLOR | RESETCOLOR | Restablece colores por defecto |

## ✨ Estilos
| Comando | Sintaxis | Efecto |
|---------|----------|--------|
| TEXTONEGRITA | TEXTONEGRITA | Peso fuerte (bold) |
| TEXTOCURSIVA | TEXTOCURSIVA | Itálico (soporte variable) |
| TEXTOSUBRAYADO | TEXTOSUBRAYADO | Subrayado simple |
| RESETTEXTO | RESETTEXTO | Quita estilos, no afecta colores |

📌 Regla de oro: RESETCOLOR y RESETTEXTO son independientes. Para volver al estado original, usá ambos. Los estilos/colores persisten hasta reset explícito.

---


# 🎨 Gráficos ASCII y Relleno

## 📏 Primitivas de Dibujo
| Procedimiento | Sintaxis | Nota |
|--------------|----------|------|
| DIBUJARLINEA | DIBUJARLINEA($x1, $y1, $x2, $y2, $char) | Algoritmo de Bresenham. $char 1 carácter. |
| DIBUJARCIRCULO | DIBUJARCIRCULO($cx, $cy, $radio, $char) | Aproximación paramétrica. Fuente monoespaciada recomendada. |
| RELLENARRECTANGULO | RELLENARRECTANGULO($x1, $y1, $x2, $y2, $char) | Rellena área incluida. Ideal para fondos/paneles. |

📌 Coordenadas 1-based (columna, fila). Si $x2 < $x1 o $y2 < $y1, comportamiento indefinido. Validar antes de llamar.

---


# 🕒 Fecha y Hora

| Comando | Sintaxis | Formato de Salida |
|---------|----------|------------------|
| HORAACTUAL | HORAACTUAL($destino) | "HH:MM:SS" (24h) |
| FECHAACTUAL | FECHAACTUAL($destino) | "DD/MM/AAAA" |

📌 Requieren VARIABLE TEXTO declarada. Escriben por referencia. Hora local del sistema (sin TZ/UTC en v2.0.0).

---


# ⌨️ Entrada y Formato de Salida

## 🔤 Lectura de Usuario
| Comando | Sintaxis | Comportamiento |
|---------|----------|---------------|
| LEER | LEER($variable) | Bloqueante hasta Enter. Conversión automática según tipo declarado (ENTERA, DECIMAL, TEXTO, CARACTER). Si falla conversión numérica → 0. |

## 🔢 Formato DECIMALES
| Uso | Sintaxis | Regla |
|-----|----------|-------|
| Impresión | ESCRIBIR("$a $b $c", DECIMALES($p1, $p2, $p3)) | $p1 aplica a 1ª variable numérica, $p2 a 2ª, etc. Ignora TEXTO. Orden posicional estricto. |
| Valores válidos | 0 a ~15 | Redondeo estándar (half-up). No modifica valor en memoria, solo salida. |

✅ ESCRIBIR("Nota: $n", DECIMALES(2)) → imprime con 2 decimales.
❌ DECIMALES fuera de ESCRIBIR → error de sintaxis.

---


# 📁 Manejo de Archivos

## 📋 Comandos y Modos
| Comando | Sintaxis | Nota |
|---------|----------|------|
| ABRIRARCHIVO | ABRIRARCHIVO($archivo, "ruta", MODO) | $archivo debe ser VARIABLE ARCHIVO. MODO es constante sin comillas. |
| ESCRIBIRARCHIVO | ESCRIBIRARCHIVO($archivo, $texto) | Agrega \n automáticamente al final. |
| LEERARCHIVO | LEERARCHIVO($archivo, $destino) | Lee 1 línea hasta \n. Remueve salto final. $destino es TEXTO. |
| CERRARARCHIVO | CERRARARCHIVO($archivo) | Libera descriptor. Hace flush implícito. Obligatorio. |

| Modo (Nico) | Equivalente C | Comportamiento |
|-------------|--------------|---------------|
| LECTURA | "r" | Solo lectura. Debe existir. |
| ESCRITURA | "w" | Escritura pura. Trunca si existe. Crea si no. |
| AGREGAR | "a" | Append al final. No trunca. Crea si no. |
| LECTOESCRITURA | "r+" | Lectura/escritura. Debe existir. |

📌 Rutas: Relativas al directorio de ejecución. Usar / para portabilidad Win/Linux.

---


# 🔁 Funciones con Retorno

## 📐 Sintaxis

    FUNCION <TIPO_RETORNO> nombre($param1, $param2, ...)
        VARIABLE <TIPO> $local = valor
        CALCULAR EN $resultado = <expresión>  // O RESULTADO EN (alias)
        RETORNAR $resultado  // Obligatorio: devuelve el valor
    FIN FUNCION

## 📋 Tipos Válidos

| Tipo | Rango Aprox. | Uso |
|------|-------------|-----|
| ENTERA | ±2.1×10⁹ | Contadores, índices |
| DECIMAL | ±1.7×10³⁰⁸ | Cálculos, promedios |
| LOGICA | VERDADERO/FALSO | Flags, condiciones |
| TEXTO | Dinámico | Cadenas UTF-8 |
| CARACTER | 0-255 | Caracteres individuales |
| ENTERA SIN SIGNO | 0 a 4.2×10⁹ | IDs, tamaños |
| DECIMAL SIN SIGNO | 0 a +1.7×10³⁰⁸ | Magnitudes físicas |

📌 RESULTADO EN es alias de CALCULAR EN (solo asigna). Solo RETORNAR devuelve el valor de la función. Parámetros por valor. Scope local para variables declaradas dentro.

---


# 🔧 Subprogramas

## 📐 Sintaxis

    SUBPROGRAMA Nombre($param1, $param2, ...)
        VARIABLE <TIPO> $local = valor  // Opcional
        // Opera sobre variables globales del PROGRAMA
        // Sin retorno
    FIN SUBPROGRAMA

    // Llamada
    LLAMAR A Nombre(valor1, valor2, ...)

## 📋 Características

| Característica | Comportamiento | Nota |
|----------------|----------------|------|
| Parámetros | ✅ Sí (por valor) | Se pasan entre paréntesis |
| Retorno | ❌ No retorna valor | Es un procedimiento |
| Scope | 🌐 Global + parámetros | Accede/modifica variables del PROGRAMA |
| Variables locales | ✅ Sí | Podés declarar VARIABLE dentro |
| Llamada | LLAMAR A Nombre(args) | Con paréntesis y argumentos |

📌 Diferencia clave: FUNCION = calcula y retorna (con RETORNAR). SUBPROGRAMA = ejecuta acciones sin retornar.

---


# 📦 Sistema de Inclusión (INCLUIR)

## 📐 Sintaxis

    PROGRAMA MiPrograma
        INCLUIR "ruta/al/archivo.nico"
        INCLUIR "otra_lib.nico"
        
        VARIABLE ENTERA $contador = 0
    BLOQUE PRINCIPAL
        // Código principal
    FIN PRINCIPAL
    FINAL

## 📋 Características

| Característica | Comportamiento | Nota |
|----------------|----------------|------|
| Ubicación | Dentro de PROGRAMA, antes de BLOQUE PRINCIPAL | No dentro de funciones o bloques |
| Orden | Secuencial, de arriba hacia abajo | Los archivos se procesan en orden |
| Scope | Variables globales accesibles | Respetan reglas normales de scope |
| Funciones/Subprogramas | Se cargan y están disponibles | Pueden llamarse desde BLOQUE PRINCIPAL |
| Anidamiento | ✅ Soportado | Un archivo incluido puede tener sus propios INCLUIR |
| Detección de duplicados | ✅ Prevención automática | Evita redefiniciones |

## 🎯 Buenas Prácticas

    // ✅ Organización por responsabilidad
    INCLUIR "operaciones.nico"      // Solo funciones matemáticas
    INCLUIR "utilidades.nico"       // Funciones auxiliares
    INCLUIR "interfaz.nico"         // Funciones de UI

    // ✅ Agrupar includes al inicio
    PROGRAMA MiApp
        INCLUIR "constantes.nico"
        INCLUIR "libreria.nico"
        
        VARIABLE ENTERA $estado = 0
    BLOQUE PRINCIPAL
        // Código
    FIN PRINCIPAL
    FINAL

📌 Las rutas son relativas al directorio donde se ejecuta el programa. Se recomienda usar rutas relativas simples para mantener portabilidad.

---


# 🏗️ Estructura Base y Sintaxis

## 📐 Esqueleto Mínimo

    PROGRAMA NombreDelPrograma
        INCLUIR "libreria.nico"  // Opcional
        
        // Declaraciones globales (fuera de bloques)
        VARIABLE ENTERA $contador = 0
        VARIABLE TEXTO $mensaje = "Hola"
        VARIABLE LOGICA $activo = VERDADERO
        LISTA LOGICA $flags[10]
        MATRIZ LOGICA $tablero[8][8]

    SUBPROGRAMA MiSub($param)
        // ...
    FIN SUBPROGRAMA

    FUNCION ENTERA MiFuncion($a, $b)
        VARIABLE ENTERA $resultado = 0
        CALCULAR EN $resultado = $a + $b
        RETORNAR $resultado
    FIN FUNCION

    BLOQUE PRINCIPAL
        // Lógica ejecutable
        ESCRIBIR("Inicio") SALTO
        LLAMAR A MiSub(42)
        CALCULAR EN $resultado = MiFuncion(2, 3)
    FIN PRINCIPAL
    FINAL

## 📋 Reglas Fundamentales

| Concepto | Regla |
|----------|-------|
| PROGRAMA | Nombre único. Sin extensión .nico en el código. |
| FINAL | Cierra obligatoriamente el archivo. |
| BLOQUE PRINCIPAL | Punto de entrada. Obligatorio. Todo código ejecutable debe ir aquí o en sub/funciones. |
| VARIABLE | Declaración explícita. Tipos: ENTERA, DECIMAL, TEXTO, CARACTER, LOGICA, ARCHIVO, y sus variantes SIN SIGNO. |
| LISTA / MATRIZ | Arrays 1D y 2D. Soportan todos los tipos incluyendo LOGICA. |
| SALTO | Equivalente a \n. Opcional tras ESCRIBIR. |
| Comentarios | // comentario de línea. Nico no admite comentarios en bloque. |

📌 Asignación: CALCULAR EN $var = valor, RESULTADO EN $var = valor (alias válidos). RETORNAR devuelve valor de función.

---


# 🔌 Control de GPIO (Raspberry Pi)

Comandos para control de pines digitales y PWM en Raspberry Pi usando libgpiod.

## 📌 Comandos Disponibles

| Comando | Sintaxis | Descripción |
|---------|----------|-------------|
| CONFIGURARPIN | CONFIGURARPIN(pin, direccion, bias) | Configura un pin como entrada/salida con resistencia pull opcional |
| ESTADOPIN | ESTADOPIN(pin, valor) | Escribe un valor digital (SI/NO) en un pin de salida |
| LEERPIN | LEERPIN(pin, $destino) | Lee el estado digital de un pin y lo guarda en una variable |
| GENERARPWM | GENERARPWM(pin, frecuencia, duty_cycle) | Genera señal PWM en un pin |
| DETENERPWM | DETENERPWM(pin) | Detiene la señal PWM en un pin |

## 📋 Parámetros Válidos

### CONFIGURARPIN(pin, direccion, bias)

| Parámetro | Valores Válidos | Descripción |
|-----------|----------------|-------------|
| pin | Número entero | Número de pin BCM (ej: 17, 27, 22) |
| direccion | ENTRADA, SALIDA | Dirección del pin |
| bias | PULLUP, PULLDOWN, o sin especificar | Resistencia pull interna (opcional) |

**Ejemplos:**

    // Configurar pin 17 como salida
    CONFIGURARPIN(17, SALIDA)
    
    // Configurar pin 27 como entrada con pull-up
    CONFIGURARPIN(27, ENTRADA, PULLUP)
    
    // Configurar pin 22 como entrada con pull-down
    CONFIGURARPIN(22, ENTRADA, PULLDOWN)

### ESTADOPIN(pin, valor)

| Parámetro | Valores Válidos | Descripción |
|-----------|----------------|-------------|
| pin | Número entero | Número de pin BCM |
| valor | SI, NO | SI = HIGH (3.3V), NO = LOW (0V) |

**Ejemplos:**

    // Encender LED en pin 17
    ESTADOPIN(17, SI)
    
    // Apagar LED en pin 17
    ESTADOPIN(17, NO)

### LEERPIN(pin, $destino)

| Parámetro | Tipo | Descripción |
|-----------|------|-------------|
| pin | Número entero | Número de pin BCM |
| $destino | VARIABLE LOGICA | Variable donde se guarda el valor leído (SI/NO) |

**Ejemplo:**

    VARIABLE LOGICA $estado = NO
    LEERPIN(27, $estado)
    SI($estado IGUAL SI) ENTONCES
        ESCRIBIR("Botón presionado") SALTO
    FIN SI

### GENERARPWM(pin, frecuencia, duty_cycle)

| Parámetro | Tipo | Rango | Descripción |
|-----------|------|-------|-------------|
| pin | Número entero | - | Número de pin BCM |
| frecuencia | Número | > 0 | Frecuencia en Hz |
| duty_cycle | Número | 0-100 | Ciclo de trabajo en porcentaje |

**Ejemplos:**

    // PWM al 50% a 1kHz para controlar servo
    GENERARPWM(18, 1000, 50)
    
    // PWM al 25% a 500Hz para dimmer LED
    GENERARPWM(18, 500, 25)

### DETENERPWM(pin)

| Parámetro | Tipo | Descripción |
|-----------|------|-------------|
| pin | Número entero | Número de pin BCM |

**Ejemplo:**

    DETENERPWM(18)

## 🎯 Ejemplo Completo: Control de LED con Botón

    PROGRAMA ControlLED
        VARIABLE LOGICA $boton = NO
        VARIABLE LOGICA $led = NO
        
    BLOQUE PRINCIPAL
        // Configurar pines
        CONFIGURARPIN(27, ENTRADA, PULLUP)  // Botón con pull-up
        CONFIGURARPIN(17, SALIDA)           // LED como salida
        
        ESCRIBIR("Presiona el botón para encender el LED") SALTO
        
        MIENTRAS VERDADERO HACER
            LEERPIN(27, $boton)
            
            SI($boton IGUAL NO) ENTONCES  // Pull-up: NO = presionado
                ESTADOPIN(17, SI)         // Encender LED
                ESCRIBIR("LED encendido") SALTO
            SINO
                ESTADOPIN(17, NO)         // Apagar LED
            FIN SI
            
            ESPERAR(50, MILISEGUNDOS)     // Debounce
        FIN MIENTRAS
    FIN PRINCIPAL
    FINAL

## ⚠️ Notas Importantes

- **Numeración BCM:** Usa numeración BCM de Raspberry Pi (no física)
- **Requiere permisos:** Ejecutar con sudo o agregar usuario al grupo gpio
- **Solo Raspberry Pi:** Estos comandos solo funcionan en Raspberry Pi con libgpiod instalado
- **PWM por hardware:** Solo algunos pines soportan PWM por hardware (ej: GPIO 18)
- **Voltaje:** Los pines operan a 3.3V (no 5V)

## 🔗 Pines PWM Recomendados

| Pin BCM | Función PWM | Notas |
|---------|-------------|-------|
| 18 | PWM0 | Soporta PWM por hardware |
| 12 | PWM0 | Alternativa |
| 13 | PWM1 | Alternativa |
| 19 | PWM1 | Alternativa |

📌 Para otros pines, se usa PWM por software (menos preciso pero funcional).

---


# 🗄️ Base de Datos SQLite

Nico soporta operaciones con bases de datos SQLite integradas. Permite crear, consultar y modificar bases de datos directamente desde scripts.

## 📌 Comandos Disponibles

| Comando | Sintaxis | Descripción |
|---------|----------|-------------|
| CONECTARBD | CONECTARBD("ruta.db") | Abre o crea una base de datos SQLite |
| EJECUTARBD | EJECUTARBD("SQL", param1, param2, ...) | Ejecuta comandos SQL sin resultado (INSERT, UPDATE, DELETE, CREATE) |
| CONSULTARBD | CONSULTARBD("SQL", param1, param2, ...) | Ejecuta consulta SQL con resultado (SELECT) |
| SIGUIENTEFILABD | SIGUIENTEFILABD() | Avanza al siguiente registro. Retorna VERDADERO/FALSO |
| CERRARCONSULTABD | CERRARCONSULTABD() | Cierra el cursor de consulta activo |
| CERRARBD | CERRARBD() | Cierra la conexión a la base de datos |

## 📋 Uso Básico

### Conectar y Crear Tabla

    PROGRAMA DemoBD
        VARIABLE TEXTO $nombre = ""
        VARIABLE ENTERA $edad = 0
        
    BLOQUE PRINCIPAL
        // Conectar a base de datos (crea el archivo si no existe)
        CONECTARBD("usuarios.db")
        
        // Crear tabla
        EJECUTARBD("CREATE TABLE IF NOT EXISTS usuarios (id INTEGER PRIMARY KEY, nombre TEXT, edad INTEGER)")
        
        // Insertar datos
        EJECUTARBD("INSERT INTO usuarios (nombre, edad) VALUES (?, ?)", "Diego", 45)
        EJECUTARBD("INSERT INTO usuarios (nombre, edad) VALUES (?, ?)", "Ana", 32)
        
        // Consultar datos
        CONSULTARBD("SELECT nombre, edad FROM usuarios WHERE edad > ?", 30)
        
        MIENTRAS(SIGUIENTEFILABD()) HACER
            // Los valores se cargan en $BDCOL1, $BDCOL2, etc.
            COPIARTEXTO($nombre, $BDCOL1)
            CALCULAR EN $edad = $BDCOL2
            ESCRIBIR("Nombre: $nombre, Edad: $edad") SALTO
        FIN MIENTRAS
        
        CERRARCONSULTABD()
        CERRARBD()
    FIN PRINCIPAL
    FINAL

## 🔍 Variables de Resultado

Cuando ejecutas CONSULTARBD, los valores de cada columna se cargan automáticamente en variables especiales:

| Variable | Tipo | Descripción |
|----------|------|-------------|
| $BDCOL1 | Variable | Valor de la primera columna |
| $BDCOL2 | Variable | Valor de la segunda columna |
| $BDCOL3 | Variable | Valor de la tercera columna |
| ... | ... | ... |
| $BDCOLn | Variable | Valor de la n-ésima columna |

**Ejemplo:**

    CONSULTARBD("SELECT nombre, edad, email FROM usuarios")
    MIENTRAS(SIGUIENTEFILABD()) HACER
        ESCRIBIR("Nombre: $BDCOL1") SALTO
        ESCRIBIR("Edad: $BDCOL2") SALTO
        ESCRIBIR("Email: $BDCOL3") SALTO
        ESCRIBIR("---") SALTO
    FIN MIENTRAS
    CERRARCONSULTABD()

## 🔄 Parámetros con ?

Usá ? como placeholder para valores dinámicos. Esto evita inyección SQL y maneja correctamente tipos de datos:

    // ✅ Correcto: usar parámetros
    EJECUTARBD("INSERT INTO productos (nombre, precio) VALUES (?, ?)", "Teclado", 1500.50)
    
    // ❌ Evitar: concatenar strings
    EJECUTARBD("INSERT INTO productos (nombre, precio) VALUES ('Teclado', 1500.50)")

## 🎯 Ejemplo Completo: Sistema de Inventario

    PROGRAMA Inventario
        VARIABLE TEXTO $producto = ""
        VARIABLE DECIMAL $precio = 0.0
        VARIABLE ENTERA $cantidad = 0
        VARIABLE ENTERA $total = 0
        
    FUNCION ENTERA contar_productos()
        VARIABLE ENTERA $count = 0
        CONSULTARBD("SELECT COUNT(*) FROM productos")
        SI(SIGUIENTEFILABD()) ENTONCES
            CALCULAR EN $count = $BDCOL1
        FIN SI
        CERRARCONSULTABD()
        RETORNAR $count
    FIN FUNCION

    BLOQUE PRINCIPAL
        CONECTARBD("inventario.db")
        
        // Crear tabla
        EJECUTARBD("CREATE TABLE IF NOT EXISTS productos (id INTEGER PRIMARY KEY AUTOINCREMENT, nombre TEXT, precio REAL, cantidad INTEGER)")
        
        // Agregar productos
        EJECUTARBD("INSERT INTO productos (nombre, precio, cantidad) VALUES (?, ?, ?)", "Mouse", 500.0, 10)
        EJECUTARBD("INSERT INTO productos (nombre, precio, cantidad) VALUES (?, ?, ?)", "Teclado", 1500.0, 5)
        EJECUTARBD("INSERT INTO productos (nombre, precio, cantidad) VALUES (?, ?, ?)", "Monitor", 8000.0, 3)
        
        // Actualizar stock
        EJECUTARBD("UPDATE productos SET cantidad = cantidad - 1 WHERE nombre = ?", "Mouse")
        
        // Consultar inventario completo
        ESCRIBIR("=== Inventario ===") SALTO
        CONSULTARBD("SELECT nombre, precio, cantidad FROM productos ORDER BY nombre")
        
        CALCULAR EN $total = 0
        MIENTRAS(SIGUIENTEFILABD()) HACER
            COPIARTEXTO($producto, $BDCOL1)
            CALCULAR EN $precio = $BDCOL2
            CALCULAR EN $cantidad = $BDCOL3
            ESCRIBIR("$producto - Precio: $precio - Stock: $cantidad") SALTO
            CALCULAR EN $total = $total + ($precio * $cantidad)
        FIN MIENTRAS
        CERRARCONSULTABD()
        
        ESCRIBIR("Valor total del inventario: $total") SALTO
        ESCRIBIR("Total de productos: ") SALTO
        ESCRIBIR(contar_productos()) SALTO
        
        CERRARBD()
    FIN PRINCIPAL
    FINAL

## ⚠️ Notas Importantes

- **Orden de cierre:** Siempre cerrar CONSULTARBD antes de CERRARBD
- **Transacciones:** SQLite maneja transacciones automáticamente
- **Tipos de datos:** SQLite usa tipado dinámico, pero Nico convierte automáticamente
- **Rutas relativas:** Las rutas de archivos son relativas al directorio de ejecución
- **Concurrencia:** SQLite maneja bloqueos automáticamente

## 📊 Operaciones SQL Comunes

| Operación | Ejemplo |
|-----------|---------|
| CREATE TABLE | EJECUTARBD("CREATE TABLE tabla (id INTEGER, nombre TEXT)") |
| INSERT | EJECUTARBD("INSERT INTO tabla VALUES (?, ?)", 1, "test") |
| UPDATE | EJECUTARBD("UPDATE tabla SET nombre = ? WHERE id = ?", "nuevo", 1) |
| DELETE | EJECUTARBD("DELETE FROM tabla WHERE id = ?", 1) |
| SELECT | CONSULTARBD("SELECT * FROM tabla WHERE id > ?", 0) |
| COUNT | CONSULTARBD("SELECT COUNT(*) FROM tabla") |
| SUM | CONSULTARBD("SELECT SUM(cantidad) FROM productos") |
| AVG | CONSULTARBD("SELECT AVG(precio) FROM productos") |

---


# 🌐 Servidor Web Embebido

Nico incluye un servidor HTTP integrado con panel de administración, renderizado dinámico y API JSON. No requiere frameworks externos.

## 📌 Comandos Disponibles

| Comando | Sintaxis | Descripción |
|---------|----------|-------------|
| INICIARSERVER | INICIARSERVER(puerto) | Inicia servidor HTTP en el puerto especificado (default: 8080) |
| DETENERSERVER | DETENERSERVER() | Detiene el servidor web |

## 📂 Endpoints Disponibles

| Ruta | Método | Función |
|------|--------|---------|
| /admin | GET | Panel SPA de administración con CRUD completo |
| /render?archivo=ruta | GET | Ejecuta script .nico y devuelve su salida como HTML |
| /api/tablas | GET | Lista JSON de todas las tablas en la BD conectada |
| /api/schema?tabla=X | GET | Estructura JSON de la tabla X (columnas, tipos, PKs, FKs) |

## 🎯 Ejemplo Básico

    PROGRAMA ServidorWeb
        VARIABLE TEXTO $mensaje = ""
        
    BLOQUE PRINCIPAL
        // Conectar a base de datos (opcional)
        CONECTARBD("datos.db")
        
        // Iniciar servidor en puerto 8080
        INICIARSERVER(8080)
        ESCRIBIR("Servidor activo en http://localhost:8080") SALTO
        
        // Mantener programa activo
        MIENTRAS(1 IGUAL 1) HACER
            ESPERAR(1, SEGUNDOS)
        FIN MIENTRAS
        
        // Limpiar recursos (nunca se alcanza en este ejemplo)
        DETENERSERVER()
        CERRARBD()
    FIN PRINCIPAL
    FINAL

## 🔐 Panel de Administración (/admin)

- **Acceso:** http://localhost:8080/admin
- **Credenciales:** Usuario `admin` | Clave `nico2026`
- **Características:**
  - CRUD completo (Crear, Leer, Editar, Eliminar registros)
  - Búsqueda por texto libre y filtros por columna
  - Paginación automática
  - Import/Export de datos en formato CSV
  - Detección automática de Foreign Keys y relaciones visuales

El panel se conecta automáticamente a la BD abierta con CONECTARBD().

## 🎨 Renderizado Dinámico (/render)

Permite generar páginas HTML desde scripts Nico:

**Archivo: pagina.nico**

    ESCRIBIR("<!DOCTYPE html><html><head>") SALTO
    ESCRIBIR("<meta charset='utf-8'>") SALTO
    ESCRIBIR("<style>body{font-family:sans-serif;padding:30px}</style>") SALTO
    ESCRIBIR("</head><body>") SALTO
    ESCRIBIR("<h1>Reporte Generado</h1>") SALTO
    ESCRIBIR("<p>Hola desde Nico!</p>") SALTO
    ESCRIBIR("</body></html>") SALTO

**Acceder:** http://localhost:8080/render?archivo=pagina.nico

⚠️ **Reglas críticas para /render:**
- NO incluyas INICIARSERVER en el script renderizado (causa conflicto de puerto)
- Usá ESCRIBIR para todo el HTML/CSS/JS
- Las rutas se resuelven desde el directorio donde ejecutaste nico

## 📡 API JSON (/api/...)

Diseñada para integración con frontend externo:

    # Listar tablas
    curl http://localhost:8080/api/tablas
    
    # Ver esquema de una tabla
    curl http://localhost:8080/api/schema?tabla=usuarios

**Respuesta típica:**

    {
      "tablas": ["usuarios", "productos", "ventas"],
      "schema": {
        "id": "INTEGER PRIMARY KEY",
        "nombre": "TEXT",
        "email": "TEXT UNIQUE"
      }
    }

## ⚠️ Notas Importantes

- **Puerto por defecto:** 8080 (cambiable con INICIARSERVER(puerto))
- **Concurrencia:** Usa select() + hilos POSIX para manejo ligero de peticiones
- **Sin dependencias:** Todo corre en un solo binario
- **Uso educativo:** Diseñado para prototipado rápido, no para producción bajo carga
- **Seguridad:** El endpoint /render solo acepta rutas de archivos locales

---

# 📋 Compatibilidad y Arquitectura

## 🖥️ Plataformas Soportadas

| Plataforma | Estado | Notas |
|------------|--------|-------|
| Linux (x86/x64) | ✅ Completo | Todas las funcionalidades |
| Raspberry Pi | ✅ Completo | Incluye GPIO, PWM, SQLite, Web |
| Windows (x86/x64) | ✅ Completo | Compatible con consola Windows |
| macOS | ⚠️ Experimental | Compilación manual requerida |

## 🏗️ Arquitectura AST

Nico v2.0.0 usa un **Abstract Syntax Tree (AST)** en lugar de interpretación línea por línea:

- **Parser:** Convierte código fuente en árbol de nodos AST
- **Evaluator:** Recorre el AST y ejecuta cada nodo
- **Scope management:** Gestión de variables globales y locales con resolución de scope
- **Recursión:** Soporta hasta 7000 niveles de recursión
- **Manejo de errores:** Reporta línea y contexto del error

## 🔧 Tipos de Datos Soportados

| Tipo | Tamaño | Rango | Uso |
|------|--------|-------|-----|
| ENTERA | 32-bit | ±2.1×10⁹ | Contadores, índices |
| DECIMAL | 64-bit | ±1.7×10³⁰⁸ | Cálculos científicos |
| LOGICA | 1 byte | VERDADERO/FALSO | Flags, condiciones |
| TEXTO | Dinámico | Limitado por RAM | Cadenas UTF-8 |
| CARACTER | 1 byte | 0-255 | Caracteres individuales |
| ARCHIVO | Puntero | - | Manejo de archivos |
| ENTERA SIN SIGNO | 32-bit | 0 a 4.2×10⁹ | IDs, tamaños |
| DECIMAL SIN SIGNO | 64-bit | 0 a +1.7×10³⁰⁸ | Magnitudes físicas |

## 📦 Estructuras de Datos

| Estructura | Sintaxis | Tamaño Máximo | Uso |
|------------|----------|---------------|-----|
| LISTA | LISTA TIPO $nombre[tamaño] | 1024 elementos | Arrays 1D |
| MATRIZ | MATRIZ TIPO $nombre[filas][columnas] | 64x64 | Arrays 2D |

Ambas soportan todos los tipos incluyendo LOGICA.

## 🎯 Resumen de Características

✅ **Lenguaje en español** con sintaxis intuitiva
✅ **Tipado fuerte** con conversión automática segura
✅ **Funciones y subprogramas** con scope local/global
✅ **Control de flujo** completo (SI/SEGUN/MIENTRAS/PARA)
✅ **Manejo de archivos** con modos LECTURA/ESCRITURA/AGREGAR
✅ **Base de datos SQLite** integrada con CRUD completo
✅ **Servidor web** con panel admin y API JSON
✅ **GPIO y PWM** para Raspberry Pi
✅ **Gráficos ASCII** con primitivas de dibujo
✅ **Colores y estilos** de terminal
✅ **Sistema de inclusión** para modularización
✅ **Portabilidad** Linux/Windows/Raspberry Pi

---

> 📚 **Documentación validada con Nico v2.0.0**
> 
> Ejemplos probados en Linux, Raspberry Pi y Windows.
> 
> Última actualización: Julio 2026
