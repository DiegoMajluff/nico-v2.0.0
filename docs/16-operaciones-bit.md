# 🔢 Operaciones Bit-a-Bit

Funciones para manipulación directa de bits en valores enteros. Ideales para flags, máscaras, criptografía básica, comunicación serial, embebidos y optimizaciones de bajo nivel.

> 💡 **Requisito**: Todas estas funciones operan sobre `ENTERA` o `ENTERA SIN SIGNO`. Usar `DECIMAL` puede causar truncamiento o comportamiento inesperado. Se recomienda `ENTERA SIN SIGNO` para evitar signos en desplazamientos.

---

## 🔄 Rotaciones de Bits (Circular Shift)

| Función | Sintaxis | Ejemplo | Resultado | Nota |
|---------|----------|---------|-----------|------|
| `ROTARIZQUIERDA` | `ROTARIZQUIERDA($valor, $bits)` | `ROTARIZQUIERDA(255, 4)` | `255` (0xFF → 0xFF) | Rota bits hacia izquierda, los que salen por izquierda entran por derecha |
| `ROTARDERECHA` | `ROTARDERECHA($valor, $bits)` | `ROTARDERECHA(255, 4)` | `255` (0xFF → 0xFF) | Rota bits hacia derecha, los que salen por derecha entran por izquierda |

> 📌 **Rotación vs Desplazamiento**: En rotación, ningún bit se pierde. En desplazamiento, los bits que salen se descartan y entran ceros.

---

## ➡️ Desplazamientos de Bits (Logical Shift)

| Función | Sintaxis | Ejemplo | Resultado | Nota |
|---------|----------|---------|-----------|------|
| `DESPLAZARIZQUIERDA` | `DESPLAZARIZQUIERDA($valor, $bits)` | `DESPLAZARIZQUIERDA(1, 8)` | `256` (1 << 8) | Equivale a multiplicar por 2^bits. Ceros entran por derecha. |
| `DESPLAZARDERECHA` | `DESPLAZARDERECHA($valor, $bits)` | `DESPLAZARDERECHA(256, 4)` | `16` (256 >> 4) | Equivale a dividir por 2^bits (truncando). Ceros entran por izquierda. |

---

## 🔀 Operaciones Lógicas Bit-a-Bit

| Función | Sintaxis | Ejemplo | Resultado | Descripción |
|---------|----------|---------|-----------|-------------|
| `BITY` | `BITY($a, $b)` | `BITY(255, 15)` | `15` | AND bit-a-bit: 1 solo si ambos bits son 1 |
| `BITO` | `BITO($a, $b)` | `BITO(240, 15)` | `255` | OR bit-a-bit: 1 si al menos un bit es 1 |
| `BITXOR` | `BITXOR($a, $b)` | `BITXOR(255, 128)` | `127` | XOR bit-a-bit: 1 si los bits difieren |
| `BITNO` | `BITNO($valor)` | `BITNO(255)` | `...1111111100000000` | NOT bit-a-bit: invierte todos los bits |

> ⚠️ **Nota sobre `BITNO`**: El resultado depende del tamaño de palabra (32-bit en Nico). `BITNO(255)` en 32-bit = `4294967040` (0xFFFFFF00), no `-256`.

---

## 🎯 Manipulación de Bits Individuales

| Función | Sintaxis | Ejemplo | Resultado | Descripción |
|---------|----------|---------|-----------|-------------|
| `LEERBIT` | `LEERBIT($valor, $posicion)` | `LEERBIT(255, 3)` | `1` | Retorna `1` si el bit en `$posicion` está activo, `0` si no |
| `ACTIVARBIT` | `ACTIVARBIT($valor, $posicion)` | `ACTIVARBIT(0, 5)` | `32` | Fuerza el bit en `$posicion` a `1`, retorna nuevo valor |
| `DESACTIVARBIT` | `DESACTIVARBIT($valor, $posicion)` | `DESACTIVARBIT(255, 5)` | `223` | Fuerza el bit en `$posicion` a `0`, retorna nuevo valor |

