# 🎨 Colores y Estilos de Texto en Consola

Comandos para controlar colores de primer plano, fondo y atributos visuales
(negrita, cursiva, subrayado) directamente en la terminal. Ideales para
resaltar mensajes, crear interfaces claras o dar feedback visual.

> 💡 **Nota importante**: Todos estos comandos modifican el estado de la
> terminal. Los cambios persisten hasta que se ejecuta explícitamente un
> reset. No se restablecen automáticamente al final de `ESCRIBIR`.

---

### 🌈 Control de Colores
| Comando      | Sintaxis            | Colores válidos 
|--------------|---------------------|--------------------------------------
| `COLORTEXTO` | `COLORTEXTO(color)` | `negro`, `rojo`, `verde`, `amarillo`, `azul`, `magenta`, `cyan`, `blanco` 
| `COLORFONDO` | `COLORFONDO(color)` | Mismos que `COLORTEXTO` 
| `RESETCOLOR` | `RESETCOLOR`        | Restablece colores a los valores por defecto de la terminal 

> ⚠️ **Regla de sintaxis**: Los colores se escriben en **minúsculas y sin
> comillas**. `COLORTEXTO(rojo)` ✅, `COLORTEXTO("rojo")` ❌.

### ✨ Control de Estilos
| Comando          | Sintaxis         | Efecto 
|------------------|------------------|---------------------------------------
| `TEXTONEGRITA`   | `TEXTONEGRITA`   | Peso fuerte (bold) 
| `TEXTOCURSIVA`   | `TEXTOCURSIVA`   | Itálico (soporte variable según terminal) 
| `TEXTOSUBRAYADO` | `TEXTOSUBRAYADO` | Subrayado simple 
| `RESETTEXTO`     | `RESETTEXTO`     | Quita estilos activos. **No afecta colores** 

---

### 🧪 Ejemplo Mínimo Validado
```nico
PROGRAMA EjemploTextoEstilos
BLOQUE PRINCIPAL
    // Texto con color
    COLORTEXTO(azul)
    ESCRIBIR("Texto en azul") SALTO
    RESETCOLOR

    // Estilos combinados
    TEXTONEGRITA
    COLORTEXTO(amarillo)
    ESCRIBIR("NEGRITA + AMARILLO") SALTO
    RESETCOLOR
    RESETTEXTO  // ← Importante: limpiar ambos
FIN PRINCIPAL
FINAL

#⚠️ Notas Técnicas Críticas

 + Persistencia independiente: RESETCOLOR solo quita colores. RESETTEXTO
   solo quita estilos. Para volver al estado original, usá ambos en cualquier
   orden.
 + Soporte de cursiva: No todas las terminales renderizan \033[3m como
   itálico real. Algunas muestran texto normal o invierten colores. Validá en
   tu entorno.
 + Legibilidad: No todos los combos texto/fondo son legibles. Ej:
   COLORTEXTO(azul) + COLORFONDO(negro) puede ser difícil de leer en
   algunas configuraciones.
 + Salida a archivo: Al redirigir output (> log.txt), las secuencias ANSI
   se guardan como texto crudo. Usá resets antes de escribir en archivos si
   querés evitar códigos de escape.
 + Windows Legacy: En consolas antiguas sin soporte VT, Nico usa fallbacks
   WinAPI (SetConsoleTextAttribute). Los colores básicos funcionan, pero
   combinaciones complejas pueden variar.

#🎯 Buenas Prácticas

// ✅ Encapsular bloques coloreados con reset explícito
COLORTEXTO(verde)
TEXTONEGRITA
ESCRIBIR("✅ Operación exitosa") SALTO
RESETCOLOR
RESETTEXTO

// ✅ Combinar con CURSOR para UI tipo menú
CURSOR(5, 10)
COLORTEXTO(cyan)
ESCRIBIR(">> Opción seleccionada") SALTO
RESETCOLOR

// ❌ Evitar estilos residuales
COLORTEXTO(azul)
TEXTONEGRITA
// ... mucho código después ...
ESCRIBIR("Este texto heredará azul + negrita") SALTO  // ← Bug visual
// Solución: siempre resetear explícitamente

#🔄 Equivalencias ANSI y WinAPI

Nico                    |  Secuencia ANSI    |      WinAPI (Fallback)
------------------------|--------------------|--------------------------
COLORTEXTO(rojo)        |     \033[31m       |       FOREGROUND_RED
COLORFONDO(azul)        |     \033[44m       |       BACKGROUND_BLUE
TEXTONEGRITA            |     \033[1m        |       FOREGROUND_INTENSITY
RESETCOLOR + RESETTEXTO |     \033[0m        |       Reset console attributes   

💡 Nico detecta automáticamente el entorno y aplica el mejor fallback
disponible. En consolas legacy, la cursiva se omite silenciosamente para
evitar artefactos visuales.
