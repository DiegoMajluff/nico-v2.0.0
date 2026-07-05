# 🔧 Subprogramas: Organización de Código Reutilizable

Los SUBPROGRAMA en Nico son bloques de código reutilizables que pueden aceptar parámetros y operar sobre variables globales o locales. No retornan valor, a diferencia de las FUNCION.

💡 Diferencia clave con FUNCION: Los subprogramas ejecutan acciones sin retornar valor. Las funciones calculan y retornan un valor con RESULTADO EN.

---

## 📐 Sintaxis de Declaración y Llamada

    // Declaración (fuera de BLOQUE PRINCIPAL)
    SUBPROGRAMA NombreDelSubprograma($param1, $param2, ...)
        // Código aquí
        // Puede usar parámetros y variables globales
    FIN SUBPROGRAMA

    // Llamada (dentro de BLOQUE PRINCIPAL u otro SUBPROGRAMA)
    LLAMAR A NombreDelSubprograma(valor1, valor2, ...)

---

## 📋 Características Clave

| Característica | Comportamiento en Nico | Nota |
|----------------|------------------------|------|
| Parámetros | ✅ Soportados | La sintaxis es SUBPROGRAMA Nombre($param1, $param2) |
| Retorno | ❌ No retorna valor | Es un procedimiento, no una función |
| Scope de variables | 🌐 Global + parámetros | Accede a globales y recibe parámetros por valor |
| Variables locales | ✅ Sí existen | Podés declarar VARIABLE dentro del subprograma |
| Llamada | LLAMAR A Nombre(args) | Con paréntesis y argumentos |

---

## 🧪 Ejemplo Mínimo Validado

    PROGRAMA TestSubs
        VARIABLE ENTERA $x = 10
        VARIABLE ENTERA $y = 0

    SUBPROGRAMA Saludar($nombre)
        ESCRIBIR("¡Hola, $nombre!") SALTO
        ESCRIBIR("El valor de \$x es: $x") SALTO
    FIN SUBPROGRAMA

    SUBPROGRAMA Sumar($a, $b)
        CALCULAR EN $y = $a + $b
    FIN SUBPROGRAMA

    BLOQUE PRINCIPAL
        ESCRIBIR("Inicio del programa") SALTO
        LLAMAR A Saludar("Diego")
        LLAMAR A Sumar(5, 3)
        ESCRIBIR("Resultado: $y") SALTO
    FIN PRINCIPAL
    FINAL

---

## ⚠️ Notas Técnicas Críticas

- Scope de parámetros: Los parámetros son locales al subprograma. Modificarlos dentro no afecta las variables del llamador.

- Acceso a globales: Cualquier $variable que no sea parámetro ni local refiere a la declaración global del PROGRAMA.

- Efectos secundarios intencionales: La modificación de variables globales es un mecanismo válido de comunicación. Documentá qué variables lee/modifica cada subprograma.

- Orden de declaración: Los SUBPROGRAMA deben declararse antes del BLOQUE PRINCIPAL o antes de su primera llamada.

- Sin retorno: No podés usar RESULTADO EN dentro de un SUBPROGRAMA. Si necesitás retornar un valor, usá FUNCION.

- Recursión: Un SUBPROGRAMA puede llamarse a sí mismo, pero tené en cuenta el límite de 7000 niveles de recursión.

---

## 🎯 Buenas Prácticas

    // ✅ Documentar qué variables globales usa cada subprograma
    // SUBPROGRAMA: ActualizarUI
    //   Lee: $mensaje, $estado
    //   Modifica: (ninguna, solo imprime)
    SUBPROGRAMA ActualizarUI($titulo)
        CURSOR(1, 1)
        COLORTEXTO(verde)
        ESCRIBIR("$titulo - Estado: $estado") SALTO
        ESCRIBIR("Mensaje: $mensaje") SALTO
        RESETCOLOR
    FIN SUBPROGRAMA

    // ✅ Agrupar lógica relacionada en subprogramas pequeños
    SUBPROGRAMA InicializarJuego()
        CALCULAR EN $vidas = 3
        CALCULAR EN $puntos = 0
        COPIARTEXTO($mensaje, "¡Nuevo juego!")
    FIN SUBPROGRAMA

    // ✅ Usar parámetros para pasar información específica
    SUBPROGRAMA RestarVida($cantidad)
        CALCULAR EN $vidas = $vidas - $cantidad
        SI($vidas MENOR 1) ENTONCES
            COPIARTEXTO($mensaje, "Game Over")
            CALCULAR EN $game_over = 1
        FIN SI
    FIN SUBPROGRAMA

    BLOQUE PRINCIPAL
        LLAMAR A InicializarJuego()
        MIENTRAS($game_over IGUAL 0) HACER
            // ... lógica del juego ...
            LLAMAR A RestarVida(1)
            LLAMAR A ActualizarUI("Juego Principal")
        FIN MIENTRAS
    FIN PRINCIPAL

---

## 🔄 Diferencias: SUBPROGRAMA vs FUNCION

| Aspecto | SUBPROGRAMA | FUNCION |
|---------|-------------|---------|
| Propósito | Ejecutar acciones, organizar código | Calcular y retornar un valor |
| Parámetros | ✅ Sí (por valor) | ✅ Sí (por valor) |
| Retorno | ❌ No retorna valor | ✅ Sí (RESULTADO EN) |
| Scope | 🌐 Local + acceso a globales | 🔒 Local + acceso a globales |
| Variables locales | ✅ Sí | ✅ Sí |
| Llamada | LLAMAR A Nombre(args) | CALCULAR EN $var = nombre(args) |

---

## 📌 Cuándo Usar SUBPROGRAMA vs FUNCION

| Escenario | Recomendación | Razón |
|-----------|--------------|-------|
| Mostrar UI repetida o menús | ✅ SUBPROGRAMA | No necesita retornar valor, organiza salida |
| Calcular un valor a partir de inputs | ✅ FUNCION | Necesita devolver el resultado al programa |
| Modificar múltiples variables de estado | ✅ SUBPROGRAMA | Acceso directo a globales, evita retornar múltiples valores |
| Lógica pura sin efectos secundarios | ✅ FUNCION | Testeable, predecible, sin dependencias ocultas |
| Inicialización / configuración global | ✅ SUBPROGRAMA | Diseñado para operar sobre estado global |

💡 Compatibilidad: Equivale a void nombre(...) en C. Los parámetros se pasan por valor. Funciona idéntico en Linux y Windows.