> 📌 **Posición de bits**: `$posicion = 0` es el bit menos significativo (LSB, valor 1). `$posicion = 7` es el bit de valor 128, etc.

---

## 🧪 Ejemplo Mínimo Validado
```nico
PROGRAMA TestBits
    VARIABLE ENTERA SIN SIGNO $x = 0
BLOQUE PRINCIPAL
    ESCRIBIR("\n=== Operaciones Bit-a-Bit ===") SALTO
    
    // Rotaciones
    CALCULAR EN $x = ROTARIZQUIERDA(255, 4)
    ESCRIBIR("ROTARIZQUIERDA(255, 4) = $x") SALTO
    
    CALCULAR EN $x = ROTARDERECHA(255, 4)
    ESCRIBIR("ROTARDERECHA(255, 4) = $x") SALTO
    
    // Desplazamientos
    CALCULAR EN $x = DESPLAZARIZQUIERDA(1, 8)
    ESCRIBIR("DESPLAZARIZQUIERDA(1, 8) = $x") SALTO
    
    CALCULAR EN $x = DESPLAZARDERECHA(256, 4)
    ESCRIBIR("DESPLAZARDERECHA(256, 4) = $x") SALTO
    
    // Operaciones lógicas
    CALCULAR EN $x = BITY(255, 15)
    ESCRIBIR("BITY(255, 15) = $x") SALTO
    
    CALCULAR EN $x = BITO(240, 15)
    ESCRIBIR("BITO(240, 15) = $x") SALTO
    
    CALCULAR EN $x = BITXOR(255, 128)
    ESCRIBIR("BITXOR(255, 128) = $x") SALTO
    
    CALCULAR EN $x = BITNO(255)
    ESCRIBIR("BITNO(255) = $x") SALTO
    
    // Manipulación de bits
    CALCULAR EN $x = LEERBIT(255, 3)
    ESCRIBIR("LEERBIT(255, 3) = $x") SALTO
    
    CALCULAR EN $x = ACTIVARBIT(0, 5)
    ESCRIBIR("ACTIVARBIT(0, 5) = $x") SALTO
    
    CALCULAR EN $x = DESACTIVARBIT(255, 5)
    ESCRIBIR("DESACTIVARBIT(255, 5) = $x") SALTO
    
    ESCRIBIR("Test de bits completado ✓\n") SALTO
FIN PRINCIPAL
FINAL
```

---

## ⚠️ Notas Técnicas Críticas
- **Tamaño de palabra:** Nico usa 32-bit para operaciones bit-a-bit. `BITNO(0)` retorna `4294967295` (0xFFFFFFFF), no `-1`.
- **ENTERA SIN SIGNO recomendado:** Los desplazamientos en valores con signo pueden tener comportamiento definido por implementación. Usá `SIN SIGNO` para predictibilidad.
- **Posiciones válidas:** `$posicion` debe estar entre `0` y `31`. Valores fuera de rango causan comportamiento indefinido.
- **Rotación con `$bits >= 32`:** El comportamiento es modular (`$bits % 32`). `ROTARIZQUIERDA($v, 33)` ≡ `ROTARIZQUIERDA($v, 1)`.
- **Performance:** Estas funciones se compilan a instrucciones CPU nativas (`rol`, `ror`, `shl`, `shr`, `and`, `or`, `xor`, `not`). Cero overhead.

---

## 🪟 Compatibilidad Windows/Linux

| Aspecto | Comportamiento |
|---------|---------------|
| Implementación | Instrucciones nativas de CPU (x86/ARM) vía C |
| Tamaño de palabra | 32-bit consistente en ambas plataformas |
| Signo en desplazamientos | Usar `ENTERA SIN SIGNO` para comportamiento predecible |
| Performance | Idéntica: operaciones bit-a-bit son O(1) en hardware |

> 💡 **Tip para embebidos:** Para máscaras de hardware, definí constantes con nombres descriptivos: `VARIABLE ENTERA SIN SIGNO $MASK_LED = 0x01, $MASK_BTN = 0x02`. Luego usá `BITY($registro, $MASK_LED)` para leer estados.