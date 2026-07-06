# 🛠️ Instalación y Compilación de Nico v2.0.0

Esta guía está diseñada para que **cualquier persona**, sin importar su experiencia previa, pueda compilar Nico correctamente. Vamos paso a paso, sin saltos, sin suposiciones y con explicaciones claras para cada acción.

> 💡 **Regla de oro:** Nico está diseñado para compilarse en la misma máquina donde se va a ejecutar. En Linux y Raspberry Pi, siempre compilamos desde cero. En Windows incluimos un ejecutable listo para usar, pero también te explicamos cómo compilarlo vos mismo si lo preferís.

---

## 🐧 Linux (Ubuntu, Debian, Mint, Fedora, etc.)

### Paso 1: Abrir la terminal
Buscá "Terminal" en tu menú de aplicaciones y abrila. Es una ventana negra donde escribirás comandos.

### Paso 2: Actualizar la lista de paquetes
Escribí esto exactamente y presioná `Enter`:

    sudo apt update

*(Si te pide contraseña, escribila con cuidado. **No verás asteriscos ni puntos** mientras tipeás, es una medida de seguridad. Presioná `Enter` al terminar).*

### Paso 3: Instalar herramientas de compilación y SQLite3
Copiá y pegá toda esta línea en la terminal y presioná `Enter`:

    sudo apt install -y build-essential libsqlite3-dev

Esperá a que termine. Verás un mensaje como `0 actualizados, X instalados...` al final. Esto instala `gcc` (el compilador) y los archivos necesarios para que Nico guarde datos.

### Paso 4: Verificar que se instaló correctamente
Escribí:

    gcc --version

Debería responder con algo como `gcc (Ubuntu 11.x.x) ...`. Si dice `comando no encontrado`, volvé al Paso 2 y repetí.

### Paso 5: Compilar Nico
En la terminal, navegá a la carpeta donde guardaste el proyecto:

    cd /ruta/a/tu/carpeta/nico-v2.0.0

*(Reemplazá `/ruta/a/tu/carpeta/` por la ruta real. Si usás Ubuntu, suele ser `~/Descargas/nico-v2.0.0` o similar).*

Ahora dale permisos de ejecución al script y compilá:

    chmod +x compile.sh
    ./compile.sh

Si todo sale bien, verás un mensaje de éxito y un nuevo archivo llamado `nico` aparecerá en la carpeta.

### Paso 5.5: Configurar límite de recursión (Opcional pero recomendado)
Para programas con recursión profunda (como Ackermann o algoritmos complejos), aumentá el límite de stack:

    ulimit -s unlimited

Esto permite hasta 7000+ niveles de recursión. El cambio es temporal (solo para la sesión actual). Para hacerlo permanente, agregá la línea a tu `~/.bashrc`:

    echo "ulimit -s unlimited" >> ~/.bashrc

> 💡 **Nota:** En Windows, el stack size ya está configurado en 64MB durante la compilación, no necesitás hacer nada adicional.

### Paso 6: Probar que funciona

    ./nico -e "2 + 2"

Si la terminal responde `4`, ¡ya está listo! 🎉

---

## 🍓 Raspberry Pi (Raspberry Pi OS)

Los pasos son casi idénticos a Linux, pero agregamos una librería extra para que Nico pueda controlar los pines físicos de la placa.

### Paso 1 a 3: Instalación con soporte GPIO
Abrí la terminal y ejecutá:
```bash
sudo apt update
sudo apt install -y build-essential libsqlite3-dev libgpiod-dev
```
`libgpiod-dev` es obligatorio si querés usar `CONFIGURARPIN`, `ESTADOPIN` o `LEERPIN`.

### Paso 4: Verificar
```bash
gcc --version
```

### Paso 5: Compilar
```bash
cd ~/nico-v2.0.0
chmod +x compile.sh
./compile.sh
```
El script detecta automáticamente que estás en una Raspberry Pi y habilita el soporte GPIO durante la compilación.

### Paso 6: Permisos para usar los pines (Importante)
Para que Nico pueda controlar el hardware sin tener que usar `sudo` todo el tiempo:
```bash
sudo usermod -a -G gpio $USER
```
**Reiniciá sesión** o reiniciá la Raspberry Pi para que el cambio surta efecto. Si no lo hacés, Nico te pedirá `sudo` cada vez que uses GPIO.

---
## 🪟 Windows 10 / 11

Windows es el sistema que requiere más pasos porque no trae compilador de C por defecto. No te preocupes, lo haremos juntos, paso a paso.

