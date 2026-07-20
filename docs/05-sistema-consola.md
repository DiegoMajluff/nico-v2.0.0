# 🖥️ Sistema y Control de Consola

Comandos para gestionar temporización, ejecución de shell, información de terminal, control de cursor, y entrada de datos (puntual, continua y multilínea).

---

## ⏱️ Temporización y Ejecución Externa

| Comando | Sintaxis | Ejemplo | Nota |
|---------|----------|---------|------|
| `ESPERAR` | `ESPERAR(valor, UNIDAD)` | `ESPERAR(500, MILISEGUNDOS)` | Unidades: `MICROS`/`US`, `MILISEGUNDOS`/`MS`, `SEGUNDOS`/`S`, `MINUTOS`/`MIN`. Case-insensitive. |
| `SISTEMA` | `SISTEMA("comando")` | `SISTEMA("ls -l")` | Ejecuta comando del SO. Comillas obligatorias. Output va a stdout de la consola. |
| `TIEMPOMS` | `TIEMPOMS($destino)` | | Lee el tiempo actual del sistema en **milisegundos** y lo guarda en `$destino`. **No bloquea**. Retorna timestamp absoluto (epoch o boot time). |

> ⚠️ `SISTEMA` es bloqueante hasta que el proceso hijo termine. Útil para invocar utilidades externas, pero evitá loops tight por overhead de fork.

---

## 📐 Información y Control de Terminal

| Comando | Sintaxis | Retorna / Acción |
|---------|----------|-----------------|
| `ANCHOTERMINAL` | `ANCHOTERMINAL($destino)` | Columnas visibles (`ENTERA`) |
| `ALTOTERMINAL` | `ALTOTERMINAL($destino)` | Filas visibles (`ENTERA`) |
| `CURSOR` | `CURSOR($fila, $col)` | Posiciona cursor (1-based) |
| `LIMPIARPANTALLA` | `LIMPIARPANTALLA` | Borra pantalla + cursor a (1,1) |
| `OCULTARCURSOR` | `OCULTARCURSOR` | Oculta cursor (sin argumentos) |
| `MOSTRARCURSOR` | `MOSTRARCURSOR` | Muestra cursor (sin argumentos) |

> 💡 `ANCHOTERMINAL` y `ALTOTERMINAL` actualizan sus valores si el usuario redimensiona la ventana. Consultalos antes de dibujar UIs adaptables.

---

⌨️ Entrada en Tiempo Real (No Bloqueante)
| Comando|Sintaxis|Comportamiento|
| ---|---|---|
| LEERTECLA|LEERTECLA($destino)|Captura 1 tecla sin esperar  Enter . Retorna código ASCII o código especial para teclas de función. No bloquea si no hay tecla disponible (devuelve 0).|

🎯 **Códigos de Teclas Especiales**:
| Tecla|Código|Tecla|Código|
| ---|---|---|---|
| Flecha Arriba|1001|Flecha Abajo|1002|
| Flecha Derecha|1003|Flecha Izquierda|1004|
| Home|1005|End|1006|
| Insert|1007|Supr (Delete)|1008|
| Page Up|1009|Page Down|1010|

💡 **Uso en Juegos**: `LEERTECLA` es ideal para juegos y UIs reactivas. El buffer se limpia automáticamente al inicio de cada iteración de `MIENTRAS`, permitiendo detectar teclas presionadas en cada frame sin acumulación histórica.

⚠️ **Notas Técnicas**:
- `LEERTECLA` consume la tecla del buffer (una sola lectura por tecla presionada)
- En bucles `MIENTRAS`, el buffer se limpia automáticamente en cada iteración
- Para teclas ASCII (a-z, 0-9), usá el código decimal (97='a', 100='d', 113='q')
- Para flechas y teclas especiales, usá los códigos 1001-1010
- Ctrl+C (código 3) interrumpe el programa automáticamente
---

## 📝 Entrada Multilínea: LEERHASTA

Comando bloqueante que acumula entrada desde `stdin` hasta encontrar un delimitador exacto. Ideal para pegar bloques de texto, scripts o configuraciones multilineales sin interrupciones por `Enter`.

### 📐 Sintaxis
```nico
LEERHASTA($destino, "delimitador")
```

