# ⌨️ Entrada de Usuario y Formato de Salida

Comandos para capturar datos desde `stdin` y controlar la precisión decimal al imprimir valores numéricos. Fundamentales para scripts interactivos, validación de entrada y generación de reportes legibles.

> 💡 **Nota**: La entrada es bloqueante por defecto. El formateo con `DECIMALES` afecta únicamente la salida visual; no modifica el valor real en memoria.

---
## 🔤 Lectura Bloqueante

| Comando | Sintaxis | Comportamiento |
|---------|----------|---------------|
| `LEER` | `LEER($variable)` | Espera a que el usuario ingrese datos y presione `Enter`. Convierte automáticamente según el tipo declarado. Si la conversión numérica falla, asigna `0`. |
| `LEERCARACTER` | `LEERCARACTER($variable)` | Lee un solo carácter sin esperar `Enter`. Consume el siguiente carácter disponible en el buffer. |
| `LEERHASTA` | `LEERHASTA($variable, "delimitador")` | Lee texto hasta encontrar el delimitador especificado. Acumula múltiples líneas. |

### 📋 Parámetros

| Parámetro | Tipo esperado | Descripción |
|-----------|--------------|-------------|
| `$variable` | `ENTERA`, `DECIMAL`, `TEXTO`, `CARACTER` | Destino del valor leído. Debe estar declarada previamente. |
| `"delimitador"` | `TEXTO` (solo para LEERHASTA) | Cadena que marca el fin de la lectura. |

---

## 🔢 Formato de Precisión: DECIMALES

| Uso | Sintaxis | Regla |
|-----|----------|-------|
| Impresión | `ESCRIBIR("$a $b", DECIMALES($p1, $p2))` | `$p1` aplica a la 1ª variable numérica, `$p2` a la 2ª. Ignora `TEXTO`. Orden posicional estricto. |
| Valores válidos | `0` a `~15` | Redondeo estándar (half-up). No modifica memoria, solo salida. |

---

## 🧪 Ejemplo Mínimo Validado
```nico
PROGRAMA EntradaYFormato
    VARIABLE ENTERA $edad = 0
    VARIABLE DECIMAL $precio = 19.567, $iva = 0.0
BLOQUE PRINCIPAL
    ESCRIBIR("\nIngresá tu edad: ")
    LEER($edad)

    ASIGNAR EN $iva = $precio * 0.21
    ESCRIBIR("Precio base: $precio") SALTO
    ESCRIBIR("IVA (21%): $iva", DECIMALES(2)) SALTO
    ESCRIBIR("Edad ingresada: $edad") SALTO
FIN PRINCIPAL
FINAL
```
## 🧪 Ejemplos Adicionales

### LEERCARACTER - Lectura de un solo carácter

    VARIABLE CARACTER $opcion = ''
    ESCRIBIR("Presioná una tecla: ")
    LEERCARACTER($opcion)
    ESCRIBIR("Presionaste: $opcion") SALTO

### LEERHASTA - Lectura hasta delimitador

    VARIABLE TEXTO $texto = ""
    ESCRIBIR("Ingresá texto (escribí FIN para terminar):") SALTO
    LEERHASTA($texto, "FIN")
    ESCRIBIR("Texto capturado:") SALTO
    ESCRIBIR("$texto") SALTO

---

## ⚠️ Notas Técnicas Críticas

- **Conversión silenciosa:** `LEER` no lanza excepciones. Si el usuario escribe `"abc"` en una `VARIABLE ENTERA`, se asigna `0`. Validá con rangos o `TEXTOVACIO()` si necesitás robustez.
- **LEERCARACTER:** Lee inmediatamente sin esperar `Enter`. Consume el siguiente carácter disponible.
- **LEERHASTA:** Acumula texto hasta el delimitador. El delimitador no se incluye en el resultado.
- **Orden posicional estricto:** `DECIMALES` cuenta solo variables numéricas en el string. `ESCRIBIR("Nota: $n", DECIMALES(2))` aplica a `$n`.
- **Persistencia:** `DECIMALES` es un modificador de `ESCRIBIR`/`MOSTRAR`. No puede usarse fuera de estas llamadas.
- **Buffer de entrada:** `LEER` limpia automáticamente el `\n` final. En bucles consecutivos, no requiere flush manual.
- **Enteras con decimales:** Si aplicás `DECIMALES(2)` a `$x = 5`, se imprime `5.00`. El valor interno sigue siendo entero.
---

## 🎯 Buenas Prácticas
```nico
// ✅ Validar entrada numérica básica
LEER($nota)
SI($nota MENOR 0 O $nota MAYOR 10) ENTONCES
    ESCRIBIR("⚠️ Valor fuera de rango") SALTO
FIN SI

// ✅ Alinear columnas numéricas con precisión fija
ESCRIBIR("$prod1 $precio1", DECIMALES(0, 2)) SALTO
ESCRIBIR("$prod2 $precio2", DECIMALES(0, 2)) SALTO

// ✅ Combinar con TEXTO EXTENSO para logs formateados
VARIABLE TEXTO EXTENSO $log = "Transacción: $monto"
// ... luego imprimir con precisión controlada ...
ESCRIBIR("$log", DECIMALES(2)) SALTO

// ❌ Evitar aplicar DECIMALES a cadenas o expresiones complejas
ESCRIBIR("$nombre $edad", DECIMALES(2))  // ⚠️ Solo aplica a $edad
```

---

## 🔄 Equivalencias en Otros Lenguajes

| Nico | C | Python | JavaScript |
|------|---|--------|------------|
| `LEER($v)` | `scanf()` / `fgets()` + `strtol` | `input()` + `int()`/`float()` | `prompt()` o `readline` |
| `ESCRIBIR("$x", DECIMALES(2))` | `printf("%.2f", x)` | `f"{x:.2f}"` | `x.toFixed(2)` |

> 💡 **Compatibilidad:** `LEER` usa `getline`/`fgets` bajo el hood, compatible con Linux y Windows. `DECIMALES` traduce a `printf` format specifiers en C.