### ⚠️ Opción A: Usar el ejecutable listo (Recomendado para empezar rápido)
1. Abrí la carpeta `bin\windows\` dentro del proyecto.
2. Verás `nico.exe` y 3 archivos `.dll`.
3. **No los separes**. Los 4 archivos deben estar **siempre juntos en la misma carpeta**.
4. Abrí una terminal en esa carpeta (hacé clic derecho en un espacio vacío → "Abrir en Windows Terminal" o `cmd`), y escribí:

    nico.exe ..\..\ejemplos\01_hola_mundo.nico

Si funciona, ¡ya podés programar! Si querés compilar vos mismo o modificar el código, seguís a la Opción B.

### 🛠️ Opción B: Compilar desde cero (Guía detallada)

#### Paso 1: Descargar e instalar MSYS2
1. Abrí tu navegador y andá a: [https://www.msys2.org/](https://www.msys2.org/)
2. Hacé clic en el botón azul que dice **"Download Installer"**.
3. Guardá el archivo `.exe` y ejecutalo.
4. Seguí los pasos por defecto (`Next` → `Next` → `Install`). Se instalará en `C:\msys64`.

#### Paso 2: Actualizar MSYS2 (primera vez)
1. Al finalizar la instalación, se abrirá automáticamente la terminal **"MSYS2 MSYS"** (ícono negro).
2. En esa terminal, escribí:

    pacman -Syu

3. Te pedirá cerrar la terminal. Cerrala.
4. Volvé a abrir **"MSYS2 MSYS"** desde el menú Inicio.
5. Ejecutá de nuevo:

    pacman -Su

6. Cuando termine, cerrá esa terminal.

#### Paso 3: Instalar el compilador y SQLite3
1. Abrí el menú Inicio de Windows.
2. Escribí `MSYS2`.
3. Verás varias opciones. Hacé clic **SOLO** en **"MSYS2 MinGW x64"** (tiene un icono de terminal **azul**).

> 🚨 **Muy importante:** No abras "MSYS2 MSYS" (negro), "UCRT64" ni "CLANG64". Solo **"MSYS2 MinGW x64"** (azul). Si abrís otra, los comandos de más abajo fallarán.

4. En la terminal que se abrió, escribí:

    pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-make mingw-w64-x86_64-sqlite3

5. Presioná `Enter`. Si te pregunta `Proceed with installation? [Y/n]`, escribí `y` y presioná `Enter`.
6. Esperá a que descargue e instale todo (puede tardar 2-5 minutos dependiendo de tu conexión).

#### Paso 4: Verificar instalación
En la misma terminal **MSYS2 MinGW x64**, escribí:

    gcc --version
    sqlite3 --version
    ls /mingw64/include/sqlite3.h

Los tres comandos deben funcionar. El último te tiene que mostrar la ruta del header de SQLite3.

#### Paso 5: Agregar al PATH de Windows (Importante)
Para que Nico encuentre `sqlite3.dll` al ejecutar desde CMD normal:

1. Presioná `Windows + R`, escribí `sysdm.cpl` y presioná `Enter`.
2. Andá a la pestaña **"Opciones avanzadas"** → **"Variables de entorno"**.
3. En **"Variables del sistema"** (abajo), buscá `Path` → **"Editar"**.
4. Hacé clic en **"Nuevo"** y agregá:

    C:\msys64\mingw64\bin

5. Aceptá todo y cerrá.
6. **Importante:** Abrí una **nueva** terminal CMD (las terminales abiertas antes no verán el cambio).

#### Paso 6: Verificar desde CMD
Abrí una **nueva** terminal CMD y escribí:

    gcc --version
    sqlite3 --version

Ambos deben mostrar su versión. Si dice "no se reconoce como comando", volvé al Paso 5 y verificá la ruta.

#### Paso 7: Compilar Nico
1. En la terminal CMD, navegá a la carpeta de tu proyecto:

    cd C:\Users\TuNombre\nico-v2.0.0

   *(Reemplazá `TuNombre` por tu usuario real de Windows).*

2. Ejecutá el script de compilación:

    compile_windows.bat

3. Si todo sale bien, verás mensajes como `[OK] src\main.c` y al final `[OK] nico.exe generado correctamente.`

#### Paso 8: Probar que funciona
En la misma terminal CMD:

    nico.exe -e "2 + 2"

Si responde `4`, ¡ya está listo! 🎉

#### Paso 9: Distribuir o ejecutar sin MSYS2 abierto
El ejecutable se creará en la carpeta del proyecto como `nico.exe`.

Para moverlo a otra PC o ejecutarlo desde fuera de la terminal MSYS2, copiá estos 4 archivos a una misma carpeta:
- `nico.exe` (de tu carpeta del proyecto)
- `libgcc_s_seh-1.dll` (de `C:\msys64\mingw64\bin\`)
- `libsqlite3-0.dll` (de `C:\msys64\mingw64\bin\`)
- `libwinpthread-1.dll` (de `C:\msys64\mingw64\bin\`)

Sin estos DLLs, Windows te mostrará un error de "falta DLL".

---

## ✅ ¿Cómo sé que todo funcionó correctamente?

En cualquier sistema operativo, abrí una terminal, andá a la carpeta del proyecto y probá:
```bash
./nico -e "3 * 4 + 2"
```
Si responde `14`, Nico está compilado, vinculado con SQLite3 y listo para usarse. 🐧✨

> 📌 **Próximo paso:** Abrí `ejemplos/01_hola_mundo.nico` en un editor de texto, modificá el mensaje y ejecutalo con `./nico ejemplos/01_hola_mundo.nico`. ¡Bienvenido a Nico!