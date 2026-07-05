# 🟢 DIBUJARCIRCULO: Referencia Técnica

Comando nativo para renderizar el perímetro de un círculo en la consola mediante aproximación angular. Ideal para UIs, menús, juegos simples o visualización de datos en terminal.

## 📐 Firma y Sintaxis
```nico
DIBUJARCIRCULO($centro_x, $centro_y, $radio, $caracter)
```

## 🔍 Desglose de Parámetros

| Parámetro | Tipo Esperado | Qué Representa | Cómo lo Interpreta Nico | Notas y Recomendaciones |
|-----------|---------------|----------------|-------------------------|-------------------------|
| `$centro_x` | `ENTERA` o `DECIMAL` | **Columna** (eje X) del centro | `1` = borde izquierdo. Crece hacia la derecha. | Equivale al **segundo** parámetro de `CURSOR($fila, $col)`. Para centrado horizontal: `$ancho / 2`. |
| `$centro_y` | `ENTERA` o `DECIMAL` | **Fila** (eje Y) del centro | `1` = borde superior. Crece hacia abajo. | Equivale al **primer** parámetro de `CURSOR($fila, $col)`. Para centrado vertical: `$alto / 2`. |
| `$radio` | `ENTERA` o `DECIMAL` | Distancia en **celdas** desde el centro al perímetro | Debe ser `> 0`. Se usa en `x = cx + r·cos(θ)` e `y = cy + r·sen(θ)`. | Valores `< 2` pueden no renderizar. Valores muy grandes pueden salir de pantalla sin recorte automático. |
| `$caracter` | `CARACTER` o `TEXTO` (1 char) | Símbolo pintado en cada punto calculado | Se imprime vía `CURSOR()` + `ESCRIBIR()`. Debe ser exactamente 1 carácter. | Recomendados: `"#"`, `"●"`, `"○"`, `"░"`. Cadenas `>1` char causan comportamiento impredecible. |

## 🌐 Sistema de Coordenadas
Nico usa un sistema **1-based `(columna, fila)`** con origen en la esquina superior izquierda:
```
(1,1) ──────────────────────────► Columna ($centro_x)
  │
  │
  ▼
Fila ($centro_y)
```
⚠️ **Atención al orden:** `DIBUJARCIRCULO(x, y, ...)` usa `(columna, fila)`, mientras que `CURSOR(fila, col)` usa `(fila, columna)`. ¡No los confundas al mezclar comandos!

## ⚙️ Funcionamiento Interno (v1.1.0)
1. Recorre un bucle angular desde `0` hasta `2π` con paso fijo (`Δθ ≈ 0.10` rad).
2. Calcula coordenadas discretas:
   `px = REDONDEAR($centro_x + $radio * COSENO(θ))`
   `py = REDONDEAR($centro_y + $radio * SENO(θ))`
3. Posiciona el cursor: `CURSOR(py, px)`
4. Imprime el carácter: `ESCRIBIR($caracter)`
5. Repite hasta completar la circunferencia.

> 💡 **Nota:** En v1.1.0 **no hay validación de límites**. Si el círculo excede el área visible, `CURSOR()` puede provocar saltos de línea o sobreescritura según la terminal.

## 🧪 Ejemplos de Uso

### 1. Posición Fija
```nico
DIBUJARCIRCULO(40, 12, 8, "#")
```

### 2. Centrado Dinámicamente
```nico
VARIABLE ENTERA $w, $h
ANCHOTERMINAL($w)
ALTOTERMINAL($h)
DIBUJARCIRCULO($w / 2, $h / 2, 6, "●")
```

### 3. Con Variable de Carácter
```nico
VARIABLE CARACTER $p = "○"
DIBUJARCIRCULO(20, 5, 4, $p)
```

## 🛡️ Limitaciones y Buenas Prácticas

| Situación | Solución Recomendada |
|-----------|----------------------|
| **El círculo se corta en bordes** | Validá antes de llamar: `SI($centro_x - $radio < 2 O $centro_x + $radio > $w-2) ENTONCES ...` |
| **Se ve ovalado** | Usá dibujo manual con `factor_y = 0.5` (ver `TestGeometriaASCII_inline.nico`) o ajustá `0.45`–`0.55` según tu terminal. |
| **Necesitás un círculo relleno** | `DIBUJARCIRCULO` solo dibuja perímetro. Para relleno, usá `RELLENARRECTANGULO` o un bucle con `√((x-cx)²+(y-cy)²) ≤ radio`. |
| **Múltiples círculos superpuestos** | Agrupá en `SUBPROGRAMA` para mantener `BLOQUE PRINCIPAL` legible. El último en dibujarse queda "encima". |
| **Performance en radios grandes** | El paso angular fijo genera ~60 puntos para `radio=8`. Para `radio>20`, considerá aumentar `Δθ` proporcionalmente para evitar sobrecarga de `CURSOR()`. |

## 🔗 Relación con Otras Primitivas
- `DIBUJARLINEA`: Usa algoritmo de Bresenham (óptimo para líneas rectas). `DIBUJARCIRCULO` usa aproximación trigonométrica.
- `RELLENARRECTANGULO`: Rellena áreas rectangulares. Combinado con `DIBUJARCIRCULO` permite crear "botones" o "paneles circulares" básicos.
- `CURSOR`: `DIBUJARCIRCULO` mueve el cursor internamente. Tras dibujar, el cursor queda en la última coordenada pintada. Usá `CURSOR($f, $c)` antes de imprimir texto adicional.

---
📚 **Validado con Nico v1.1.0**. Compatible con Linux, Windows y Raspberry Pi.
💡 **Tip:** Para ajustes finos de aspecto visual, probá `0.08`–`0.12` como paso angular interno si modificás el runtime en futuras versiones.