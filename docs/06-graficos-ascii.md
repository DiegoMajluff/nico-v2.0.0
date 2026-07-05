# 🎨 Gráficos ASCII y Primitivas de Dibujo

Comandos para renderizar formas geométricas y áreas rellenas directamente en la consola. Ideales para UIs, juegos simples, visualización de datos o efectos visuales sin dependencias externas.

> 💡 **Sistema de coordenadas**: Todas las primitivas usan `(columna, fila)` **1-based**. El origen `(1,1)` es la esquina superior izquierda. Consultá `ANCHOTERMINAL` y `ALTOTERMINAL` (documentados en `05-sistema-consola.md`) para obtener los límites visibles antes de dibujar.

---

## 📐 Primitivas de Dibujo

| Procedimiento | Sintaxis | Ejemplo | Nota |
|---------------|----------|---------|------|
| `DIBUJARLINEA` | `DIBUJARLINEA($x1, $y1, $x2, $y2, $char)` | `DIBUJARLINEA(1, 1, 80, 1, "═")` | Algoritmo de Bresenham. `$char` debe ser 1 carácter visible. |
| `DIBUJARCIRCULO` | `DIBUJARCIRCULO($cx, $cy, $radio, $char)` | `DIBUJARCIRCULO(40, 12, 8, "○")` | Aproximación paramétrica. Fuente monoespaciada recomendada. |
| `RELLENARRECTANGULO` | `RELLENARRECTANGULO($x1, $y1, $x2, $y2, $char)` | `RELLENARRECTANGULO(1, 1, 80, 24, "░")` | Rellena área rectangular incluida. Ideal fondos/paneles. |

---

## 🎨 Caracteres de Relleno Recomendados

| Categoría | Ejemplos | Uso típico |
|-----------|----------|------------|
| **Sombras** | `"░"`, `"▒"`, `"▓"` | Degradados, overlays, progreso |
| **Bloques** | `"█"`, `"▀"`, `"▄"` | Barras, indicadores, separadores |
| **Bordes** | `"─"`, `"│"`, `"═"`, `"║"` | Marcos, tablas, UI estructurada |
| **Espacio** | `" "` | Limpiar zonas, crear "ventanas" |

> ⚠️ **UTF-8 requerido**: Caracteres box-drawing y bloques requieren terminal con soporte UTF-8. En consolas legacy, pueden verse como `?` o símbolos alternativos.

---

## 🧪 Ejemplo Mínimo Validado
```nico
PROGRAMA demo_graficos
    VARIABLE ENTERA $cx, $cy
BLOQUE PRINCIPAL
    ANCHOTERMINAL($cx)
    ALTOTERMINAL($cy)
    LIMPIARPANTALLA

    // Fondo degradado simulado
    RELLENARRECTANGULO(1, 1, $cx, $cy/2, "░")
    RELLENARRECTANGULO(1, $cy/2, $cx, $cy, "▒")

    // Caja central
    RELLENARRECTANGULO(10, 5, 30, 15, " ")
    DIBUJARLINEA(10, 5, 30, 5, "═")
    DIBUJARLINEA(10, 15, 30, 15, "═")
    DIBUJARLINEA(10, 5, 10, 15, "║")
    DIBUJARLINEA(30, 5, 30, 15, "║")

    CURSOR(9, 12)
    MOSTRAR("🟦 Fondo activo!!") SALTO
    ESPERAR(3000, MILISEGUNDOS)
FIN PRINCIPAL
FINAL
```

---

## ⚠️ Notas Técnicas Críticas
- **Orden de dibujo:** Los comandos se ejecutan secuencialmente. Un relleno posterior cubre lo anterior. Para capas, dibujá de atrás hacia adelante.
- **Performance:** Rellenar áreas grandes carácter por carácter puede ser lento en terminales remotas (SSH). Minimiza áreas redibujadas en bucles.
- **Coordenadas válidas:** Si `$x2 < $x1` o `$y2 < $y1`, el comportamiento es indefinido. Validá antes de llamar:
  ```nico
  SI($x2 MENOR $x1) ENTONCES 
      ASIGNAR EN $x2 = $x1 
  FIN SI
  ```
- **Carácter unitario:** `$char` debe ser exactamente 1 carácter. `"AB"` o `""` causan comportamiento impredecible.
- **Integración con CURSOR:** Tras dibujar, el cursor queda en la última posición renderizada. Reposicioná con `CURSOR($fila, $col)` antes de imprimir.

---

## 🎯 Buenas Prácticas
```nico
// ✅ Encapsular paneles en subprogramas reusables
SUBPROGRAMA DibujarPanel($x1, $y1, $x2, $y2, $titulo)
    RELLENARRECTANGULO($x1, $y1, $x2, $y2, " ")
    DIBUJARLINEA($x1, $y1, $x2, $y1, "═")
    DIBUJARLINEA($x1, $y2, $x2, $y2, "═")
    DIBUJARLINEA($x1, $y1, $x1, $y2, "║")
    DIBUJARLINEA($x2, $y1, $x2, $y2, "║")
    CURSOR($y1, $x1 + 2)
    MOSTRAR(" $titulo ")
FIN SUBPROGRAMA

// ✅ Usar espacios para limpiar antes de redibujar
RELLENARRECTANGULO(10, 5, 40, 15, " ")
// ... luego dibujar nuevo contenido ...

// ❌ Evitar redibujar pantalla completa en bucles tight
MIENTRAS($activo) HACER
    RELLENARRECTANGULO(1,1,80,24," ")  // ← Lento a 60 FPS
    // Mejor: actualizar solo celdas cambiadas
FIN MIENTRAS
```

---

## 🪟 Compatibilidad Windows/Linux

| Aspecto | Linux/macOS | Windows (Win10+ VT) | Windows Legacy |
|---------|-------------|---------------------|---------------|
| Box-drawing | ✅ UTF-8 nativo | ✅ Con VT habilitado | ⚠️ Codepage 437 |
| Sombras/Bloques | ✅ | ✅ | ⚠️ Símbolos alternativos |
| Performance | ✅ Rápido local | ✅ Similar | ⚠️ Más lento por emulación |

> 💡 Nico adapta la salida cuando es posible. Para máxima portabilidad, probá tu UI en ambas plataformas o usá caracteres ASCII seguros (`+`, `-`, `|`, `#`) si no controlás el entorno del usuario.