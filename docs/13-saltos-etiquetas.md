# 🔄 Control de Flujo Avanzado: SALTAR A / ETIQUETA

Permite saltos incondicionales dentro del código mediante etiquetas con nombre. Similar al `goto` de C, útil para máquinas de estado, manejo de errores centralizado o optimizaciones de bajo nivel. **Usar con precaución y documentación clara**.

> 💡 **Nota importante**: Los saltos rompen el flujo lineal. Nico no verifica en tiempo de compilación si la etiqueta existe o si el salto cruza scopes de manera inválida. El comportamiento recae en la lógica del programador.

---

## 📐 Sintaxis Exacta
```nico
// Definir punto de destino
ETIQUETA nombre_etiqueta

// Saltar incondicionalmente al destino
SALTAR A nombre_etiqueta
```

---

## 📋 Reglas de Uso

| Concepto | Regla | Ejemplo |
|----------|-------|---------|
| Nombre | Alfanumérico + guion bajo. Sin espacios ni símbolos. | `etiqueta1`, `fin_bucle`, `error_handler` |
| Scope | Global dentro del `PROGRAMA`. Visible desde cualquier bloque. | `SALTAR A fin` funciona desde `BLOQUE PRINCIPAL` o `FUNCION` |
| Unicidad | Cada nombre debe ser único en todo el archivo. | No puede haber dos `ETIQUETA inicio` |
| Ejecución | Salto inmediato. No evalúa condición ni guarda estado. | `SALTAR A x` salta siempre que se ejecuta |
| Posición | Pueden definirse antes o después del salto. | Forward y backward jumps válidos |

---

## 🧪 Ejemplo Mínimo Validado
```nico
PROGRAMA TestSaltos
    VARIABLE ENTERA $opcion = 2
BLOQUE PRINCIPAL
    // Salto condicional simulando switch/dispatch
    SI ($opcion IGUAL 1) ENTONCES
        SALTAR A caso1
    FIN SI
    SI ($opcion IGUAL 2) ENTONCES
        SALTAR A caso2
    FIN SI
    SALTAR A casoDefault

    ETIQUETA caso1
    MOSTRAR("Caso 1 seleccionado") SALTO
    SALTAR A finCasos

    ETIQUETA caso2
    MOSTRAR("Caso 2 seleccionado") SALTO
    SALTAR A finCasos

    ETIQUETA casoDefault
    MOSTRAR("Caso por defecto") SALTO

    ETIQUETA finCasos
    MOSTRAR("Fin de casos") SALTO
FIN PRINCIPAL
FINAL
```

---

## ⚠️ Notas Técnicas Críticas
- **No hay verificación en compilación:** Si saltás a una etiqueta inexistente, el comportamiento es indefinido. Puede crashear o ignorar el salto.
- **Código muerto:** Todo lo que esté después de `FINAL` nunca se ejecuta, incluso si hay etiquetas definidas allí. El intérprete termina en `FINAL`.
- **Scope global:** Una etiqueta en `BLOQUE PRINCIPAL` es visible desde una `FUNCION` o `SUBPROGRAMA`. Esto puede generar acoplamiento oculto difícil de depurar.
- **No hay pila de retorno:** `SALTAR A` no guarda el punto de origen. No es un `CALL/RETURN`. Para subrutinas, usá `FUNCION` o `SUBPROGRAMA`.
- **Legibilidad:** El flujo no-lineal dificulta el seguimiento mental. Documentá claramente los destinos y mantené los saltos cortos.

---

## 🔄 Equivalencias en Otros Lenguajes

| Nico | C | Python | JavaScript |
|------|---|--------|------------|
| `ETIQUETA fin` | `fin:` | (no tiene goto nativo) | (no aplica) |
| `SALTAR A fin` | `goto fin;` | (no aplica) | (no aplica) |
| Uso típico | `goto` para cleanup/error | `while`/`break`/`continue` | Excepciones o flags |

---

## 🎯 Casos de Uso Legítimos
```nico
// ✅ Manejo centralizado de errores en funciones
FUNCION DECIMAL dividir($a, $b)
    SI($b IGUAL 0) ENTONCES
        SALTAR A error_div_cero
    FIN SI
    RETORNAR $a / $b

    ETIQUETA error_div_cero
    ESCRIBIR("Error: división por cero") SALTO
    RETORNAR 0.0
FIN FUNCION

// ✅ Máquina de estados simple
VARIABLE ENTERA $estado = 0
BLOQUE PRINCIPAL
    ETIQUETA inicio
    // ... lógica estado 0 ...
    SI($condicion) ENTONCES 
        SALTAR A estado1 
    FIN SI
    SALTAR A inicio

    ETIQUETA estado1
    // ... lógica estado 1 ...
    SI($otra_cond) ENTONCES 
        SALTAR A inicio 
    FIN SI
    SALTAR A estado1
FIN PRINCIPAL

// ✅ Salir temprano de bucles anidados complejos
MIENTRAS($externo) HACER
    MIENTRAS($interno) HACER
        SI($error_grave) ENTONCES 
            SALTAR A limpieza 
        FIN SI
    FIN MIENTRAS
FIN MIENTRAS
ETIQUETA limpieza
// recursos limpios aquí
```

---

## ❌ Cuándo NO Usar SALTAR A

| Situación | Alternativa Recomendada | Razón |
|-----------|------------------------|-------|
| Salir de un bucle simple | `CORTE` o flag + `FIN MIENTRAS` | Más legible y estructurado |
| Ramificación múltiple | `SEGUN CASO` o `SI/SINOSI` | Flujo lineal, fácil de seguir |
| Reutilizar lógica | `FUNCION` o `SUBPROGRAMA` | Aislamiento, parámetros, retorno |
| Saltar entre archivos | No soportado (ni debería) | Acoplamiento excesivo, imposible de mantener |

---

## 🧩 Integración con Otras Estructuras
```nico
// SALTAR A + SI para dispatch manual limpio
SI($comando IGUAL 1) ENTONCES 
    SALTAR A cmd_save 
FIN SI
SI($comando IGUAL 2) ENTONCES 
    SALTAR A cmd_load 
FIN SI
SALTAR A cmd_unknown

ETIQUETA cmd_save
// guardar...
SALTAR A fin_dispatch

ETIQUETA cmd_load
// cargar...
SALTAR A fin_dispatch

ETIQUETA cmd_unknown
ESCRIBIR("Comando no reconocido") SALTO

ETIQUETA fin_dispatch
// continuar flujo normal...
```

---

## 🪟 Compatibilidad Windows/Linux

| Aspecto | Comportamiento |
|---------|---------------|
| Implementación | Traducido directamente a `goto` en el C generado |
| Performance | Salto directo, overhead cero. Idéntico en ambas plataformas |
| Depuración | Los debuggers muestran el salto, pero el stack trace puede perderse |
| Optimización | El compilador C respeta el flujo lógico; no reordena saltos etiquetados |

> 💡 **Consejo profesional:** Usá `SALTAR A` solo cuando las estructuras de alto nivel (`SI`, `MIENTRAS`, `FUNCION`) no expresen claramente tu intención. En el 95% de los casos, hay una alternativa más legible y mantenible.