# 📦 Sistema de Inclusión de Archivos (INCLUIR)

El sistema INCLUIR permite modularizar código Nico dividiéndolo en múltiples archivos. Esto facilita la reutilización de funciones y subprogramas, y mantiene el código organizado.

💡 Concepto clave: INCLUIR inserta el contenido del archivo especificado en el punto donde se declara, como si hubieras escrito ese código directamente en tu programa.

---

## 📐 Sintaxis

    PROGRAMA MiPrograma
        INCLUIR "ruta/al/archivo.nico"
        INCLUIR "otra_lib.nico"
        
        // Variables globales y declaraciones
        VARIABLE ENTERA $contador = 0

    BLOQUE PRINCIPAL
        // Código principal
    FIN PRINCIPAL
    FINAL

### Reglas importantes:
- INCLUIR debe aparecer dentro del bloque PROGRAMA, antes de BLOQUE PRINCIPAL
- La ruta del archivo va entre comillas dobles
- Se puede incluir múltiples archivos
- Los archivos incluidos se procesan en orden de aparición

---

## 🧪 Ejemplo Completo

### Archivo principal: calculadora.nico

    PROGRAMA Calculadora
        INCLUIR "operaciones.nico"
        INCLUIR "utilidades.nico"
        
        VARIABLE ENTERA $resultado = 0

    BLOQUE PRINCIPAL
        CALCULAR EN $resultado = sumar(10, 5)
        ESCRIBIR("10 + 5 = $resultado") SALTO
        
        CALCULAR EN $resultado = multiplicar(4, 3)
        ESCRIBIR("4 * 3 = $resultado") SALTO
        
        LLAMAR A saludar("Diego")
    FIN PRINCIPAL
    FINAL

---

### Archivo separado: operaciones.nico

    FUNCION ENTERA sumar($a, $b)
        VARIABLE ENTERA $suma = 0
        CALCULAR EN $suma = $a + $b
        RETORNAR $suma
    FIN FUNCION

    FUNCION ENTERA multiplicar($a, $b)
        VARIABLE ENTERA $producto = 0
        CALCULAR EN $producto = $a * $b
        RETORNAR $producto
    FIN FUNCION

---

### Archivo separado: utilidades.nico

    SUBPROGRAMA saludar($nombre)
        ESCRIBIR("¡Hola, $nombre!") SALTO
    FIN SUBPROGRAMA

    ---

## 📋 Características del Sistema INCLUIR

| Característica | Comportamiento | Nota |
|----------------|----------------|------|
| Ubicación | Dentro de PROGRAMA, antes de BLOQUE PRINCIPAL | No se puede incluir dentro de funciones o bloques |
| Orden de procesamiento | Secuencial, de arriba hacia abajo | Los archivos se procesan en el orden que aparecen |
| Scope de variables | Las variables globales del archivo incluido son accesibles | Respetan las reglas normales de scope |
| Funciones y subprogramas | Se cargan y están disponibles en todo el programa | Pueden ser llamadas desde BLOQUE PRINCIPAL |
| Inclusión anidada | Un archivo incluido puede tener sus propios INCLUIR | Soporta múltiples niveles de inclusión |
| Detección de duplicados | El sistema previene incluir el mismo archivo múltiples veces | Evita redefiniciones de funciones |

---

## 🎯 Buenas Prácticas

### ✅ Organización por responsabilidad
    // operaciones.nico - Solo funciones matemáticas
    FUNCION ENTERA sumar($a, $b)
        VARIABLE ENTERA $resultado = 0
        CALCULAR EN $resultado = $a + $b
        RETORNAR $resultado
    FIN FUNCION

    // ui.nico - Solo funciones de interfaz
    SUBPROGRAMA mostrar_menu()
        ESCRIBIR("=== Menú Principal ===") SALTO
        ESCRIBIR("1. Sumar") SALTO
        ESCRIBIR("2. Restar") SALTO
    FIN SUBPROGRAMA

### ✅ Nombres descriptivos para archivos incluidos
    INCLUIR "operaciones_matematicas.nico"  // ✅ Claro
    INCLUIR "ops.nico"                      // ❌ Ambiguo
    INCLUIR "funciones_interfaz_usuario.nico"  // ✅ Descriptivo

### ✅ Agrupar includes al inicio
    PROGRAMA MiApp
        // Todos los includes al principio
        INCLUIR "constantes.nico"
        INCLUIR "utilidades.nico"
        INCLUIR "operaciones.nico"
        
        // Luego las variables globales
        VARIABLE ENTERA $estado = 0
    BLOQUE PRINCIPAL
        // Código principal
    FIN PRINCIPAL
    FINAL

---

## ⚠️ Notas Técnicas Importantes

### 🔹 Resolución de rutas
- Las rutas son relativas al directorio donde se ejecuta el programa principal
- Se recomienda usar rutas relativas simples: INCLUIR "lib/operaciones.nico"
- Evitar rutas absolutas para mantener portabilidad entre sistemas

### 🔹 Orden de dependencia
Si una función en archivo A usa una función de archivo B, asegurate de incluir B antes que A:

    PROGRAMA MiPrograma
        INCLUIR "base.nico"      // ✅ Primero las dependencias base
        INCLUIR "avanzado.nico"  // ✅ Luego las que usan funciones de base
    BLOQUE PRINCIPAL
        // Código
    FIN PRINCIPAL
    FINAL

