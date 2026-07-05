# 🕒 Fecha y Hora del Sistema

Comandos para obtener la fecha y hora actuales del sistema operativo. Útiles para logging, generación de reportes, timestamps en archivos o UIs que muestran información temporal.

> 💡 **Requisito**: Ambos comandos requieren una `VARIABLE TEXTO` previamente declarada. Escriben el resultado por referencia directamente en la variable.

---

## 📋 Referencia de Comandos

| Comando | Sintaxis | Formato de Salida | Nota |
|---------|----------|-----------------|------|
| `HORAACTUAL` | `HORAACTUAL($destino)` | `"HH:MM:SS"` (24h) | Hora local del sistema. Sin ajuste de zona horaria. |
| `FECHAACTUAL` | `FECHAACTUAL($destino)` | `"DD/MM/AAAA"` | Fecha local. Día/mes/año con ceros iniciales si aplica. |

---

## 🧪 Ejemplo Mínimo Validado
```nico
PROGRAMA DemoFechaHora
    VARIABLE TEXTO $fecha = ""
    VARIABLE TEXTO $hora = ""
BLOQUE PRINCIPAL
    FECHAACTUAL($fecha)
    HORAACTUAL($hora)

    ESCRIBIR("Fecha actual: $fecha") SALTO
    ESCRIBIR("Hora actual:  $hora") SALTO

    // Uso en logging o nombres de archivo
    VARIABLE TEXTO EXTENSO $nombre_archivo = "backup_"
    CONCATENARTEXTO($nombre_archivo, $fecha)
    CONCATENARTEXTO($nombre_archivo, "_")
    CONCATENARTEXTO($nombre_archivo, $hora)
    ESCRIBIR("Nombre sugerido: $nombre_archivo.txt") SALTO
FIN PRINCIPAL
FINAL
```

---

## ⚠️ Notas Técnicas Críticas
- **Formato fijo:** La salida sigue estrictamente `DD/MM/AAAA` y `HH:MM:SS`. No hay opciones de personalización en v1.1.0.
- **Hora local:** Usa la configuración regional del SO. Si el sistema está en UTC, mostrará UTC. No hay conversión automática de zonas.
- **Actualización en tiempo real:** La función lee el reloj del sistema al momento de la llamada. No se actualiza sola; llamala nuevamente si necesitás un valor fresco.
- **Variables TEXTO:** Funciona con cualquier variable TEXTO declarada. El sistema maneja automáticamente el tamaño necesario.
- **Uso en nombres de archivo:** Los formatos usan `/` y `:`, caracteres inválidos en Windows para rutas. Reemplazalos antes de usar como nombre:
  ```nico
  // Ejemplo: convertir "24/05/2026 14:30:00" a "2026-05-24_14-30-00"
  // (requiere manipulación de texto, ver 02-cadenas.md)
  ```

---

## 🎯 Buenas Prácticas
```nico
// ✅ Obtener fecha/hora justo antes de usarla
HORAACTUAL($inicio)
// ... proceso largo ...
HORAACTUAL($fin)
// Calcular duración o loguear timestamp

// ✅ Usar en logs de auditoría
VARIABLE TEXTO EXTENSO $log_linea = "["
FECHAACTUAL($tmp_fecha)
CONCATENARTEXTO($log_linea, $tmp_fecha)
CONCATENARTEXTO($log_linea, " ")
HORAACTUAL($tmp_hora)
CONCATENARTEXTO($log_linea, $tmp_hora)
CONCATENARTEXTO($log_linea, "] Evento registrado")

// ❌ No asumir formato internacional (YYYY-MM-DD)
// Nico usa estándar local DD/MM/AAAA. Validá si integrás con APIs externas.
```

---

## 🔄 Equivalencias Multiplataforma

| Nico | C | Python | JavaScript |
|------|---|--------|------------|
| `FECHAACTUAL($f)` | `strftime("%d/%m/%Y", ...)` | `datetime.now().strftime("%d/%m/%Y")` | `new Date().toLocaleDateString()` |
| `HORAACTUAL($h)` | `strftime("%H:%M:%S", ...)` | `datetime.now().strftime("%H:%M:%S")` | `new Date().toLocaleTimeString()` |

> 💡 **Compatibilidad:** Funciona idéntico en Linux y Windows. Usa `localtime()` del runtime C, respetando la zona horaria configurada en el SO.