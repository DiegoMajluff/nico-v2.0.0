# 🌐 Servidor Web y Panel de Administración

Nico incluye un servidor HTTP embebido con panel SPA de administración, endpoint 
de renderizado dinámico y API JSON. No requiere frameworks externos, Node.js 
ni configuración de reverse proxies.

## 🚀 Iniciar el Servidor
El servidor corre en un hilo separado. Debés mantener el programa activo para 
atender peticiones:
```nico
INICIARSERVER(8080)
ESCRIBIR("🌐 Servidor activo en http://localhost:8080") SALTO

MIENTRAS(1) HACER
    ESPERAR(1, SEGUNDOS)
FIN MIENTRAS

DETENERSERVER()
```
## 📂 Endpoints Disponibles

| Ruta | Método | Función |
|------|--------|---------|
| `/admin` | GET | Panel SPA completo: CRUD, búsqueda, paginación, import/export CSV, detección visual de Foreign Keys. |
| `/render?archivo=ruta` | GET | Ejecuta un script .nico y devuelve su salida (ESCRIBIR) como text/html |
| `/api/schema/TABLA` | GET | Estructura JSON de la tabla (columnas, tipos, PKs, FKs). |
| `/api/foreign-keys/TABLA` | GET | Lista JSON de foreign keys de la tabla. |
| `/api/exec` | POST | Ejecuta SQL (INSERT, UPDATE, DELETE). Body = SQL string. |
| `/api/query` | POST | Ejecuta consulta SQL (SELECT). Body = SQL string. Retorna JSON con resultados. |

## 🔐 Panel de Administración (/admin)
- **Acceso:** `http://localhost:8080/admin`
- **Credenciales:** Usuario `admin` | Clave `nico2026`
- **Características:**
  - ✅ CRUD completo (Crear, Leer, Editar, Eliminar registros)
  - 🔍 Búsqueda por texto libre y filtros por columna
  - 📄 Paginación automática (configurable)
  - 📥📤 Import/Export de datos en formato `.csv`
  - 🔗 Detección automática de Foreign Keys y relaciones visuales
  - 🍪 Sesión basada en cookies HTTP (vencimiento: 24h o cierre de navegador)

> 💡 El panel se conecta a la BD que el script haya abierto con `CONECTARBD()`. Si no hay BD activa, mostrará un mensaje de configuración.

## 🎨 Renderizado Dinámico (/render)
Permite generar páginas HTML/CSS/JS desde scripts Nico. Ideal para prototipos, dashboards o generación de reportes.

**Ejemplo de script (`mi_pagina.nico`):**
```nico
ESCRIBIR("<!DOCTYPE html><html><head><meta charset='utf-8'><style>body{font-family:sans-serif;padding:30px}</style></head><body>") SALTO
ESCRIBIR("<h1>Reporte Generado</h1>") SALTO
ESCRIBIR("<p>¡Hola, mundo!</p>") SALTO
ESCRIBIR("</body></html>") SALTO
```
**Acceder:** `http://localhost:8080/render?archivo=mi_pagina.nico`

### ⚠️ Reglas Críticas para /render
- ❌ **NO** incluyas `INICIARSERVER` en el script renderizado. El endpoint ya corre en un proceso hijo; intentar iniciar otro servidor causará conflicto de puerto.
- ✅ Usá `ESCRIBIR` para todo el HTML/CSS/JS. El endpoint captura stdout, limpia secuencias ANSI y lo sirve como `text/html`.
- 📁 **Rutas relativas:** El `archivo=` se resuelve desde la carpeta donde ejecutaste `nico.exe` o `./nico`.
- 🔒 **Seguridad:** No validar parámetros de URL en el script renderizado. El endpoint solo acepta rutas de archivos locales.

```

## 📡 API JSON (/api/...)
Diseñada para integración con frontend externo o scripts de terceros.

### Ver esquema de una tabla

    curl http://localhost:8080/api/schema/usuarios

**Respuesta típica:**

    [
      {"name": "id", "type": "INTEGER", "pk": 1, "notnull": 0},
      {"name": "nombre", "type": "TEXT", "pk": 0, "notnull": 0},
      {"name": "email", "type": "TEXT", "pk": 0, "notnull": 0}
    ]

```

### Ver foreign keys

    curl http://localhost:8080/api/foreign-keys/productos

**Respuesta típica:**

    [
      {"column": "id_categoria", "table": "categorias", "to_column": "id"}
    ]

### Ejecutar SQL (POST)

    curl -X POST http://localhost:8080/api/exec -d "INSERT INTO usuarios (nombre) VALUES ('Diego')"

### Consultar datos (POST)

    curl -X POST http://localhost:8080/api/query -d "SELECT * FROM usuarios"

**Respuesta típica:**

    [
      {"id": 1, "nombre": "Diego"},
      {"id": 2, "nombre": "Ana"}
    ]
```
## 🛠️ Troubleshooting Rápido

**Problema:** /render muestra página en blanco
**Solución:** Verificá que el archivo exista y use ESCRIBIR para generar HTML

**Problema:** bind falló: Address already in use
**Solución:** Cambiá a INICIARSERVER(9090) o cerrá el proceso anterior

**Problema:** Panel /admin no carga JS/CSS
**Solución:** Ctrl+F5 (hard refresh) o verificá que el servidor no esté filtrando recursos

**Problema:** Alta CPU/ventilador acelerado
**Solución:** Aumentá ESPERAR(1, SEGUNDOS) en el loop principal

**Problema:** API retorna error 500
**Solución:** Asegurate de llamar CONECTARBD() antes de iniciar el servidor
```
## 📌 Notas de Arquitectura

- El servidor usa `select()` + hilos POSIX (`pthread`) para concurrencia ligera.
- Cada petición `/render` usa `popen()` para ejecutar el script y capturar su salida.
- **Sin dependencias externas:** todo corre en un solo binario de ~200-500 KB.
- Diseñado para entornos educativos, IoT ligero y prototipado rápido. No reemplaza servidores de producción bajo carga.
- Las credenciales del panel admin están hardcodeadas: usuario `admin`, clave `nico2026`.