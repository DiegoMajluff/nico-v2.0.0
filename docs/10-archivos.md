# 📁 Manejo de Archivos

Comandos para lectura, escritura y manipulación de archivos de texto en disco. Usa el runtime estándar de C con abstracción segura mediante variables `VARIABLE ARCHIVO`.

> 💡 **Nota**: Todos los comandos operan sobre archivos de texto plano. Para binarios, se recomienda manejo directo de bytes en C (fuera del alcance de Nico v1.1.0).

---

## 📋 Referencia de Comandos

| Comando | Sintaxis | Ejemplo | Nota |
|---------|----------|---------|------|
| `ABRIRARCHIVO` | `ABRIRARCHIVO($archivo, "ruta", MODO)` | `ABRIRARCHIVO($f, "datos.txt", ESCRITURA)` | `$archivo` debe ser `VARIABLE ARCHIVO`. `MODO` es constante sin comillas. |
| `ESCRIBIRARCHIVO` | `ESCRIBIRARCHIVO($archivo, $texto)` | `ESCRIBIRARCHIVO($f, "Hola")` | Agrega salto de línea `\n` automáticamente al final. |
| `LEERARCHIVO` | `LEERARCHIVO($archivo, $destino)` | `LEERARCHIVO($f, $linea)` | Lee 1 línea hasta `\n`. Remueve el salto final. `$destino` es `TEXTO`. |
| `CERRARARCHIVO` | `CERRARARCHIVO($archivo)` | `CERRARARCHIVO($f)` | Libera descriptor. Hace flush implícito. Obligatorio tras usar el archivo. |

---

## 🗝️ Modos de Apertura (Constantes)

| Modo (Nico) | Equivalente C | Comportamiento | ¿Crea si no existe? |
|-------------|--------------|----------------|---------------------|
| `LECTURA` | `"r"` | Solo lectura. El archivo debe existir. | ❌ No (error si falta) |
| `ESCRITURA` | `"w"` | Escritura pura. **Trunca si ya existe**. | ✅ Sí |
| `AGREGAR` | `"a"` | Escritura al final (append). No trunca. | ✅ Sí |
| `LECTOESCRITURA` | `"r+"` | Lectura y escritura. El archivo debe existir. | ❌ No |

> ⚠️ **Regla de sintaxis**: Los modos se escriben en **mayúsculas y sin comillas**. `ABRIRARCHIVO($f, "x.txt", ESCRITURA)` ✅.

---

## 🧪 Ejemplo Mínimo Validado
```nico
PROGRAMA ArchivosYTexto
    VARIABLE ARCHIVO $archivo
    VARIABLE TEXTO $leido = ""
BLOQUE PRINCIPAL
    // 1. Escritura
    ABRIRARCHIVO($archivo, "registro.txt", ESCRITURA)
    ESCRIBIRARCHIVO($archivo, "Línea 1")
    ESCRIBIRARCHIVO($archivo, "Línea 2")
    CERRARARCHIVO($archivo)

    // 2. Lectura secuencial
    ABRIRARCHIVO($archivo, "registro.txt", LECTURA)
    LEERARCHIVO($archivo, $leido)
    ESCRIBIR("Leído 1: $leido") SALTO
    LEERARCHIVO($archivo, $leido)
    ESCRIBIR("Leído 2: $leido") SALTO
    CERRARARCHIVO($archivo)
FIN PRINCIPAL
FINAL
```

---

## ⚠️ Notas Técnicas Críticas
- **Variable ARCHIVO:** Tipo especial que encapsula `FILE*` de C. No puede usarse como `TEXTO` ni `ENTERO`. Debe declararse explícitamente.
- **Saltos de línea:** `ESCRIBIRARCHIVO` agrega `\n` automáticamente. `LEERARCHIVO` remueve el `\n` final al leer. El valor en `$destino` está limpio.
- **Buffering:** La escritura en disco puede estar bufferizada. `CERRARARCHIVO` hace flush implícito.
- **Rutas relativas:** Se resuelven desde el directorio donde ejecutás `nico`. Para rutas absolutas, usá sintaxis del SO o `/` para portabilidad.
- **Error handling:** Si `ABRIRARCHIVO` falla, Nico imprime en stderr y continúa. Validá con `ARCHIVOVALIDO($archivo)` si tu build lo soporta.

---

## 🎯 Buenas Prácticas
```nico
// ✅ Usar AGREGAR para logs acumulativos
ABRIRARCHIVO($log, "app.log", AGREGAR)
ESCRIBIRARCHIVO($log, "Evento registrado")
CERRARARCHIVO($log)

// ✅ Leer archivo completo línea por línea
VARIABLE TEXTO $linea = ""
ABRIRARCHIVO($f, "datos.txt", LECTURA)
MIENTRAS(1) HACER
    LEERARCHIVO($f, $linea)
    SI(TEXTOVACIO($linea)) ENTONCES 
        CORTE 
    FIN SI
    ESCRIBIR("→ $linea") SALTO
FIN MIENTRAS
CERRARARCHIVO($f)

// ❌ Evitar abrir sin cerrar (pérdida de datos o descriptores agotados)
ABRIRARCHIVO($f, "x.txt", ESCRITURA)
ESCRIBIRARCHIVO($f, "dato")
// ← Olvidar CERRARARCHIVO puede perder el último write por buffering
```

---

## 🔄 Equivalencias C Internas

| Nico | C Runtime | Descripción |
|------|-----------|-------------|
| `ABRIRARCHIVO(..., LECTURA)` | `fopen(..., "r")` | Apertura solo lectura |
| `ABRIRARCHIVO(..., ESCRITURA)` | `fopen(..., "w")` | Escritura con truncado |
| `ESCRIBIRARCHIVO($f, "txt")` | `fprintf(f, "%s\n", "txt")` | Write + newline |
| `LEERARCHIVO($f, $buf)` | `fgets(buf, MAX, f) + strip \n` | Read line + cleanup |
| `CERRARARCHIVO($f)` | `fclose(f)` | Close + flush |

---

## 🪟 Compatibilidad Windows/Linux

| Aspecto | Linux/macOS | Windows |
|---------|-------------|---------|
| Separador de rutas | `/` | `\` (escapar: `\\`) o `/` (funciona) |
| Nombres de archivo | Case-sensitive | Case-insensitive |
| Permisos | `chmod` relevante | ACLs / atributos de solo lectura |
| Rutas absolutas | `/home/user/x.txt` | `C:\Users\user\x.txt` |

> 💡 **Portabilidad:** Nico normaliza rutas internamente. Usá `/` incluso en Windows para máxima portabilidad: `ABRIRARCHIVO($f, "C:/datos/x.txt", LECTURA)`