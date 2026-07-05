# 🔁 Funciones con Retorno y Variables Locales

Nico soporta funciones definidas por el usuario con tipo de retorno explícito, parámetros por valor y variables locales aisladas por scope.

💡 Diferencia clave con SUBPROGRAMA: Las funciones calculan y retornan un valor con aislamiento de scope. Los subprogramas ejecutan acciones sin retornar valor.

---

## 📐 Sintaxis de Declaración

    FUNCION <TIPO_RETORNO> nombre_funcion($param1, $param2, ...)
        VARIABLE <TIPO> $local = valor  // Variables locales (opcionales)
        // ... lógica de cálculo ...
        CALCULAR EN $resultado = <expresión>  // O RESULTADO EN (son alias)
        RETORNAR $resultado  // Obligatorio: devuelve el valor
    FIN FUNCION

💡 RESULTADO EN es alias de CALCULAR EN (solo asigna/calcula). Solo RETORNAR devuelve el valor de la función al llamador.

---

## 📋 Tipos de Retorno Válidos

| Tipo (Nico) | Rango Aprox. | Uso Típico | Nota |
|-------------|--------------|------------|------|
| ENTERA | ±2.1×10⁹ | Contadores, índices, flags | Signed 32-bit |
| DECIMAL | ±1.7×10³⁰⁸ | Cálculos, promedios, precios | IEEE 754 double |
| LOGICA | VERDADERO/FALSO | Flags, condiciones | Booleano |
| TEXTO | Dinámico | Cadenas UTF-8 | Memoria dinámica |
| CARACTER | 0-255 | Caracteres individuales | 1 byte |
| ENTERA SIN SIGNO | 0 a 4.2×10⁹ | IDs, tamaños, hardware | Unsigned 32-bit |
| DECIMAL SIN SIGNO | 0 a +1.7×10³⁰⁸ | Magnitudes físicas, prob. | Sin negativos |

---

## 📋 Referencia de Keywords

| Keyword | Función | ¿Obligatorio? | Nota |
|---------|---------|--------------|------|
| FUNCION <TIPO> | Declara función con retorno | Sí | <TIPO>: uno de los tipos listados arriba |
| ($param1, ...) | Lista de parámetros | No | Se pasan por valor |
| VARIABLE ... | Variable local | No | Scope limitado a la función |
| CALCULAR EN $var = <expr> | Asigna/calcula valor | No | Alias: RESULTADO EN |
| RETORNAR <expr> | Devuelve valor al llamador | Sí | Obligatorio, debe coincidir con <TIPO_RETORNO> |

---

## 🧪 Ejemplo Mínimo Validado

    PROGRAMA FuncionesTipos
        VARIABLE DECIMAL $d = 0.0

    FUNCION DECIMAL promedio($a, $b, $c)
        VARIABLE DECIMAL $resultado = 0.0
        CALCULAR EN $resultado = ($a + $b + $c) / 3.0
        RETORNAR $resultado  // Devuelve el valor al llamador
    FIN FUNCION

    BLOQUE PRINCIPAL
        CALCULAR EN $d = promedio(8.5, 9.0, 7.5)
        ESCRIBIR("Promedio: $d") SALTO
    FIN PRINCIPAL
    FINAL

---

## ⚠️ Notas Técnicas Críticas

- RETORNAR obligatorio: Debe haber al menos un RETORNAR en cada ruta de ejecución. Si el flujo puede salir sin retornar, el comportamiento es indefinido.

- RESULTADO EN vs RETORNAR: RESULTADO EN es alias de CALCULAR EN (solo asigna/calcula). RETORNAR es lo que realmente devuelve el valor de la función al llamador.

- Conversión implícita: Si el tipo de la expresión en RETORNAR no coincide exactamente, Nico intenta convertir: DECIMAL → ENTERA trunca hacia cero. ENTERA → DECIMAL promueve sin pérdida.

- Scope aislado: Variables declaradas dentro son locales. No son visibles fuera, ni colisionan con globales del mismo nombre.

- Parámetros por valor: Modificar $a dentro no cambia la variable original del llamador.

- Sin sobrecarga: No podés tener dos funciones con el mismo nombre y distintos parámetros.

---

## 🎯 Buenas Prácticas

    // ✅ Función que retorna LOGICA
    FUNCION LOGICA es_par($numero)
        VARIABLE LOGICA $resultado = FALSO
        SI($numero % 2 IGUAL 0) ENTONCES
            CALCULAR EN $resultado = VERDADERO
        FIN SI
        RETORNAR $resultado
    FIN FUNCION

    // ✅ Función que retorna TEXTO
    FUNCION TEXTO saludar($nombre)
        VARIABLE TEXTO $mensaje = ""
        COPIARTEXTO($mensaje, "Hola, $nombre")
        RETORNAR $mensaje
    FIN FUNCION

    // ✅ Validar antes de retornar
    FUNCION DECIMAL dividir_seguro($numerador, $denominador)
        VARIABLE DECIMAL $resultado = 0.0
        SI($denominador IGUAL 0) ENTONCES
            CALCULAR EN $resultado = 0.0
        SINO
            CALCULAR EN $resultado = $numerador / $denominador
        FIN SI
        RETORNAR $resultado
    FIN FUNCION

    // ✅ Múltiples RETORNAR en distintas rutas
    FUNCION ENTERA signo($valor)
        SI($valor MAYOR 0) ENTONCES
            RETORNAR 1
        FIN SI
        SI($valor MENOR 0) ENTONCES
            RETORNAR -1
        FIN SI
        RETORNAR 0
    FIN FUNCION

    // ❌ Evitar conversión peligrosa o división por cero sin validar
    FUNCION ENTERA dividir_y_truncar($a, $b)
        RETORNAR $a / $b  // ← Sin validación previa
    FIN FUNCION

---

## 🔄 Equivalencias C Internas

| Nico | C Runtime | Descripción |
|------|-----------|-------------|
| FUNCION ENTERA f($x) | int f(int x) { | Signed 32-bit |
| FUNCION DECIMAL f($x) | double f(double x) { | IEEE 754 double |
| FUNCION LOGICA f($x) | bool f(bool x) { | Booleano |
| FUNCION TEXTO f($x) | char* f(char* x) { | Puntero a string |
| RETORNAR <expr> | return expr; | Retorno de valor |
| CALCULAR EN $var = expr | var = expr; | Asignación interna |
| FIN FUNCION | } | Cierre de función |

---

## 🧩 Integración con Otras Estructuras

    // Función + impresión
    FUNCION DECIMAL calcular_iva($monto, $tasa)
        VARIABLE DECIMAL $iva = 0.0
        CALCULAR EN $iva = $monto * $tasa / 100.0
        RETORNAR $iva
    FIN FUNCION

    BLOQUE PRINCIPAL
        VARIABLE DECIMAL $precio = 100.0, $iva = 0.0
        CALCULAR EN $iva = calcular_iva($precio, 21.0)
        ESCRIBIR("IVA: $iva") SALTO
    FIN PRINCIPAL

    // Función dentro de expresiones matemáticas
    CALCULAR EN $total = calcular_iva($precio, 21.0) + $precio

💡 Compatibilidad: Funciona idéntico en Linux y Windows. Los tipos se mapean directamente a tipos estándar de C, garantizando portabilidad y rendimiento nativo.