### 🔹 Variables globales compartidas
Los archivos incluidos pueden acceder y modificar variables globales del programa principal:

    // archivo: contador.nico
    SUBPROGRAMA incrementar()
        CALCULAR EN $contador = $contador + 1  // Accede a variable global
    FIN SUBPROGRAMA

    // programa principal
    PROGRAMA MiPrograma
        INCLUIR "contador.nico"
        VARIABLE ENTERA $contador = 0
    BLOQUE PRINCIPAL
        LLAMAR A incrementar()
        ESCRIBIR("Contador: $contador") SALTO  // Imprime: 1
    FIN PRINCIPAL
    FINAL

### 🔹 Evitar inclusiones circulares
No incluyas archivos que se incluyen mutuamente, esto puede causar comportamientos indefinidos:

    // ❌ MAL: archivo_a.nico incluye archivo_b.nico
    //         y archivo_b.nico incluye archivo_a.nico

    ---

## 🧰 Troubleshooting Rápido

| Síntoma | Causa probable | Solución |
|---------|---------------|----------|
| Error "archivo no encontrado" | Ruta incorrecta o archivo no existe | Verificar ruta relativa desde el directorio de ejecución |
| Función no definida | Archivo incluido después de su uso | Reordenar INCLUIR antes de BLOQUE PRINCIPAL |
| Variable no reconocida | Variable declarada en archivo incluido pero no en scope | Declarar variable global en PROGRAMA principal |
| Función duplicada | Mismo archivo incluido múltiples veces | Verificar que no haya inclusiones redundantes |
| Error de sintaxis en archivo incluido | Error en el archivo .nico incluido | Revisar sintaxis del archivo incluido independientemente |

---

## 💼 Casos de Uso Comunes

### 📚 Biblioteca de funciones matemáticas
    // matematicas.nico
    FUNCION DECIMAL promedio($a, $b, $c)
        VARIABLE DECIMAL $resultado = 0.0
        CALCULAR EN $resultado = ($a + $b + $c) / 3.0
        RETORNAR $resultado
    FIN FUNCION

    FUNCION ENTERA factorial($n)
        VARIABLE ENTERA $resultado = 1
        VARIABLE ENTERA $i = 1
        MIENTRAS($i MENOR_IGUAL $n) HACER
            CALCULAR EN $resultado = $resultado * $i
            CALCULAR EN $i = $i + 1
        FIN MIENTRAS
        RETORNAR $resultado
    FIN FUNCION

### 🎮 Sistema de juego modular
    // constantes.nico
    VARIABLE ENTERA $VIDAS_INICIALES = 3
    VARIABLE ENTERA $PUNTOS_POR_MONEDA = 100

    // jugador.nico
    SUBPROGRAMA inicializar_jugador()
        CALCULAR EN $vidas = $VIDAS_INICIALES
        CALCULAR EN $puntos = 0
    FIN SUBPROGRAMA

    // nivel.nico
    SUBPROGRAMA cargar_nivel($numero)
        ESCRIBIR("Cargando nivel $numero...") SALTO
        // Lógica de carga
    FIN SUBPROGRAMA

    // juego.nico (principal)
    PROGRAMA Juego
        INCLUIR "constantes.nico"
        INCLUIR "jugador.nico"
        INCLUIR "nivel.nico"
        
        VARIABLE ENTERA $vidas = 0
        VARIABLE ENTERA $puntos = 0
    BLOQUE PRINCIPAL
        LLAMAR A inicializar_jugador()
        LLAMAR A cargar_nivel(1)
        // Lógica del juego
    FIN PRINCIPAL
    FINAL

### 📊 Procesamiento de datos
    // archivos.nico
    SUBPROGRAMA cargar_datos($ruta)
        // Leer archivo y procesar datos
        ESCRIBIR("Cargando datos desde $ruta") SALTO
    FIN SUBPROGRAMA

    // estadisticas.nico
    FUNCION DECIMAL calcular_media($suma, $cantidad)
        VARIABLE DECIMAL $resultado = 0.0
        SI($cantidad MAYOR 0) ENTONCES
            CALCULAR EN $resultado = $suma / $cantidad
        FIN SI
        RETORNAR $resultado
    FIN FUNCION

    // analisis.nico (principal)
    PROGRAMA Analisis
        INCLUIR "archivos.nico"
        INCLUIR "estadisticas.nico"
        
        VARIABLE DECIMAL $media = 0.0
    BLOQUE PRINCIPAL
        LLAMAR A cargar_datos("datos.csv")
        CALCULAR EN $media = calcular_media(100.0, 10)
        ESCRIBIR("Media: $media") SALTO
    FIN PRINCIPAL
    FINAL

---

## 🔗 Ver también
- 02-sintaxis.md - Estructura base de programas Nico
- 11-funciones.md - Declaración de funciones con retorno
- 12-subprogramas.md - Declaración de subprogramas sin retorno
- 16-funciones-texto.md - Funciones de manipulación de texto

> 📚 Documentación validada con Nico v2.0.0. Ejemplos probados en Linux/Raspberry Pi/Windows.