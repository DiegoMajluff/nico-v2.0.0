# 🎨 Extensión de VS Code para Nico

Esta guía explica cómo instalar y configurar la extensión oficial de Nico para Visual Studio Code, que proporciona resaltado de sintaxis con colores personalizados.

## 📦 ¿Qué incluye la extensión?

- ✅ Resaltado de sintaxis para archivos `.nico`
- ✅ Colores personalizados para keywords, tipos, funciones y constantes
- ✅ Soporte para comentarios, strings, números y variables
- ✅ Folding (plegado) de bloques de código
- ✅ Auto-cierre de brackets y comillas

## 🐧 Instalación en Linux

### Paso 1: Clonar o descargar el repositorio

    git clone https://github.com/DiegoMajluff/nico-v2.0.0.git
    cd nico-v2.0.0

O descargá el ZIP desde GitHub y extraelo.

### Paso 2: Instalar la extensión

    code --install-extension nico-vscode-extension/nico-language-1.0.0.vsix

### Paso 3: Configurar colores personalizados

Abrí la configuración de usuario de VS Code:

    nano ~/.config/Code/User/settings.json

Agregá esto al archivo (si ya tenés contenido, integrá la sección `"editor.tokenColorCustomizations"` con lo que ya tengas):

    {
        "editor.tokenColorCustomizations": {
            "textMateRules": [
                {
                    "scope": "storage.type.nico",
                    "settings": {
                        "foreground": "#4EC9B0"
                    }
                },
                {
                    "scope": "entity.name.function.nico",
                    "settings": {
                        "foreground": "#DCDCAA"
                    }
                },
                {
                    "scope": "constant.language.nico",
                    "settings": {
                        "foreground": "#569CD6"
                    }
                }
            ]
        }
    }

Guardá con `Ctrl+O`, `Enter`, `Ctrl+X`.

### Paso 4: Recargar VS Code

    code --reload-window

O desde VS Code: `Ctrl+Shift+P` → `Developer: Reload Window`


## 🪟 Instalación en Windows

### Paso 1: Descargar la extensión

1. Andá a: https://github.com/DiegoMajluff/nico-v2.0.0/tree/master/nico-vscode-extension
2. Hacé clic en `nico-language-1.0.0.vsix`
3. Clic derecho → "Save link as..." o "Guardar enlace como..."
4. Guardalo en cualquier carpeta, por ejemplo `C:\Users\TuUsuario\Downloads\`

### Paso 2: Instalar la extensión en VS Code

1. Abrí VS Code
2. Presioná `Ctrl+Shift+P`
3. Escribí: `Extensions: Install from VSIX...`
4. Navegá hasta el archivo `nico-language-1.0.0.vsix` que descargaste
5. Seleccioná el archivo y hacé clic en "Install"

### Paso 3: Configurar colores personalizados

1. En VS Code, presioná `Ctrl+Shift+P`
2. Escribí: `Preferences: Open User Settings (JSON)`
3. Se va a abrir el archivo `settings.json`
4. Agregá esto al final del archivo (antes del `}` de cierre):

        ,
        "editor.tokenColorCustomizations": {
            "textMateRules": [
                {
                    "scope": "storage.type.nico",
                    "settings": {
                        "foreground": "#4EC9B0"
                    }
                },
                {
                    "scope": "entity.name.function.nico",
                    "settings": {
                        "foreground": "#DCDCAA"
                    }
                },
                {
                    "scope": "constant.language.nico",
                    "settings": {
                        "foreground": "#569CD6"
                    }
                }
            ]
        }

**Importante**: Si ya tenés contenido en `settings.json`, asegurate de agregar una coma `,` después del último elemento antes de agregar `"editor.tokenColorCustomizations"`.

5. Guardá el archivo con `Ctrl+S`

### Paso 4: Recargar VS Code

`Ctrl+Shift+P` → `Developer: Reload Window` → `Enter`


## 🎨 Colores configurados

La extensión usa estos colores (optimizados para el tema "Visual Studio Dark"):

| Elemento | Color | Ejemplos |
|----------|-------|----------|
| **Tipos de datos** | Verde azulado (#4EC9B0) | ENTERA, DECIMAL, TEXTO, LISTA, MATRIZ |
| **Funciones nativas** | Amarillo (#DCDCAA) | ESCRIBIR, LEER, SIGMOIDE, COSENO |
| **Constantes** | Azul (#569CD6) | VERDADERO, FALSO, SALTO, PI |
| **Keywords** | Morado/Azul (del tema) | PROGRAMA, SI, MIENTRAS, FUNCION |
| **Variables** | Cyan (del tema) | $nombre, $contador, $resultado |
| **Números** | Azul claro (del tema) | 123, 45.67 |
| **Strings** | Verde (del tema) | "Hola mundo" |
| **Comentarios** | Gris (del tema) | // Esto es un comentario |


## 📁 Estructura de archivos

    nico-vscode-extension/
    ├── language-configuration.json    # Configuración de brackets, comentarios, folding
    ├── package.json                   # Manifiesto de la extensión
    ├── nico-language-1.0.0.vsix      # Extensión empaquetada (instalable)
    └── syntaxes/
        └── nico.tmLanguage.json       # Definición de sintaxis y scopes

## 🛠️ Desarrollo y personalización

Si querés modificar los colores o agregar más palabras clave:

1. Editá `nico-vscode-extension/syntaxes/nico.tmLanguage.json`
2. Reempaquetá la extensión:

        cd nico-vscode-extension
        vsce package

3. Desinstalá la versión anterior en VS Code
4. Instalá el nuevo `.vsix` siguiendo los pasos de instalación

## 🔄 Actualizar la extensión

Cuando se actualice la sintaxis en el repositorio:

1. Descargá el nuevo `nico-language-1.0.0.vsix` del repositorio
2. En VS Code: `Ctrl+Shift+P` → `Extensions: Show Installed Extensions`
3. Buscá "Nico Language" y hacé clic en el ícono de desinstalar
4. Volvé a instalar el nuevo `.vsix` siguiendo los pasos de instalación

## ✅ Verificar que funciona

1. Abrí cualquier archivo `.nico` (por ejemplo `ejemplos/01_hola_mundo.nico`)
2. Debería tener colores automáticamente
3. Verificá en la esquina inferior derecha que diga "Nico" (no "Plain Text")
4. Si dice "Plain Text", hacé clic ahí y seleccioná "Nico"

## 🐛 Solución de problemas

### El código no tiene colores

- Verificá que la extensión esté instalada: `Ctrl+Shift+P` → `Extensions: Show Installed Extensions` → buscá "Nico Language"
- Recargá VS Code: `Ctrl+Shift+P` → `Developer: Reload Window`
- Verificá que el archivo tenga extensión `.nico`

### Los colores no son los esperados

- Verificá que `settings.json` tenga la sección `"editor.tokenColorCustomizations"` correctamente formateada
- Probá con otro tema de VS Code (la extensión está optimizada para "Visual Studio Dark")

### VS Code no reconoce archivos .nico

- Verificá que la extensión esté instalada
- Abrí `settings.json` y agregá:

        "files.associations": {
            "*.nico": "nico"
        }

---

> 📚 **Material educativo elaborado con asistencia de Qwen AI (Alibaba Cloud)**
> 
> **Autor:** Diego Alejandro Majluff | 2026 | MIT
