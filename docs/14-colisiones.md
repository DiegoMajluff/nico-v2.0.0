# 🎮 Detección de Colisiones: COLISIONRECTANGULOS

Función que determina si dos rectángulos alineados a los ejes (AABB) se solapan en un espacio 2D. Ideal para juegos, menús interactivos o validación de zonas en consola. Implementa el algoritmo clásico de 4 comparaciones.

> 💡 **Regla mnemotécnica**: "Primero el rectángulo 1 completo (x,y,w,h), luego el rectángulo 2 completo (x,y,w,h), finalmente dónde guardar el resultado". Total: 4 + 4 + 1 = 9 parámetros.

---

## 📐 Sintaxis Exacta
```nico
COLISIONRECTANGULOS($x1, $y1, $w1, $h1, $x2, $y2, $w2, $h2, $resultado)
```

---

## 📋 Parámetros (Orden Visual)

| Grupo | Parámetro | Tipo | Descripción | Ejemplo |
|-------|-----------|------|-------------|---------|
| Rectángulo A | `$x1, $y1` | `ENTERA`/`DECIMAL` | Esquina superior-izquierda de A | `10, 5` |
| | `$w1, $h1` | `ENTERA`/`DECIMAL` | Ancho y alto de A | `20, 10` |
| Rectángulo B | `$x2, $y2` | `ENTERA`/`DECIMAL` | Esquina superior-izquierda de B | `25, 8` |
| | `$w2, $h2` | `ENTERA`/`DECIMAL` | Ancho y alto de B | `15, 12` |
| Resultado | `$resultado` | `VARIABLE` declarada | Recibe `1` si colisionan, `0` si no | `$colisiona` |

> ⚠️ **Importante:** El último parámetro debe ser una variable declarada que comience con `$`. No puede ser un literal ni una expresión.

---

## 🧮 Algoritmo AABB (Explicación Visual)

Dos rectángulos **NO colisionan** si cumple **ALGUNA** de estas condiciones:
- A está a la izquierda de B: `x1 + w1 <= x2`
- A está a la derecha de B: `x1 >= x2 + w2`
- A está arriba de B: `y1 + h1 <= y2`
- A está abajo de B: `y1 >= y2 + h2`

Por lo tanto, **HAY COLISIÓN** cuando **NINGUNA** se cumple:
```
(x1 < x2 + w2) AND (x1 + w1 > x2) AND
(y1 < y2 + h2) AND (y1 + h1 > y2)
```

---

## 🖼️ Diagrama de Ejemplo

```
Rect A: (10,10) tamaño 20x20    Rect B: (25,15) tamaño 15x15

     10        30                   25        40
    ┌────────────┐                 ┌─────────┐
 10 │████████████│               15│█████████│
    │████████████│                 │█████████│ ← ¡Se solapan!
    │████████████│                 │█████████│
 30 └────────────┘                 └─────────┘
                                    30

Resultado: $colisiona = 1 ✅
```

---

## 🧪 Ejemplo Mínimo Validado
```nico
PROGRAMA TestColision
    VARIABLE ENTERA $colisiona
BLOQUE PRINCIPAL
    // Rect A: posición (10,10), tamaño 20x20
    // Rect B: posición (25,15), tamaño 15x15
    COLISIONRECTANGULOS(10, 10, 20, 20, 25, 15, 15, 15, $colisiona)
    
    SI($colisiona IGUAL 1) ENTONCES
        ESCRIBIR("✅ ¡Colisión detectada!") SALTO
    SINO
        ESCRIBIR("❌ Sin colisión") SALTO
    FIN SI
FIN PRINCIPAL
FINAL
```

---

## 🎯 Casos de Prueba Rápidos

| Escenario | Rect A | Rect B | Resultado | Razón |
|-----------|--------|--------|-----------|-------|
| ✅ Solapamiento total | `(0,0,10,10)` | `(2,2,6,6)` | `1` | B dentro de A |
| ✅ Solapamiento parcial | `(0,0,10,10)` | `(8,5,10,10)` | `1` | Bordes se cruzan |
| ❌ Separados en X | `(0,0,10,10)` | `(15,5,10,10)` | `0` | `A.x+wA=10 < B.x=15` |
| ❌ Separados en Y | `(0,0,10,10)` | `(5,15,10,10)` | `0` | `A.y+hA=10 < B.y=15` |
| ❌ Toque exacto | `(0,0,10,10)` | `(10,0,10,10)` | `0` | Bordes adyacentes ≠ colisión |

> 💡 **Nota:** Los bordes que se tocan exactamente (`x1+w1 == x2`) no se consideran colisión. Si necesitás detectar "toque", usá márgenes de tolerancia expandiendo ligeramente los rectángulos.

---

## ⚠️ Notas Técnicas Críticas
- **Coordenadas 1-based:** Nico usa `(columna, fila)` iniciando en `1,1`. Asegurate de que tus cálculos sean consistentes con `CURSOR` y otros comandos gráficos.
- **Tipos flexibles:** Los 8 parámetros numéricos aceptan `ENTERA` o `DECIMAL`. Se evalúan como `double` internamente para precisión.
- **Variable destino:** `$resultado` puede ser cualquier tipo numérico declarado. El valor `1/0` se convierte implícitamente al tipo.
- **Performance O(1):** Solo 4 comparaciones y asignación. Ideal para bucles de juego a 60 FPS o detección en tiempo real.
- **No valida rangos:** Valores negativos o extremos se calculan igual. Validá inputs si necesitás robustez en producción.

---

## 🎯 Buenas Prácticas
```nico
// ✅ Encapsular en función reusable para claridad
FUNCION ENTERA Colisionan($ax, $ay, $aw, $ah, $bx, $by, $bw, $bh)
    VARIABLE ENTERA $res
    COLISIONRECTANGULOS($ax, $ay, $aw, $ah, $bx, $by, $bw, $bh, $res)
    RETORNAR $res
FIN FUNCION

// ✅ Resetear flag antes de cada chequeo en bucles
MIENTRAS($activo) HACER
    ASIGNAR EN $colisiona = 0  // ← Importante
    COLISIONRECTANGULOS(..., $colisiona)
    SI($colisiona IGUAL 1) ENTONCES ... FIN SI
FIN MIENTRAS

// ✅ Usar márgenes de tolerancia para "toque suave"
COLISIONRECTANGULOS($x1-1, $y1-1, $w1+2, $h1+2, $x2, $y2, $w2, $h2, $res)
```

---

## 🪟 Compatibilidad Windows/Linux

| Aspecto | Comportamiento |
|---------|---------------|
| Implementación | Lógica pura en C, sin dependencias de SO |
| Performance | Idéntica en ambas plataformas (4 comparaciones) |
| Precisión | Usa `double` para cálculos intermedios |
| Uso con consola | Coordenadas consistentes con `CURSOR` y gráficos |

> 💡 **Tip para juegos:** Para optimizar, pre-calculá `x+w` e `y+h` fuera del bucle si los rectángulos no cambian de tamaño. Ahorrás 2 sumas por frame y mantenés el loop tight.