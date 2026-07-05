#!/bin/bash
# compile.sh - Script de compilación estándar para Linux y Raspberry Pi.
set -e  # Salir inmediatamente si algún comando falla

echo "╔════════════════════════════════════════╗"
echo "║        Compilando Nico v2.0.0          ║"
echo "╚════════════════════════════════════════╝"
echo ""

# Limpieza previa: evitar ejecutar binario viejo si falla compilación
rm -f nico

# Lista de archivos fuente (actualizada a src/)
SOURCES="src/main.c src/lexer.c src/parser.c src/evaluator.c src/ast.c src/web.c"

# Flags base (consistentes en todas las plataformas)
CFLAGS="-std=gnu11 -Wall -Wextra -O2 -Isrc -Wno-stringop-truncation -Wno-unused-result"
LIBS="-lm -lsqlite3 -lpthread"

# Detectar si estamos en Raspberry Pi
if [ -f /proc/device-tree/model ]; then
    MODEL=$(tr -d '\0' < /proc/device-tree/model)
    if [[ "$MODEL" == *"Raspberry Pi"* ]]; then
        echo "🍓 Detectado: Raspberry Pi"
        echo "   Compilando con soporte GPIO (libgpiod)..."
        gcc $CFLAGS -DHAVE_LIBGPIOD -o nico $SOURCES src/nico_gpio.c src/nico_pwm.c $LIBS -lgpiod -lpthread
    else
        echo "🖥️  Detectado: $MODEL"
        echo "   Compilando sin soporte GPIO..."
        gcc $CFLAGS -o nico $SOURCES src/nico_gpio_stub.c src/nico_pwm_stub.c $LIBS -lpthread
    fi
else
    echo "🖥️  Detectado: PC Linux (no Raspberry Pi)"
    echo "   Compilando sin soporte GPIO..."
    gcc $CFLAGS -o nico $SOURCES src/nico_gpio_stub.c src/nico_pwm_stub.c $LIBS -lpthread
fi

# Verificar si compiló correctamente
if [ $? -eq 0 ]; then
    echo ""
    echo "✅ ¡Compilación exitosa!"
    echo "   Ejecutable: ./nico"
    echo ""
    echo "   Para ejecutar: ./nico programa.nico"
    echo "   Para modo interactivo: ./nico"
    echo ""
    echo "   💡 Tip: Para máxima recursión (ej: Ackermann), ejecutá:"
    echo "      ulimit -s unlimited"
    echo "      ./nico programa.nico"
    echo ""
    echo "   💡 Tip: Usa -DDEBUG para ver logs internos:"
    echo "      gcc -DDEBUG ... (o edita este script)"
else
    echo ""
    echo "❌ Error de compilación. Revisá los mensajes arriba."
    exit 1
fi