| Parámetro | Tipo | Descripción | Ejemplo |
|-----------|------|-------------|---------|
| `$destino` | `VARIABLE TEXTO` | Recibe todo el texto acumulado, incluyendo saltos de línea | `$entrada`, `$script` |
| `"delimitador"` | TEXTO (literal entre comillas) | Cadena exacta que detiene la lectura. Case-sensitive. | `"EOF"`, `"FIN"`, `"/end"` |

### 🧪 Ejemplo Mínimo Validado
```nico
PROGRAMA leer_hasta
    VARIABLE TEXTO $entrada = ""
BLOQUE PRINCIPAL
    ESCRIBIR("\nIngresá el texto y al final escribí EOF para finalizar: ") SALTO
    LEERHASTA($entrada, "EOF") // Insertá EOF en una nueva línea de texto.
    ESCRIBIR("") SALTO
    ESCRIBIR("\nEl texto que ingresaste es el siguiente: ") SALTO
    ESCRIBIR("$entrada\n") SALTO
FIN PRINCIPAL
FINAL
```

### ⚠️ Notas Técnicas Críticas
- **Bloqueante hasta delimitador:** La ejecución se pausa hasta que el usuario escribe la cadena exacta y presiona Enter. Enter intermedios se acumulan.
- **Coincidencia exacta:** `"EOF"` ≠ `"eof"` ≠ `" EOF "`. El delimitador debe coincidir carácter por carácter, sin espacios implícitos.
- **Manejo de memoria:** Si usás `TEXTO EXTENSO` como destino, el buffer crece dinámicamente. Con `TEXTO` fijo, asegurá que el tamaño declarado sea suficiente para evitar truncamiento.
- **No consume el delimitador:** La cadena `"EOF"` no se incluye en `$destino`. Solo se acumula el texto anterior.
- **Buffer de terminal:** La línea se envía al runtime solo al presionar Enter. Escribí `EOF` en línea separada. Es lo conveniente para un buen comportamiento.

### 🎯 Buenas Prácticas
```nico
// ✅ Usar delimitadores únicos y poco comunes
LEERHASTA($config, "###FIN_CONFIG###")

// ✅ Limpiar destino antes de reusar en bucles
COPIARTEXTO($entrada, "")
LEERHASTA($entrada, "FIN")

// ✅ Validar longitud tras lectura para evitar procesamiento vacío
LEERHASTA($datos, "EOF")
SI(LONGITUDTEXTO($datos) IGUAL 0) ENTONCES
    ESCRIBIR("⚠️ No se ingresaron datos") SALTO
FIN SI
```

### 🔄 Equivalencias y Compatibilidad

| Nico | C Runtime | Python | Bash |
|------|-----------|--------|------|
| `ESPERAR(50, MS)` | `nanosleep()` / `Sleep()` | `time.sleep(0.05)` | `sleep 0.05` |
| `TIEMPOMS($t)` | `clock_gettime()` / `GetTickCount()` | `time.time() * 1000` | `date +%s%3N` |
| `SISTEMA("cmd")` | `system("cmd")` | `os.system("cmd")` | `` `cmd` `` |
| `LEERTECLA($t)` | `getch()` / `_getch()` | `sys.stdin.read(1)` | `read -n1` |
| `LEERHASTA($v, "D")` | Loop `fgets` + `strstr` | `sys.stdin.read().split("D")[0]` | `read -d 'D' var` |

> 📌  Windows/Linux:  Todos los comandos usan fallbacks nativos.  `SISTEMA`  ejecuta  `cmd.exe`  en Win y  `/bin/sh`  en Linux.  `LEERTECLA`  usa  `conio.h`  (Win) o  `termios`  (Linux). Comportamiento lógico idéntico.

### 🧪 Ejemplo Mínimo Validado: Medir Duración de Código (TIEMPOMS)
```nico
PROGRAMA MedirTiempo
    VARIABLE DECIMAL $inicio, $fin, $duracion
BLOQUE PRINCIPAL
    TIEMPOMS($inicio)  // Capturar timestamp inicial
    
    // Código a perfilar
    VARIABLE ENTERA $suma = 0
    PARA($i DESDE 1 HASTA 10000) HACER
        CALCULAR EN $suma = $suma + $i
    FIN PARA
    
    TIEMPOMS($fin)  // Capturar timestamp final
    CALCULAR EN $duracion = ($fin - $inicio) / 1000.0  // Convertir ms → segundos
    
    ESCRIBIR("Operación completada en $duracion segundos") SALTO
FIN PRINCIPAL
FINAL
```