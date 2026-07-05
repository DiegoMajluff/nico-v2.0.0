# 🗄️ Base de Datos SQLite

Nico incluye soporte nativo para bases de datos SQLite. No requiere instalación externa, drivers ni configuración de rutas.

## 🔗 Conexión y Ejecución Básica

    // Abrir o crear la base de datos en disco
    CONECTARBD("tienda.db")
    
    // Ejecutar comandos DDL/DML (CREATE, INSERT, UPDATE, DELETE)
    EJECUTARBD("CREATE TABLE IF NOT EXISTS productos (id INTEGER PRIMARY KEY, nombre TEXT, precio REAL)")
    EJECUTARBD("INSERT INTO productos (nombre, precio) VALUES ('Teclado', 1500.50)")
    
    // Cerrar conexión (opcional, Nico la cierra automáticamente al finalizar)
    CERRARBD()

    ## 🔍 Consultas y Variables de Resultado

Usá SQL estándar directamente. Nico no traduce sintaxis en español a SQL.

    // Consulta con SELECT, FROM, WHERE (SQL estándar)
    CONSULTARBD("SELECT nombre, precio FROM productos WHERE precio > 1000")
    
    MIENTRAS(SIGUIENTEFILABD()) HACER
        ESCRIBIR("Nombre: $BDCOL1 | Precio: $BDCOL2") SALTO
    FIN MIENTRAS
    
    CERRARCONSULTABD()

💡 `$BDCOL1`, `$BDCOL2`, etc. acceden a las columnas en el orden exacto de la consulta.

### Ejemplo Completo: JOIN

    CONSULTARBD("SELECT c.nombre, p.precio FROM categorias c INNER JOIN productos p ON c.id = p.id_categoria ORDER BY c.nombre")
    
    MIENTRAS(SIGUIENTEFILABD()) HACER
        ESCRIBIR("Categoría: $BDCOL1 | Precio: $BDCOL2") SALTO
    FIN MIENTRAS
    
    CERRARCONSULTABD()

    ## 🔒 Parámetros Seguros

Usá `?` como placeholder para valores dinámicos. Esto evita inyección SQL:

    // ✅ Correcto: usar parámetros
    EJECUTARBD("INSERT INTO productos (nombre, precio) VALUES (?, ?)", "Teclado", 1500.50)
    
    // ❌ Evitar: concatenar strings
    EJECUTARBD("INSERT INTO productos (nombre, precio) VALUES ('Teclado', 1500.50)")

    ## ⚠️ Persistencia y Buenas Prácticas

- ✅ Los archivos `.db` se guardan en disco y sobreviven entre ejecuciones.
- ✅ Usá `CREATE TABLE IF NOT EXISTS` para evitar errores en inicializaciones repetidas.
- ⚠️ Evitá `DELETE FROM tabla` o `DROP TABLE` en scripts de producción a menos que sea intencional.
- 🧪 Para entornos de prueba, podés usar `EJECUTARBD("DELETE FROM tabla")` para limpiar datos antes de ejecutar.

## 🔑 Foreign Keys (Claves Foráneas)

Nico respeta el esquema nativo de SQLite. Si definís `FOREIGN KEY`, el panel web las detecta automáticamente y las muestra como relaciones visuales entre tablas, sin configuración extra.

    EJECUTARBD("CREATE TABLE categorias (id INTEGER PRIMARY KEY, nombre TEXT)")
    EJECUTARBD("CREATE TABLE productos (id INTEGER PRIMARY KEY, nombre TEXT, id_categoria INTEGER, FOREIGN KEY(id_categoria) REFERENCES categorias(id))")

    ## 🔄 Múltiples Consultas

Nico solo mantiene **un cursor activo** a la vez. Si necesitás ejecutar otra consulta, primero cerrá la anterior:

    // Primera consulta
    CONSULTARBD("SELECT nombre FROM usuarios")
    MIENTRAS(SIGUIENTEFILABD()) HACER
        ESCRIBIR("Usuario: $BDCOL1") SALTO
    FIN MIENTRAS
    CERRARCONSULTABD()
    
    // Segunda consulta (después de cerrar la primera)
    CONSULTARBD("SELECT titulo FROM libros")
    MIENTRAS(SIGUIENTEFILABD()) HACER
        ESCRIBIR("Libro: $BDCOL1") SALTO
    FIN MIENTRAS
    CERRARCONSULTABD()

    