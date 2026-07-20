/*
 * Nico v2.0.0 - Intérprete Educativo de Scripting en Español
 * @file:         evaluator.c
 * @author:       Diego Alejandro Majluff (Diseño, Arquitectura y Supervisión)
 * @ai_assist:    Qwen (Alibaba Cloud) - Implementación, Debugging y Optimización
 * @license:      MIT / Personal Use (ver LICENSE)
 * @description:  Implementación del evaluador (intérprete AST). Recorre el árbol
 *                de sintaxis abstracta y ejecuta cada nodo. Maneja variables,
 *                operaciones aritméticas/lógicas, control de flujo, llamadas a
 *                funciones, I/O, hardware (GPIO/PWM), SQLite y servidor web.
 *                Incluye gestión de scope, recursión y manejo de errores.
 */
#include "evaluator.h"
#include "lexer.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <time.h>
#include <ctype.h>
#include <sqlite3.h>
#include "web.h"

// Cross-platform
#ifdef _WIN32
    #include <windows.h>
    #include <conio.h>
#else
    #include <sys/ioctl.h>
    #include <sys/select.h>
    #include <fcntl.h>
    #include <poll.h>
    #include <signal.h>
    #include <termios.h>
    #include <unistd.h>
#endif

static bool tipo_valor_compatible(TipoDato tipo_declarado, TipoValor tipo_valor);
// Declaraciones de funciones GPIO
extern void procesar_gpio_configurar(int pin, int direccion, int bias);
extern void procesar_gpio_estado_pin(int pin, int valor);
extern int procesar_gpio_leer(int pin);
extern int pwm_configurar(int pin, unsigned int frecuencia, int duty_cycle);
extern int pwm_detener(int pin);

// ============================================================
// BUFFER GLOBAL DE TECLADO
// ============================================================
static unsigned char teclado_buffer[256];
static int teclado_buffer_len = 0;
static int teclado_modo_raw = 0;
static struct termios teclado_config_original;

void teclado_iniciar_modo_raw(void)
{
    if (teclado_modo_raw)
        return;
#ifdef _WIN32
    teclado_modo_raw = 1;
#else
    tcgetattr(STDIN_FILENO, &teclado_config_original);
    struct termios newt = teclado_config_original;
    cfmakeraw(&newt);
    newt.c_cc[VMIN] = 0;
    newt.c_cc[VTIME] = 0;
    newt.c_oflag |= OPOST | ONLCR;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    teclado_modo_raw = 1;
#endif
}

void teclado_actualizar_buffer(void)
{
    if (!teclado_modo_raw)
        teclado_iniciar_modo_raw();

    teclado_buffer_len = 0;

#ifdef _WIN32
    while (_kbhit() && teclado_buffer_len < 256)
    {
        unsigned char ch = _getch();
        if (ch == 0 || ch == 224)
        {
            if (_kbhit())
            {
                unsigned char ch2 = _getch();
                int key_code = 0;
                switch (ch2)
                {
                case 72:
                    key_code = 1001;
                    break;
                case 80:
                    key_code = 1002;
                    break;
                case 77:
                    key_code = 1003;
                    break;
                case 75:
                    key_code = 1004;
                    break;
                case 71:
                    key_code = 1005;
                    break;
                case 79:
                    key_code = 1006;
                    break;
                case 82:
                    key_code = 1007;
                    break;
                case 83:
                    key_code = 1008;
                    break;
                case 73:
                    key_code = 1009;
                    break;
                case 81:
                    key_code = 1010;
                    break;
                default:
                    key_code = ch2;
                    break;
                }
                if (key_code > 255)
                {
                    if (teclado_buffer_len < 255)
                    {
                        teclado_buffer[teclado_buffer_len++] = (key_code >> 8) & 0xFF;
                        teclado_buffer[teclado_buffer_len++] = key_code & 0xFF;
                    }
                }
                else
                {
                    teclado_buffer[teclado_buffer_len++] = key_code;
                }
            }
        }
        else
        {
            if (ch == 3)
            {
                teclado_restaurar_modo();
                printf("\n> Programa interrumpido por Ctrl+C.\n");
                fflush(stdout);
                exit(0);
            }
            teclado_buffer[teclado_buffer_len++] = ch;
        }
    }
#else
    // Linux: lectura directa y simple
    struct pollfd pfd = {.fd = STDIN_FILENO, .events = POLLIN};

    if (poll(&pfd, 1, 0) <= 0)
        return;

    char buf[8] = {0};
    ssize_t n = read(STDIN_FILENO, buf, sizeof(buf) - 1);
    if (n <= 0)
        return;

    buf[n] = '\0';

    // Ctrl+C
    if (n == 1 && buf[0] == 3)
    {
        teclado_restaurar_modo();
        printf("\n> Programa interrumpido por Ctrl+C.\n");
        fflush(stdout);
        exit(0);
    }

    // Carácter simple
    if (n == 1)
    {
        if (teclado_buffer_len < 256)
            teclado_buffer[teclado_buffer_len++] = (unsigned char)buf[0];
        return;
    }

    // Secuencia ANSI: ESC [ código
    if (n >= 3 && buf[0] == '\x1B' && buf[1] == '[')
    {
        int key_code = 0;
        switch (buf[2])
        {
        case 'A':
            key_code = 1001;
            break;
        case 'B':
            key_code = 1002;
            break;
        case 'C':
            key_code = 1003;
            break;
        case 'D':
            key_code = 1004;
            break;
        case 'H':
            key_code = 1005;
            break;
        case 'F':
            key_code = 1006;
            break;
        case '2':
            if (n >= 4 && buf[3] == '~')
                key_code = 1007;
            break;
        case '3':
            if (n >= 4 && buf[3] == '~')
                key_code = 1008;
            break;
        case '5':
            if (n >= 4 && buf[3] == '~')
                key_code = 1009;
            break;
        case '6':
            if (n >= 4 && buf[3] == '~')
                key_code = 1010;
            break;
        }

        if (key_code > 0 && teclado_buffer_len < 254)
        {
            teclado_buffer[teclado_buffer_len++] = (key_code >> 8) & 0xFF;
            teclado_buffer[teclado_buffer_len++] = key_code & 0xFF;
        }
        return;
    }

    // Otros caracteres
    for (ssize_t i = 0; i < n && teclado_buffer_len < 256; i++)
    {
        teclado_buffer[teclado_buffer_len++] = (unsigned char)buf[i];
    }
#endif
}

void teclado_limpiar_buffer(void)
{
    teclado_buffer_len = 0;
}

void teclado_restaurar_modo(void)
{
    if (teclado_modo_raw)
    {
#ifndef _WIN32
        tcsetattr(STDIN_FILENO, TCSANOW, &teclado_config_original);
#endif
        teclado_modo_raw = 0;
    }
}

static Simbolo *tabla_simbolos_buscar_simbolo(TablaSimbolos *tabla, const char *nombre);
// ============================================================
// FUNCIONES AUXILIARES
// ============================================================
#ifndef _WIN32
struct termios *obtener_terminal_original(void)
{
    static struct termios oldt;
    static int inicializado = 0;
    if (!inicializado)
    {
        tcgetattr(STDIN_FILENO, &oldt);
        inicializado = 1;
    }
    return &oldt;
}
#else
void *obtener_terminal_original(void)
{
    return NULL;
}
#endif

// Función auxiliar para extraer el nombre de una variable desde el AST
static char *extraer_nombre_variable(NodoAST *nodo)
{
    if (!nodo)
        return NULL;

    if (nodo->tipo == AST_VARIABLE)
    {
        return nodo->datos.variable.nombre;
    }

    return NULL;
}

// Función auxiliar para asignar texto a variable, lista o matriz
static void asignar_texto_destino(NodoAST *destino, const char *texto, Contexto *ctx)
{
    if (!destino)
        return;

    if (destino->tipo == AST_VARIABLE)
    {
        // Variable simple
        Simbolo *sim = tabla_simbolos_buscar_simbolo(ctx->tabla_actual, destino->datos.variable.nombre);
        if (sim)
        {
            free(sim->valor.datos.texto);
            sim->valor.datos.texto = strdup(texto);
        }
    }
    else if (destino->tipo == AST_ACCESO_LISTA)
    {
        // Acceso a lista: $lista[indice]
        Valor indice = evaluar_nodo(destino->datos.acceso_lista.indice, ctx);
        if (ctx->hay_error)
            return;

        long long idx = 0;
        if (indice.tipo == VALOR_ENTERO)
            idx = indice.datos.entero;
        else if (indice.tipo == VALOR_DECIMAL)
            idx = (long long)indice.datos.decimal;
        valor_destruir(&indice);

        Simbolo *sim = tabla_simbolos_buscar_simbolo(ctx->tabla_actual, destino->datos.acceso_lista.nombre_lista);
        if (sim && sim->valor.tipo == VALOR_LISTA && idx >= 0 && idx < sim->valor.tamano)
        {
            free(sim->valor.datos.lista[idx].datos.texto);
            sim->valor.datos.lista[idx].datos.texto = strdup(texto);
        }
    }
    else if (destino->tipo == AST_ACCESO_MATRIZ)
    {
        // Acceso a matriz: $matriz[fila][columna]
        Valor fila = evaluar_nodo(destino->datos.acceso_matriz.fila, ctx);
        if (ctx->hay_error)
            return;
        Valor columna = evaluar_nodo(destino->datos.acceso_matriz.columna, ctx);
        if (ctx->hay_error)
            return;

        long long f = 0, c = 0;
        if (fila.tipo == VALOR_ENTERO)
            f = fila.datos.entero;
        else if (fila.tipo == VALOR_DECIMAL)
            f = (long long)fila.datos.decimal;
        if (columna.tipo == VALOR_ENTERO)
            c = columna.datos.entero;
        else if (columna.tipo == VALOR_DECIMAL)
            c = (long long)columna.datos.decimal;

        valor_destruir(&fila);
        valor_destruir(&columna);

        Simbolo *sim = tabla_simbolos_buscar_simbolo(ctx->tabla_actual, destino->datos.acceso_matriz.nombre_matriz);
        if (sim && sim->valor.tipo == VALOR_MATRIZ &&
            f >= 0 && f < sim->valor.filas && c >= 0 && c < sim->valor.columnas)
        {
            free(sim->valor.datos.matriz[f][c].datos.texto);
            sim->valor.datos.matriz[f][c].datos.texto = strdup(texto);
        }
    }
}

// Función auxiliar para obtener el texto actual de un destino
static char *obtener_texto_destino(NodoAST *destino, Contexto *ctx)
{
    if (!destino)
        return NULL;

    if (destino->tipo == AST_VARIABLE)
    {
        Simbolo *sim = tabla_simbolos_buscar_simbolo(ctx->tabla_actual, destino->datos.variable.nombre);
        if (sim && sim->valor.datos.texto)
            return sim->valor.datos.texto;
    }
    else if (destino->tipo == AST_ACCESO_LISTA)
    {
        Valor indice = evaluar_nodo(destino->datos.acceso_lista.indice, ctx);
        if (ctx->hay_error)
            return NULL;

        long long idx = 0;
        if (indice.tipo == VALOR_ENTERO)
            idx = indice.datos.entero;
        else if (indice.tipo == VALOR_DECIMAL)
            idx = (long long)indice.datos.decimal;
        valor_destruir(&indice);

        Simbolo *sim = tabla_simbolos_buscar_simbolo(ctx->tabla_actual, destino->datos.acceso_lista.nombre_lista);
        if (sim && sim->valor.tipo == VALOR_LISTA && idx >= 0 && idx < sim->valor.tamano)
            return sim->valor.datos.lista[idx].datos.texto;
    }
    else if (destino->tipo == AST_ACCESO_MATRIZ)
    {
        Valor fila = evaluar_nodo(destino->datos.acceso_matriz.fila, ctx);
        if (ctx->hay_error)
            return NULL;
        Valor columna = evaluar_nodo(destino->datos.acceso_matriz.columna, ctx);
        if (ctx->hay_error)
            return NULL;

        long long f = 0, c = 0;
        if (fila.tipo == VALOR_ENTERO)
            f = fila.datos.entero;
        else if (fila.tipo == VALOR_DECIMAL)
            f = (long long)fila.datos.decimal;
        if (columna.tipo == VALOR_ENTERO)
            c = columna.datos.entero;
        else if (columna.tipo == VALOR_DECIMAL)
            c = (long long)columna.datos.decimal;

        valor_destruir(&fila);
        valor_destruir(&columna);

        Simbolo *sim = tabla_simbolos_buscar_simbolo(ctx->tabla_actual, destino->datos.acceso_matriz.nombre_matriz);
        if (sim && sim->valor.tipo == VALOR_MATRIZ &&
            f >= 0 && f < sim->valor.filas && c >= 0 && c < sim->valor.columnas)
            return sim->valor.datos.matriz[f][c].datos.texto;
    }

    return NULL;
}

static bool tipo_valor_compatible(TipoDato tipo_declarado, TipoValor tipo_valor)
{
    switch (tipo_declarado)
    {
    case TIPO_ENTERO:
        // Aceptar ENTERO, ENTERO_SIN_SIGNO, y DECIMAL (la conversión se maneja en asignar)
        if (tipo_valor == VALOR_ENTERO || tipo_valor == VALOR_ENTERO_SIN_SIGNO)
            return true;
        if (tipo_valor == VALOR_DECIMAL || tipo_valor == VALOR_DECIMAL_SIN_SIGNO)
            return true;
        return false;

    case TIPO_ENTERO_SIN_SIGNO:
        return tipo_valor == VALOR_ENTERO_SIN_SIGNO ||
               tipo_valor == VALOR_ENTERO ||
               tipo_valor == VALOR_DECIMAL ||
               tipo_valor == VALOR_DECIMAL_SIN_SIGNO;

    case TIPO_DECIMAL:
        return tipo_valor == VALOR_DECIMAL ||
               tipo_valor == VALOR_DECIMAL_SIN_SIGNO ||
               tipo_valor == VALOR_ENTERO ||
               tipo_valor == VALOR_ENTERO_SIN_SIGNO;

    case TIPO_DECIMAL_SIN_SIGNO:
        return tipo_valor == VALOR_DECIMAL_SIN_SIGNO ||
               tipo_valor == VALOR_DECIMAL ||
               tipo_valor == VALOR_ENTERO_SIN_SIGNO ||
               tipo_valor == VALOR_ENTERO;

    case TIPO_CARACTER:
        return tipo_valor == VALOR_CARACTER ||
               tipo_valor == VALOR_CARACTER_SIN_SIGNO ||
               tipo_valor == VALOR_ENTERO ||         
               tipo_valor == VALOR_ENTERO_SIN_SIGNO; 

    case TIPO_CARACTER_SIN_SIGNO:
        return tipo_valor == VALOR_CARACTER_SIN_SIGNO ||
               tipo_valor == VALOR_CARACTER ||
               tipo_valor == VALOR_ENTERO ||         
               tipo_valor == VALOR_ENTERO_SIN_SIGNO; 

    case TIPO_TEXTO:
        return tipo_valor == VALOR_TEXTO || tipo_valor == VALOR_TEXTO_EXTENSO;

    case TIPO_TEXTO_EXTENSO:
        return tipo_valor == VALOR_TEXTO_EXTENSO || tipo_valor == VALOR_TEXTO;

    case TIPO_LOGICA:
        return tipo_valor == VALOR_LOGICA;

    case TIPO_ARCHIVO:
        return tipo_valor == VALOR_ARCHIVO;

    case TIPO_LISTA:
        return tipo_valor == VALOR_LISTA;

    case TIPO_MATRIZ:
        return tipo_valor == VALOR_MATRIZ;

    case TIPO_VACIO:
        return tipo_valor == VALOR_VACIO;
    }
    return false;
}

static void contexto_set_error(Contexto *ctx, const char *mensaje)
{
    ctx->hay_error = true;
    // Truncar mensaje a 240 caracteres para asegurar que quepa en el buffer de 512 bytes
    char msg_seguro[256];
    snprintf(msg_seguro, sizeof(msg_seguro), "%.240s", mensaje ? mensaje : "");
    snprintf(ctx->mensaje_error, sizeof(ctx->mensaje_error),
             "Línea %d: %s", ctx->linea_actual, msg_seguro);
}

static TablaSimbolos* tabla_simbolos_crear(TablaSimbolos* padre) {
    TablaSimbolos* tabla = malloc(sizeof(TablaSimbolos));
    if (!tabla) return NULL;
    tabla->primera = NULL;
    tabla->padre = padre;
    return tabla;
}

// Registrar una etiqueta
static void registrar_etiqueta(Contexto *ctx, const char *nombre, NodoAST *nodo)
{
    Etiqueta *nueva = malloc(sizeof(Etiqueta));
    nueva->nombre = strdup(nombre);
    nueva->nodo = nodo;
    nueva->siguiente = ctx->etiquetas;
    ctx->etiquetas = nueva;
}

// Buscar una etiqueta
static Etiqueta *buscar_etiqueta(Contexto *ctx, const char *nombre)
{
    Etiqueta *actual = ctx->etiquetas;
    while (actual)
    {
        if (strcmp(actual->nombre, nombre) == 0)
        {
            return actual;
        }
        actual = actual->siguiente;
    }
    return NULL;
}

static void registrar_etiquetas_ast(NodoAST *nodo, Contexto *ctx)
{
    if (!nodo)
        return;

    if (nodo->tipo == AST_ETIQUETA)
    {
        registrar_etiqueta(ctx, nodo->datos.etiqueta.nombre, nodo);
    }

    // Recorrer lista enlazada de sentencias
    if (nodo->tipo == AST_BLOQUE)
    {
        NodoAST *actual = nodo->datos.bloque.primera;
        while (actual)
        {
            registrar_etiquetas_ast(actual, ctx);
            actual = actual->siguiente;
        }
    }
    else if (nodo->tipo == AST_SI)
    {
        if (nodo->datos.si.bloque_si)
            registrar_etiquetas_ast(nodo->datos.si.bloque_si, ctx);
        if (nodo->datos.si.bloque_sino)
            registrar_etiquetas_ast(nodo->datos.si.bloque_sino, ctx);
    }
    else if (nodo->tipo == AST_MIENTRAS)
    {
        if (nodo->datos.mientras.bloque)
            registrar_etiquetas_ast(nodo->datos.mientras.bloque, ctx);
    }
    else if (nodo->tipo == AST_PARA)
    {
        if (nodo->datos.para.bloque)
            registrar_etiquetas_ast(nodo->datos.para.bloque, ctx);
    }
}

// ============================================================
// FUNCIONES DE VALOR
// ============================================================

Valor valor_crear_entero(long long valor) {
    Valor v;
    v.tipo = VALOR_ENTERO;
    v.datos.entero = valor;
    v.tamano = 0;
    v.filas = 0;
    v.columnas = 0;
    return v;
}

Valor valor_crear_decimal(double valor) {
    Valor v;
    v.tipo = VALOR_DECIMAL;
    v.datos.decimal = valor;
    v.tamano = 0;
    v.filas = 0;
    v.columnas = 0;
    return v;
}

Valor valor_crear_texto(const char* valor) {
    Valor v;
    v.tipo = VALOR_TEXTO;
    v.datos.texto = strdup(valor ? valor : "");
    v.tamano = 0;
    v.filas = 0;
    v.columnas = 0;
    return v;
}

Valor valor_crear_logica(bool valor) {
    Valor v;
    v.tipo = VALOR_LOGICA;
    v.datos.logica = valor;
    v.tamano = 0;
    v.filas = 0;
    v.columnas = 0;
    return v;
}

Valor valor_crear_caracter(const char *valor)
{
    Valor v;
    v.tipo = VALOR_CARACTER;
    v.datos.texto = strdup(valor); // Copiar el string
    v.tamano = 0;
    v.filas = 0;
    v.columnas = 0;
    return v;
}

Valor valor_crear_entero_sin_signo(unsigned long long valor) {
    Valor v;
    v.tipo = VALOR_ENTERO_SIN_SIGNO;
    v.datos.entero_sin_signo = valor;
    v.tamano = 0;
    v.filas = 0;
    v.columnas = 0;
    return v;
}

Valor valor_crear_decimal_sin_signo(double valor) {
    Valor v;
    v.tipo = VALOR_DECIMAL_SIN_SIGNO;
    v.datos.decimal_sin_signo = valor;
    v.tamano = 0;
    v.filas = 0;
    v.columnas = 0;
    return v;
}

Valor valor_crear_caracter_sin_signo(unsigned char valor)
{
    Valor v;
    v.tipo = VALOR_CARACTER_SIN_SIGNO;
    char buffer[2] = {(char)valor, '\0'};
    v.datos.texto = strdup(buffer);
    v.tamano = 0;
    v.filas = 0;
    v.columnas = 0;
    return v;
}

Valor valor_crear_vacio(void) {
    Valor v;
    v.tipo = VALOR_VACIO;
    v.tamano = 0;
    v.filas = 0;
    v.columnas = 0;
    return v;
}

void valor_destruir(Valor *valor)
{
    if (!valor)
        return;

    if (valor->tipo == VALOR_TEXTO || valor->tipo == VALOR_TEXTO_EXTENSO)
    {
        if (valor->datos.texto)
        {
            free(valor->datos.texto);
            valor->datos.texto = NULL;
        }
    }
    else if (valor->tipo == VALOR_CARACTER || valor->tipo == VALOR_CARACTER_SIN_SIGNO)
    {
        if (valor->datos.texto)
        {
            free(valor->datos.texto);
            valor->datos.texto = NULL;
        }
    }
    else if (valor->tipo == VALOR_ARCHIVO)
    {
        if (valor->datos.archivo)
        {
            fclose(valor->datos.archivo);
            valor->datos.archivo = NULL;
        }
    }
}

void valor_imprimir(Valor valor)
{
    switch (valor.tipo)
    {
    case VALOR_ENTERO:
        printf("%lld", valor.datos.entero);
        break;
    case VALOR_ENTERO_SIN_SIGNO:
        printf("%llu", valor.datos.entero_sin_signo);
        break;
    case VALOR_DECIMAL:
    {
        double val = valor.datos.decimal;
        // Verificar si es un número "entero" (sin parte decimal significativa)
        if (val == floor(val) && fabs(val) < 1e15)
        {
            printf("%.0f", val); // Sin decimales
        }
        else
        {
            // Usar %f pero eliminar ceros finales innecesarios
            char buffer[64];
            snprintf(buffer, sizeof(buffer), "%.15g", val);

            // Encontrar el punto decimal
            char *punto = strchr(buffer, '.');
            if (punto)
            {
                // Eliminar ceros finales después del punto
                char *fin = buffer + strlen(buffer) - 1;
                while (fin > punto && *fin == '0')
                {
                    *fin = '\0';
                    fin--;
                }
                // Si solo queda el punto, eliminarlo también
                if (*fin == '.')
                {
                    *fin = '\0';
                }
            }
            printf("%s", buffer);
        }
    }
    break;
    case VALOR_DECIMAL_SIN_SIGNO:
    {
        double val = valor.datos.decimal_sin_signo;
        if (val == floor(val) && fabs(val) < 1e15)
        {
            printf("%.0f", val);
        }
        else
        {
            char buffer[64];
            snprintf(buffer, sizeof(buffer), "%.15g", val);
            char *punto = strchr(buffer, '.');
            if (punto)
            {
                char *fin = buffer + strlen(buffer) - 1;
                while (fin > punto && *fin == '0')
                {
                    *fin = '\0';
                    fin--;
                }
                if (*fin == '.')
                {
                    *fin = '\0';
                }
            }
            printf("%s", buffer);
        }
    }
    break;
    case VALOR_TEXTO:
    case VALOR_TEXTO_EXTENSO:
        if (valor.datos.texto)
        {
            for (const char *p = valor.datos.texto; *p; p++)
            {
                if (*p == '\x01')
                    printf("[");
                else if (*p == '\x02')
                    printf("]");
                else if (*p == '\x03')
                    printf("$");
                else
                    printf("%c", *p);
            }
        }
        break;
    case VALOR_LOGICA:
        printf("%s", valor.datos.logica ? "VERDADERO" : "FALSO");
        break;
    case VALOR_CARACTER:
    case VALOR_CARACTER_SIN_SIGNO:
        if (valor.datos.texto)
        {
            printf("%s", valor.datos.texto);
        }
        break;
    case VALOR_VACIO:
        break;
    default:
        printf("[valor no soportado]");
        break;
    }
}

// ============================================================
// FUNCIONES DE TABLA DE SÍMBOLOS
// ============================================================

// Función auxiliar para normalizar nombres (quitar $ si existe)
static char* normalizar_nombre(const char* nombre) {
    if (!nombre) return NULL;
    if (nombre[0] == '$') {
        return strdup(nombre + 1);
    }
    return strdup(nombre);
}

void tabla_simbolos_definir(TablaSimbolos *tabla, const char *nombre, Valor valor, bool es_constante)
{
    if (!tabla || !nombre)
        return;

    char *nombre_norm = normalizar_nombre(nombre);

    // IMPORTANTE: Para DECLARACIONES, siempre crear en el scope actual
    // NO buscar en scopes padres - eso es para ASIGNACIONES, no declaraciones

    // Verificar si ya existe en el scope ACTUAL (no en padres)
    Simbolo *sim = tabla->primera;
    while (sim)
    {
        if (strcmp(sim->nombre, nombre_norm) == 0)
        {
            // Variable ya existe en este scope, actualizarla
            valor_destruir(&sim->valor);
            sim->valor = valor;
            sim->es_constante = es_constante;
            free(nombre_norm);
            return;
        }
        sim = sim->siguiente;
    }

    // Si no existe en este scope, crearla aquí
    Simbolo *nuevo = malloc(sizeof(Simbolo));
    if (!nuevo)
    {
        free(nombre_norm);
        return;
    }

    nuevo->nombre = nombre_norm;
    nuevo->valor = valor;
    nuevo->es_constante = es_constante;
    nuevo->siguiente = tabla->primera;
    tabla->primera = nuevo;
}

bool tabla_simbolos_asignar(TablaSimbolos *tabla, const char *nombre, Valor valor)
{
    if (!tabla || !nombre)
        return false;
    char *nombre_norm = normalizar_nombre(nombre);

    // Buscar en TODA la cadena de scopes (desde actual hacia arriba)
    TablaSimbolos *scope_actual = tabla;
    while (scope_actual)
    {
        Simbolo *sim = scope_actual->primera;
        while (sim)
        {
            if (strcmp(sim->nombre, nombre_norm) == 0)
            {
                // Variable encontrada, convertir valor al tipo declarado si es necesario
                Valor valor_convertido = valor;

                // Si la variable existente es CARACTER y el valor es ENTERO, convertir
                if (sim->valor.tipo == VALOR_CARACTER || sim->valor.tipo == VALOR_CARACTER_SIN_SIGNO)
                {
                    if (valor.tipo == VALOR_ENTERO || valor.tipo == VALOR_ENTERO_SIN_SIGNO)
                    {
                        long long val_entero = (valor.tipo == VALOR_ENTERO) ? valor.datos.entero : (long long)valor.datos.entero_sin_signo;
                        char buffer[2] = {(char)val_entero, '\0'};
                        valor_convertido = valor_crear_caracter(buffer);
                    }
                    else if (valor.tipo == VALOR_DECIMAL || valor.tipo == VALOR_DECIMAL_SIN_SIGNO)
                    {
                        double val_decimal = (valor.tipo == VALOR_DECIMAL) ? valor.datos.decimal : valor.datos.decimal_sin_signo;
                        char buffer[2] = {(char)val_decimal, '\0'};
                        valor_convertido = valor_crear_caracter(buffer);
                    }
                }
                // Si la variable existente es ENTERO y el valor es CARACTER, convertir
                else if (sim->valor.tipo == VALOR_ENTERO || sim->valor.tipo == VALOR_ENTERO_SIN_SIGNO)
                {
                    if (valor.tipo == VALOR_CARACTER || valor.tipo == VALOR_CARACTER_SIN_SIGNO)
                    {
                        if (valor.datos.texto && strlen(valor.datos.texto) > 0)
                        {
                            valor_convertido = valor_crear_entero((long long)valor.datos.texto[0]);
                        }
                        else
                        {
                            valor_convertido = valor_crear_entero(0);
                        }
                    }
                }

                // Convertir al tipo declarado de la variable
                if (sim->tipo_declarado == TIPO_ENTERO_SIN_SIGNO)
                {
                    if (valor_convertido.tipo == VALOR_ENTERO)
                    {
                        unsigned long long val = (unsigned long long)valor_convertido.datos.entero;
                        valor_convertido = valor_crear_entero_sin_signo(val);
                    }
                    else if (valor_convertido.tipo == VALOR_DECIMAL)
                    {
                        unsigned long long val = (unsigned long long)valor_convertido.datos.decimal;
                        valor_convertido = valor_crear_entero_sin_signo(val);
                    }
                }
                else if (sim->tipo_declarado == TIPO_DECIMAL_SIN_SIGNO)
                {
                    if (valor_convertido.tipo == VALOR_DECIMAL)
                    {
                        valor_convertido = valor_crear_decimal_sin_signo(valor_convertido.datos.decimal);
                    }
                    else if (valor_convertido.tipo == VALOR_ENTERO)
                    {
                        valor_convertido = valor_crear_decimal_sin_signo((double)valor_convertido.datos.entero);
                    }
                    else if (valor_convertido.tipo == VALOR_ENTERO_SIN_SIGNO)
                    {
                        valor_convertido = valor_crear_decimal_sin_signo((double)valor_convertido.datos.entero_sin_signo);
                    }
                }
                else if (sim->tipo_declarado == TIPO_ENTERO)
                {
                    if (valor_convertido.tipo == VALOR_ENTERO_SIN_SIGNO)
                    {
                        valor_convertido = valor_crear_entero((long long)valor_convertido.datos.entero_sin_signo);
                    }
                }
                else if (sim->tipo_declarado == TIPO_DECIMAL)
                {
                    if (valor_convertido.tipo == VALOR_DECIMAL_SIN_SIGNO)
                    {
                        valor_convertido = valor_crear_decimal(valor_convertido.datos.decimal_sin_signo);
                    }
                    else if (valor_convertido.tipo == VALOR_ENTERO)
                    {
                        valor_convertido = valor_crear_decimal((double)valor_convertido.datos.entero);
                    }
                    else if (valor_convertido.tipo == VALOR_ENTERO_SIN_SIGNO)
                    {
                        valor_convertido = valor_crear_decimal((double)valor_convertido.datos.entero_sin_signo);
                    }
                }

                // Convertir al tipo declarado de la variable
                if (sim->tipo_declarado == TIPO_ENTERO_SIN_SIGNO && valor_convertido.tipo == VALOR_ENTERO)
                {
                    if (valor_convertido.datos.entero >= 0)
                    {
                        unsigned long long val = (unsigned long long)valor_convertido.datos.entero;
                        valor_convertido = valor_crear_entero_sin_signo(val);
                    }
                }
                else if (sim->tipo_declarado == TIPO_DECIMAL_SIN_SIGNO && valor_convertido.tipo == VALOR_DECIMAL)
                {
                    if (valor_convertido.datos.decimal >= 0.0)
                    {
                        valor_convertido = valor_crear_decimal_sin_signo(valor_convertido.datos.decimal);
                    }
                }
                else if (sim->tipo_declarado == TIPO_DECIMAL && valor_convertido.tipo == VALOR_ENTERO)
                {
                    valor_convertido = valor_crear_decimal((double)valor_convertido.datos.entero);
                }
                else if (sim->tipo_declarado == TIPO_DECIMAL && valor_convertido.tipo == VALOR_ENTERO_SIN_SIGNO)
                {
                    valor_convertido = valor_crear_decimal((double)valor_convertido.datos.entero_sin_signo);
                }

                // Variable encontrada, actualizarla
                valor_destruir(&sim->valor);
                sim->valor = valor_convertido;
                free(nombre_norm);
                return true;
            }
            sim = sim->siguiente;
        }
        scope_actual = scope_actual->padre;
    }

    // Si no existe en ningún scope, NO crearla (declaración estricta)
    free(nombre_norm);
    return false;
}

Valor* tabla_simbolos_buscar(TablaSimbolos* tabla, const char* nombre) {
    if (!tabla || !nombre) return NULL;
    
    char* nombre_norm = normalizar_nombre(nombre);
    
    TablaSimbolos* actual = tabla;
    while (actual) {
        Simbolo* sim = actual->primera;
        while (sim) {
            if (strcmp(sim->nombre, nombre_norm) == 0) {
                free(nombre_norm);
                return &sim->valor;
            }
            sim = sim->siguiente;
        }
        actual = actual->padre;
    }
    
    free(nombre_norm);
    return NULL;
}

Simbolo *tabla_simbolos_buscar_simbolo(TablaSimbolos *tabla, const char *nombre)
{
    if (!tabla || !nombre)
        return NULL;

    char *nombre_norm = normalizar_nombre(nombre);

    TablaSimbolos *actual = tabla;
    while (actual)
    {
        Simbolo *sim = actual->primera;
        while (sim)
        {
            if (strcmp(sim->nombre, nombre_norm) == 0)
            {
                free(nombre_norm);
                return sim;
            }
            sim = sim->siguiente;
        }
        actual = actual->padre;
    }

    free(nombre_norm);
    return NULL;
}

void tabla_simbolos_destruir(TablaSimbolos* tabla) {
    if (!tabla) return;
    
    Simbolo* actual = tabla->primera;
    while (actual) {
        Simbolo* siguiente = actual->siguiente;
        free(actual->nombre);
        valor_destruir(&actual->valor);
        free(actual);
        actual = siguiente;
    }
    
    free(tabla);
}

// ============================================================
// GESTIÓN DE FUNCIONES DEFINIDAS POR USUARIO
// ============================================================
void contexto_registrar_funcion(Contexto *ctx, DefinicionFuncion *func)
{
    if (!ctx || !func)
        return;

    // Verificar si ya existe
    DefinicionFuncion *actual = ctx->funciones;
    while (actual)
    {
        if (strcasecmp(actual->nombre, func->nombre) == 0)
        {
            // Ya existe, no registrar de nuevo
            return;
        }
        actual = actual->siguiente;
    }

    // Agregar al inicio de la lista
    func->siguiente = ctx->funciones;
    ctx->funciones = func;
}

DefinicionFuncion *contexto_buscar_funcion(Contexto *ctx, const char *nombre)
{
    if (!ctx || !nombre)
        return NULL;

    // Normalizar nombre (quitar $ si existe)
    const char *nombre_norm = (nombre[0] == '$') ? nombre + 1 : nombre;

    DefinicionFuncion *actual = ctx->funciones;
    while (actual)
    {
        if (strcasecmp(actual->nombre, nombre_norm) == 0)
        {
            return actual;
        }
        actual = actual->siguiente;
    }

    return NULL;
}

void contexto_liberar_funciones(Contexto *ctx)
{
    if (!ctx)
        return;

    DefinicionFuncion *actual = ctx->funciones;
    while (actual)
    {
        DefinicionFuncion *siguiente = actual->siguiente;
        free(actual->nombre);
        for (int i = 0; i < actual->num_parametros; i++)
        {
            free(actual->parametros[i].nombre);
        }
        free(actual->parametros);
        free(actual);
        actual = siguiente;
    }
    ctx->funciones = NULL;
}

// ============================================================
// FUNCIONES DE CONTEXTO
// ============================================================
Contexto *contexto_crear(void)
{
    Contexto *ctx = malloc(sizeof(Contexto));
    if (!ctx)
        return NULL;
    ctx->nombre_programa = NULL;
    ctx->tabla_global = tabla_simbolos_crear(NULL);
    ctx->tabla_actual = ctx->tabla_global;
    ctx->estado_flujo = FLUJO_NORMAL;
    ctx->valor_retorno = valor_crear_vacio();
    ctx->hay_error = false;
    ctx->mensaje_error[0] = '\0';
    ctx->linea_actual = 0;
    ctx->funciones = NULL;
    ctx->profundidad_llamada = 0;
    // Inicializar pool de tablas
    ctx->pool_index = 0;
    for (int i = 0; i < 1000; i++)
    {
        ctx->pool_tablas[i] = NULL;
    }
    ctx->etiquetas = NULL;
    ctx->salto_pendiente = NULL;
    ctx->etiqueta_salto = NULL;

    // Inicializar campos SQLite
    ctx->sqlite_db = NULL;
    ctx->sqlite_stmt = NULL;
    ctx->sqlite_columnas = 0;
    ctx->sqlite_columnas_nombre = NULL;

    // Inicializar stack de INTENTAR/ATRAPAR
    ctx->intentar_stack_top = 0;
    for (int i = 0; i < MAX_INTENTAR_STACK; i++)
    {
        ctx->intentar_stack[i].activo = false;
        ctx->intentar_stack[i].error_capturado = false;
        ctx->intentar_stack[i].mensaje_error[0] = '\0';
        ctx->intentar_stack[i].linea_error = 0;
        ctx->intentar_stack[i].codigo_error = 0;
    }

    return ctx;
}

void contexto_destruir(Contexto *ctx)
{
    if (ctx->nombre_programa)
    {
        free(ctx->nombre_programa);
    }

    if (!ctx)
        return;

    // Limpiar recursos SQLite
    if (ctx->sqlite_stmt)
    {
        sqlite3_finalize((sqlite3_stmt *)ctx->sqlite_stmt);
        ctx->sqlite_stmt = NULL;
    }
    if (ctx->sqlite_db)
    {
        sqlite3_close((sqlite3 *)ctx->sqlite_db);
        ctx->sqlite_db = NULL;
    }
    if (ctx->sqlite_columnas_nombre)
    {
        free(ctx->sqlite_columnas_nombre);
        ctx->sqlite_columnas_nombre = NULL;
    }

    // Destruir todas las tablas de símbolos
    TablaSimbolos *actual = ctx->tabla_actual;
    while (actual && actual != ctx->tabla_global)
    {
        TablaSimbolos *padre = actual->padre;
        tabla_simbolos_destruir(actual);
        actual = padre;
    }

    if (ctx->tabla_global)
    {
        tabla_simbolos_destruir(ctx->tabla_global);
    }

    // Liberar etiquetas
    Etiqueta *etiqueta = ctx->etiquetas;
    while (etiqueta)
    {
        Etiqueta *siguiente = etiqueta->siguiente;
        free(etiqueta->nombre);
        free(etiqueta);
        etiqueta = siguiente;
    }

    // Liberar salto pendiente
    if (ctx->salto_pendiente)
    {
        free(ctx->salto_pendiente);
    }

    if (ctx->etiqueta_salto)
    {
        free(ctx->etiqueta_salto);
    }

    valor_destruir(&ctx->valor_retorno);
    contexto_liberar_funciones(ctx);
    free(ctx);
}

bool contexto_tiene_error(const Contexto* ctx) {
    return ctx && ctx->hay_error;
}

const char* contexto_obtener_error(const Contexto* ctx) {
    if (!ctx) return "Contexto nulo";
    return ctx->mensaje_error;
}

static bool ambos_operandos_enteros(TipoValor a, TipoValor b)
{
    bool a_entero = (a == VALOR_ENTERO || a == VALOR_ENTERO_SIN_SIGNO);
    bool b_entero = (b == VALOR_ENTERO || b == VALOR_ENTERO_SIN_SIGNO);
    return a_entero && b_entero;
}

// ============================================================
// EVALUACIÓN DE EXPRESIONES
// ============================================================
static Valor evaluar_operador_binario(OperadorBinario op, Valor izq, Valor der, Contexto* ctx) {
    switch (op) {
    case OP_SUMA:
    {
        // Concatenación de textos
        if (izq.tipo == VALOR_TEXTO || der.tipo == VALOR_TEXTO)
        {
            char buffer[4096];
            snprintf(buffer, sizeof(buffer), "%s%s",
                     izq.tipo == VALOR_TEXTO ? izq.datos.texto : "",
                     der.tipo == VALOR_TEXTO ? der.datos.texto : "");
            return valor_crear_texto(buffer);
        }
        // Camino rápido: ambos operandos enteros
        if ((izq.tipo == VALOR_ENTERO || izq.tipo == VALOR_ENTERO_SIN_SIGNO) &&
            (der.tipo == VALOR_ENTERO || der.tipo == VALOR_ENTERO_SIN_SIGNO))
        {
            // Si AL MENOS UNO es SIN SIGNO, usar aritmética sin signo
            if (izq.tipo == VALOR_ENTERO_SIN_SIGNO || der.tipo == VALOR_ENTERO_SIN_SIGNO)
            {
                unsigned long long a = (izq.tipo == VALOR_ENTERO_SIN_SIGNO)
                                           ? izq.datos.entero_sin_signo
                                           : (unsigned long long)izq.datos.entero;
                unsigned long long b = (der.tipo == VALOR_ENTERO_SIN_SIGNO)
                                           ? der.datos.entero_sin_signo
                                           : (unsigned long long)der.datos.entero;
                return valor_crear_entero_sin_signo(a + b);
            }
            // Ambos son ENTERO con signo
            return valor_crear_entero(izq.datos.entero + der.datos.entero);
        }
        // Al menos uno es decimal
        {
            double v_izq = (izq.tipo == VALOR_ENTERO)              ? (double)izq.datos.entero
                           : (izq.tipo == VALOR_ENTERO_SIN_SIGNO)  ? (double)izq.datos.entero_sin_signo
                           : (izq.tipo == VALOR_DECIMAL)           ? izq.datos.decimal
                           : (izq.tipo == VALOR_DECIMAL_SIN_SIGNO) ? izq.datos.decimal_sin_signo
                                                                   : 0.0;
            double v_der = (der.tipo == VALOR_ENTERO)              ? (double)der.datos.entero
                           : (der.tipo == VALOR_ENTERO_SIN_SIGNO)  ? (double)der.datos.entero_sin_signo
                           : (der.tipo == VALOR_DECIMAL)           ? der.datos.decimal
                           : (der.tipo == VALOR_DECIMAL_SIN_SIGNO) ? der.datos.decimal_sin_signo
                                                                   : 0.0;
            return valor_crear_decimal(v_izq + v_der);
        }
    }
    break;
    case OP_RESTA:
    {
        // Camino rápido: ambos operandos enteros
        if (ambos_operandos_enteros(izq.tipo, der.tipo))
        {
            // Si AL MENOS UNO es SIN SIGNO, usar aritmética sin signo
            if (izq.tipo == VALOR_ENTERO_SIN_SIGNO || der.tipo == VALOR_ENTERO_SIN_SIGNO)
            {
                unsigned long long a = (izq.tipo == VALOR_ENTERO_SIN_SIGNO)
                                           ? izq.datos.entero_sin_signo
                                           : (unsigned long long)izq.datos.entero;
                unsigned long long b = (der.tipo == VALOR_ENTERO_SIN_SIGNO)
                                           ? der.datos.entero_sin_signo
                                           : (unsigned long long)der.datos.entero;
                return valor_crear_entero_sin_signo(a - b);
            }
            // Ambos son ENTERO con signo
            return valor_crear_entero(izq.datos.entero - der.datos.entero);
        }
        // Al menos uno es decimal
        {
            double v_izq = (izq.tipo == VALOR_ENTERO) ? (double)izq.datos.entero : (izq.tipo == VALOR_ENTERO_SIN_SIGNO) ? (double)izq.datos.entero_sin_signo
                                                                               : (izq.tipo == VALOR_DECIMAL)            ? izq.datos.decimal
                                                                               : (izq.tipo == VALOR_DECIMAL_SIN_SIGNO)  ? izq.datos.decimal_sin_signo
                                                                                                                        : 0.0;
            double v_der = (der.tipo == VALOR_ENTERO) ? (double)der.datos.entero : (der.tipo == VALOR_ENTERO_SIN_SIGNO) ? (double)der.datos.entero_sin_signo
                                                                               : (der.tipo == VALOR_DECIMAL)            ? der.datos.decimal
                                                                               : (der.tipo == VALOR_DECIMAL_SIN_SIGNO)  ? der.datos.decimal_sin_signo
                                                                                                                        : 0.0;
            return valor_crear_decimal(v_izq - v_der);
        }
        }
        break;
        case OP_MULTIPLICACION:
        {
            // Camino rápido: ambos operandos enteros
            if ((izq.tipo == VALOR_ENTERO || izq.tipo == VALOR_ENTERO_SIN_SIGNO) &&
                (der.tipo == VALOR_ENTERO || der.tipo == VALOR_ENTERO_SIN_SIGNO))
            {
                // Si AL MENOS UNO es SIN SIGNO, usar aritmética sin signo
                if (izq.tipo == VALOR_ENTERO_SIN_SIGNO || der.tipo == VALOR_ENTERO_SIN_SIGNO)
                {
                    unsigned long long a = (izq.tipo == VALOR_ENTERO_SIN_SIGNO)
                                               ? izq.datos.entero_sin_signo
                                               : (unsigned long long)izq.datos.entero;
                    unsigned long long b = (der.tipo == VALOR_ENTERO_SIN_SIGNO)
                                               ? der.datos.entero_sin_signo
                                               : (unsigned long long)der.datos.entero;
                    return valor_crear_entero_sin_signo(a * b);
                }
                // Ambos son ENTERO con signo
                return valor_crear_entero(izq.datos.entero * der.datos.entero);
            }
            // Al menos uno es decimal
            {
                double v_izq = (izq.tipo == VALOR_ENTERO) ? (double)izq.datos.entero : (izq.tipo == VALOR_ENTERO_SIN_SIGNO) ? (double)izq.datos.entero_sin_signo
                                                                                   : (izq.tipo == VALOR_DECIMAL)            ? izq.datos.decimal
                                                                                   : (izq.tipo == VALOR_DECIMAL_SIN_SIGNO)  ? izq.datos.decimal_sin_signo
                                                                                                                            : 0.0;
                double v_der = (der.tipo == VALOR_ENTERO) ? (double)der.datos.entero : (der.tipo == VALOR_ENTERO_SIN_SIGNO) ? (double)der.datos.entero_sin_signo
                                                                                   : (der.tipo == VALOR_DECIMAL)            ? der.datos.decimal
                                                                                   : (der.tipo == VALOR_DECIMAL_SIN_SIGNO)  ? der.datos.decimal_sin_signo
                                                                                                                            : 0.0;
                return valor_crear_decimal(v_izq * v_der);
            }
        }
        break;
        case OP_DIVISION:
        {
            double v_izq = (izq.tipo == VALOR_ENTERO) ? (double)izq.datos.entero : (izq.tipo == VALOR_ENTERO_SIN_SIGNO) ? (double)izq.datos.entero_sin_signo
                                                                               : (izq.tipo == VALOR_DECIMAL)            ? izq.datos.decimal
                                                                               : (izq.tipo == VALOR_DECIMAL_SIN_SIGNO)  ? izq.datos.decimal_sin_signo
                                                                                                                        : 0.0;
            double v_der = (der.tipo == VALOR_ENTERO) ? (double)der.datos.entero : (der.tipo == VALOR_ENTERO_SIN_SIGNO) ? (double)der.datos.entero_sin_signo
                                                                               : (der.tipo == VALOR_DECIMAL)            ? der.datos.decimal
                                                                               : (der.tipo == VALOR_DECIMAL_SIN_SIGNO)  ? der.datos.decimal_sin_signo
                                                                                                                        : 0.0;
            if (v_der == 0)
            {
                contexto_set_error(ctx, "División por cero");
                return valor_crear_vacio();
            }
            return valor_crear_decimal(v_izq / v_der);
        }
        break;
        case OP_MODULO:
            if ((izq.tipo == VALOR_ENTERO || izq.tipo == VALOR_ENTERO_SIN_SIGNO) &&
                (der.tipo == VALOR_ENTERO || der.tipo == VALOR_ENTERO_SIN_SIGNO))
            {
                long long a = (izq.tipo == VALOR_ENTERO) ? izq.datos.entero : (long long)izq.datos.entero_sin_signo;
                long long b = (der.tipo == VALOR_ENTERO) ? der.datos.entero : (long long)der.datos.entero_sin_signo;
                if (b == 0) {
                    contexto_set_error(ctx, "Módulo por cero");
                    return valor_crear_vacio();
                }
                return valor_crear_entero(a % b);
            }
            {
                double v_izq = (izq.tipo == VALOR_ENTERO) ? (double)izq.datos.entero :
                               (izq.tipo == VALOR_ENTERO_SIN_SIGNO) ? (double)izq.datos.entero_sin_signo :
                               (izq.tipo == VALOR_DECIMAL) ? izq.datos.decimal :
                               (izq.tipo == VALOR_DECIMAL_SIN_SIGNO) ? izq.datos.decimal_sin_signo : 0.0;
                double v_der = (der.tipo == VALOR_ENTERO) ? (double)der.datos.entero :
                               (der.tipo == VALOR_ENTERO_SIN_SIGNO) ? (double)der.datos.entero_sin_signo :
                               (der.tipo == VALOR_DECIMAL) ? der.datos.decimal :
                               (der.tipo == VALOR_DECIMAL_SIN_SIGNO) ? der.datos.decimal_sin_signo : 0.0;
                if ((int)v_der == 0) {
                    contexto_set_error(ctx, "Módulo por cero");
                    return valor_crear_vacio();
                }
                return valor_crear_entero((long long)v_izq % (long long)v_der);
            }
            
        case OP_POTENCIA:
            {
                double v_izq = (izq.tipo == VALOR_ENTERO) ? (double)izq.datos.entero :
                               (izq.tipo == VALOR_ENTERO_SIN_SIGNO) ? (double)izq.datos.entero_sin_signo :
                               (izq.tipo == VALOR_DECIMAL) ? izq.datos.decimal :
                               (izq.tipo == VALOR_DECIMAL_SIN_SIGNO) ? izq.datos.decimal_sin_signo : 0.0;
                double v_der = (der.tipo == VALOR_ENTERO) ? (double)der.datos.entero :
                               (der.tipo == VALOR_ENTERO_SIN_SIGNO) ? (double)der.datos.entero_sin_signo :
                               (der.tipo == VALOR_DECIMAL) ? der.datos.decimal :
                               (der.tipo == VALOR_DECIMAL_SIN_SIGNO) ? der.datos.decimal_sin_signo : 0.0;
                return valor_crear_decimal(pow(v_izq, v_der));
            }

        case OP_IGUAL:
            if (izq.tipo == VALOR_TEXTO && der.tipo == VALOR_TEXTO)
            {
                return valor_crear_logica(strcmp(izq.datos.texto, der.datos.texto) == 0);
            }
            {
                double v_izq = (izq.tipo == VALOR_ENTERO) ? (double)izq.datos.entero :
                               (izq.tipo == VALOR_ENTERO_SIN_SIGNO) ? (double)izq.datos.entero_sin_signo :
                               (izq.tipo == VALOR_DECIMAL) ? izq.datos.decimal :
                               (izq.tipo == VALOR_DECIMAL_SIN_SIGNO) ? izq.datos.decimal_sin_signo : 0.0;
                double v_der = (der.tipo == VALOR_ENTERO) ? (double)der.datos.entero :
                               (der.tipo == VALOR_ENTERO_SIN_SIGNO) ? (double)der.datos.entero_sin_signo :
                               (der.tipo == VALOR_DECIMAL) ? der.datos.decimal :
                               (der.tipo == VALOR_DECIMAL_SIN_SIGNO) ? der.datos.decimal_sin_signo : 0.0;
                return valor_crear_logica(v_izq == v_der);
            }

        case OP_DIFERENTE:
            if (izq.tipo == VALOR_TEXTO && der.tipo == VALOR_TEXTO)
            {
                return valor_crear_logica(strcmp(izq.datos.texto, der.datos.texto) != 0);
            }
            {
                double v_izq = (izq.tipo == VALOR_ENTERO) ? (double)izq.datos.entero :
                               (izq.tipo == VALOR_ENTERO_SIN_SIGNO) ? (double)izq.datos.entero_sin_signo :
                               (izq.tipo == VALOR_DECIMAL) ? izq.datos.decimal :
                               (izq.tipo == VALOR_DECIMAL_SIN_SIGNO) ? izq.datos.decimal_sin_signo : 0.0;
                double v_der = (der.tipo == VALOR_ENTERO) ? (double)der.datos.entero :
                               (der.tipo == VALOR_ENTERO_SIN_SIGNO) ? (double)der.datos.entero_sin_signo :
                               (der.tipo == VALOR_DECIMAL) ? der.datos.decimal :
                               (der.tipo == VALOR_DECIMAL_SIN_SIGNO) ? der.datos.decimal_sin_signo : 0.0;
                return valor_crear_logica(v_izq != v_der);
            }

        case OP_MAYOR:
            if (izq.tipo == VALOR_TEXTO && der.tipo == VALOR_TEXTO)
            {
                return valor_crear_logica(strcmp(izq.datos.texto, der.datos.texto) > 0);
            }
            {
                double v_izq = (izq.tipo == VALOR_ENTERO) ? (double)izq.datos.entero :
                               (izq.tipo == VALOR_ENTERO_SIN_SIGNO) ? (double)izq.datos.entero_sin_signo :
                               (izq.tipo == VALOR_DECIMAL) ? izq.datos.decimal :
                               (izq.tipo == VALOR_DECIMAL_SIN_SIGNO) ? izq.datos.decimal_sin_signo : 0.0;
                double v_der = (der.tipo == VALOR_ENTERO) ? (double)der.datos.entero :
                               (der.tipo == VALOR_ENTERO_SIN_SIGNO) ? (double)der.datos.entero_sin_signo :
                               (der.tipo == VALOR_DECIMAL) ? der.datos.decimal :
                               (der.tipo == VALOR_DECIMAL_SIN_SIGNO) ? der.datos.decimal_sin_signo : 0.0;
                return valor_crear_logica(v_izq > v_der);
            }

        case OP_MENOR:
            if (izq.tipo == VALOR_TEXTO && der.tipo == VALOR_TEXTO)
            {
                return valor_crear_logica(strcmp(izq.datos.texto, der.datos.texto) < 0);
            }
            {
                double v_izq = (izq.tipo == VALOR_ENTERO) ? (double)izq.datos.entero :
                               (izq.tipo == VALOR_ENTERO_SIN_SIGNO) ? (double)izq.datos.entero_sin_signo :
                               (izq.tipo == VALOR_DECIMAL) ? izq.datos.decimal :
                               (izq.tipo == VALOR_DECIMAL_SIN_SIGNO) ? izq.datos.decimal_sin_signo : 0.0;
                double v_der = (der.tipo == VALOR_ENTERO) ? (double)der.datos.entero :
                               (der.tipo == VALOR_ENTERO_SIN_SIGNO) ? (double)der.datos.entero_sin_signo :
                               (der.tipo == VALOR_DECIMAL) ? der.datos.decimal :
                               (der.tipo == VALOR_DECIMAL_SIN_SIGNO) ? der.datos.decimal_sin_signo : 0.0;
                return valor_crear_logica(v_izq < v_der);
            }

        case OP_MAYOR_IGUAL:
            if (izq.tipo == VALOR_TEXTO && der.tipo == VALOR_TEXTO)
            {
                return valor_crear_logica(strcmp(izq.datos.texto, der.datos.texto) >= 0);
            }
            {
                double v_izq = (izq.tipo == VALOR_ENTERO) ? (double)izq.datos.entero :
                               (izq.tipo == VALOR_ENTERO_SIN_SIGNO) ? (double)izq.datos.entero_sin_signo :
                               (izq.tipo == VALOR_DECIMAL) ? izq.datos.decimal :
                               (izq.tipo == VALOR_DECIMAL_SIN_SIGNO) ? izq.datos.decimal_sin_signo : 0.0;
                double v_der = (der.tipo == VALOR_ENTERO) ? (double)der.datos.entero :
                               (der.tipo == VALOR_ENTERO_SIN_SIGNO) ? (double)der.datos.entero_sin_signo :
                               (der.tipo == VALOR_DECIMAL) ? der.datos.decimal :
                               (der.tipo == VALOR_DECIMAL_SIN_SIGNO) ? der.datos.decimal_sin_signo : 0.0;
                return valor_crear_logica(v_izq >= v_der);
            }

        case OP_MENOR_IGUAL:
            if (izq.tipo == VALOR_TEXTO && der.tipo == VALOR_TEXTO)
            {
                return valor_crear_logica(strcmp(izq.datos.texto, der.datos.texto) <= 0);
            }
            {
                double v_izq = (izq.tipo == VALOR_ENTERO) ? (double)izq.datos.entero :
                               (izq.tipo == VALOR_ENTERO_SIN_SIGNO) ? (double)izq.datos.entero_sin_signo :
                               (izq.tipo == VALOR_DECIMAL) ? izq.datos.decimal :
                               (izq.tipo == VALOR_DECIMAL_SIN_SIGNO) ? izq.datos.decimal_sin_signo : 0.0;
                double v_der = (der.tipo == VALOR_ENTERO) ? (double)der.datos.entero :
                               (der.tipo == VALOR_ENTERO_SIN_SIGNO) ? (double)der.datos.entero_sin_signo :
                               (der.tipo == VALOR_DECIMAL) ? der.datos.decimal :
                               (der.tipo == VALOR_DECIMAL_SIN_SIGNO) ? der.datos.decimal_sin_signo : 0.0;
                return valor_crear_logica(v_izq <= v_der);
            }

        case OP_Y_LOGICO:
            {
                double v_izq = (izq.tipo == VALOR_ENTERO) ? (double)izq.datos.entero :
                               (izq.tipo == VALOR_ENTERO_SIN_SIGNO) ? (double)izq.datos.entero_sin_signo :
                               (izq.tipo == VALOR_DECIMAL) ? izq.datos.decimal :
                               (izq.tipo == VALOR_LOGICA) ? (izq.datos.logica ? 1.0 : 0.0) : 0.0;
                double v_der = (der.tipo == VALOR_ENTERO) ? (double)der.datos.entero :
                               (der.tipo == VALOR_ENTERO_SIN_SIGNO) ? (double)der.datos.entero_sin_signo :
                               (der.tipo == VALOR_DECIMAL) ? der.datos.decimal :
                               (der.tipo == VALOR_LOGICA) ? (der.datos.logica ? 1.0 : 0.0) : 0.0;
                return valor_crear_logica((v_izq != 0) && (v_der != 0));
            }
            
        case OP_O_LOGICO:
            {
                double v_izq = (izq.tipo == VALOR_ENTERO) ? (double)izq.datos.entero :
                               (izq.tipo == VALOR_ENTERO_SIN_SIGNO) ? (double)izq.datos.entero_sin_signo :
                               (izq.tipo == VALOR_DECIMAL) ? izq.datos.decimal :
                               (izq.tipo == VALOR_LOGICA) ? (izq.datos.logica ? 1.0 : 0.0) : 0.0;
                double v_der = (der.tipo == VALOR_ENTERO) ? (double)der.datos.entero :
                               (der.tipo == VALOR_ENTERO_SIN_SIGNO) ? (double)der.datos.entero_sin_signo :
                               (der.tipo == VALOR_DECIMAL) ? der.datos.decimal :
                               (der.tipo == VALOR_LOGICA) ? (der.datos.logica ? 1.0 : 0.0) : 0.0;
                return valor_crear_logica((v_izq != 0) || (v_der != 0));
            }

        case OP_BIT_AND:
            {
                unsigned long long v_izq = 0;
                if (izq.tipo == VALOR_ENTERO)
                    v_izq = (unsigned long long)izq.datos.entero;
                else if (izq.tipo == VALOR_ENTERO_SIN_SIGNO)
                    v_izq = izq.datos.entero_sin_signo;
                else if (izq.tipo == VALOR_DECIMAL)
                    v_izq = (unsigned long long)izq.datos.decimal;
                else if (izq.tipo == VALOR_DECIMAL_SIN_SIGNO)
                    v_izq = (unsigned long long)izq.datos.decimal_sin_signo;

                unsigned long long v_der = 0;
                if (der.tipo == VALOR_ENTERO)
                    v_der = (unsigned long long)der.datos.entero;
                else if (der.tipo == VALOR_ENTERO_SIN_SIGNO)
                    v_der = der.datos.entero_sin_signo;
                else if (der.tipo == VALOR_DECIMAL)
                    v_der = (unsigned long long)der.datos.decimal;
                else if (der.tipo == VALOR_DECIMAL_SIN_SIGNO)
                    v_der = (unsigned long long)der.datos.decimal_sin_signo;

                return valor_crear_entero_sin_signo(v_izq & v_der);
            }
        case OP_BIT_OR:
            {
                unsigned long long v_izq = 0;
                if (izq.tipo == VALOR_ENTERO)
                    v_izq = (unsigned long long)izq.datos.entero;
                else if (izq.tipo == VALOR_ENTERO_SIN_SIGNO)
                    v_izq = izq.datos.entero_sin_signo;
                else if (izq.tipo == VALOR_DECIMAL)
                    v_izq = (unsigned long long)izq.datos.decimal;
                else if (izq.tipo == VALOR_DECIMAL_SIN_SIGNO)
                    v_izq = (unsigned long long)izq.datos.decimal_sin_signo;

                unsigned long long v_der = 0;
                if (der.tipo == VALOR_ENTERO)
                    v_der = (unsigned long long)der.datos.entero;
                else if (der.tipo == VALOR_ENTERO_SIN_SIGNO)
                    v_der = der.datos.entero_sin_signo;
                else if (der.tipo == VALOR_DECIMAL)
                    v_der = (unsigned long long)der.datos.decimal;
                else if (der.tipo == VALOR_DECIMAL_SIN_SIGNO)
                    v_der = (unsigned long long)der.datos.decimal_sin_signo;

                return valor_crear_entero_sin_signo(v_izq | v_der);
            }
        case OP_BIT_XOR:
            {
                unsigned long long v_izq = 0;
                if (izq.tipo == VALOR_ENTERO)
                    v_izq = (unsigned long long)izq.datos.entero;
                else if (izq.tipo == VALOR_ENTERO_SIN_SIGNO)
                    v_izq = izq.datos.entero_sin_signo;
                else if (izq.tipo == VALOR_DECIMAL)
                    v_izq = (unsigned long long)izq.datos.decimal;
                else if (izq.tipo == VALOR_DECIMAL_SIN_SIGNO)
                    v_izq = (unsigned long long)izq.datos.decimal_sin_signo;

                unsigned long long v_der = 0;
                if (der.tipo == VALOR_ENTERO)
                    v_der = (unsigned long long)der.datos.entero;
                else if (der.tipo == VALOR_ENTERO_SIN_SIGNO)
                    v_der = der.datos.entero_sin_signo;
                else if (der.tipo == VALOR_DECIMAL)
                    v_der = (unsigned long long)der.datos.decimal;
                else if (der.tipo == VALOR_DECIMAL_SIN_SIGNO)
                    v_der = (unsigned long long)der.datos.decimal_sin_signo;

                return valor_crear_entero_sin_signo(v_izq ^ v_der);
            }
        case OP_DESPLAZAR_IZQ:
            {
                unsigned long long v_izq = (izq.tipo == VALOR_ENTERO) ? (unsigned long long)izq.datos.entero : (izq.tipo == VALOR_ENTERO_SIN_SIGNO) ? izq.datos.entero_sin_signo
                                                                                                                                                    : 0;
                unsigned long long v_der = (der.tipo == VALOR_ENTERO) ? (unsigned long long)der.datos.entero : (der.tipo == VALOR_ENTERO_SIN_SIGNO) ? der.datos.entero_sin_signo
                                                                                                                                                    : 0;
                return valor_crear_entero_sin_signo(v_izq << v_der);
            }
        case OP_DESPLAZAR_DER:
            {
                unsigned long long v_izq = (izq.tipo == VALOR_ENTERO) ? (unsigned long long)izq.datos.entero : (izq.tipo == VALOR_ENTERO_SIN_SIGNO) ? izq.datos.entero_sin_signo
                                                                                                                                                    : 0;
                unsigned long long v_der = (der.tipo == VALOR_ENTERO) ? (unsigned long long)der.datos.entero : (der.tipo == VALOR_ENTERO_SIN_SIGNO) ? der.datos.entero_sin_signo
                                                                                                                                                    : 0;
                return valor_crear_entero_sin_signo(v_izq >> v_der);
            }
        case OP_ROTAR_IZQ:
            {
                unsigned long long val = (izq.tipo == VALOR_ENTERO) ? (unsigned long long)izq.datos.entero : (izq.tipo == VALOR_ENTERO_SIN_SIGNO) ? izq.datos.entero_sin_signo
                                                                                                                                                  : 0;
                unsigned long long shift = (der.tipo == VALOR_ENTERO) ? (unsigned long long)der.datos.entero : (der.tipo == VALOR_ENTERO_SIN_SIGNO) ? der.datos.entero_sin_signo
                                                                                                                                                    : 0;
                shift = shift % 64;
                return valor_crear_entero_sin_signo((val << shift) | (val >> (64 - shift)));
            }
        case OP_ROTAR_DER:
            {
                unsigned long long val = (izq.tipo == VALOR_ENTERO) ? (unsigned long long)izq.datos.entero : (izq.tipo == VALOR_ENTERO_SIN_SIGNO) ? izq.datos.entero_sin_signo
                                                                                                                                                  : 0;
                unsigned long long shift = (der.tipo == VALOR_ENTERO) ? (unsigned long long)der.datos.entero : (der.tipo == VALOR_ENTERO_SIN_SIGNO) ? der.datos.entero_sin_signo
                                                                                                                                                    : 0;
                shift = shift % 64;
                return valor_crear_entero_sin_signo((val >> shift) | (val << (64 - shift)));
            }

        default:
            contexto_set_error(ctx, "Operador binario no soportado");
            return valor_crear_vacio();
    }
}

static Valor evaluar_operador_unario(OperadorUnario op, Valor operando, Contexto *ctx)
{
    switch (op)
    {
    case OP_NEGACION:
    {
        double v = 0;
        if (operando.tipo == VALOR_ENTERO)
            v = (double)operando.datos.entero;
        else if (operando.tipo == VALOR_DECIMAL)
            v = operando.datos.decimal;
        else if (operando.tipo == VALOR_LOGICA)
            v = operando.datos.logica ? 1.0 : 0.0;
        return valor_crear_decimal(-v);
    }
    case OP_NO_LOGICO:
    {
        double v = 0;
        if (operando.tipo == VALOR_ENTERO)
            v = (double)operando.datos.entero;
        else if (operando.tipo == VALOR_DECIMAL)
            v = operando.datos.decimal;
        else if (operando.tipo == VALOR_LOGICA)
            v = operando.datos.logica ? 1.0 : 0.0;
        return valor_crear_logica(v == 0);
    }
    case OP_BIT_NOT:
    {
        unsigned long long val = 0;
        if (operando.tipo == VALOR_ENTERO)
            val = (unsigned long long)operando.datos.entero;
        else if (operando.tipo == VALOR_ENTERO_SIN_SIGNO)
            val = operando.datos.entero_sin_signo;
        else if (operando.tipo == VALOR_DECIMAL)
            val = (unsigned long long)operando.datos.decimal;
        else if (operando.tipo == VALOR_DECIMAL_SIN_SIGNO)
            val = (unsigned long long)operando.datos.decimal_sin_signo;
        return valor_crear_entero_sin_signo(~val);
    }
    default:
        contexto_set_error(ctx, "Operador unario no soportado");
        return valor_crear_vacio();
    }
}

// Evalúa una expresión matemática completa dentro de corchetes
static char *evaluar_expresion_completa(const char *expr, Contexto *ctx)
{
    if (!expr || strlen(expr) == 0)
        return strdup("");

    // Crear un lexer temporal para la expresión
    Lexer *lexer_expr = lexer_crear(expr);
    if (!lexer_expr)
        return strdup(expr);

    // Crear un parser temporal
    Parser *parser_expr = parser_crear(lexer_expr);
    if (!parser_expr)
    {
        lexer_destruir(lexer_expr);
        return strdup(expr);
    }

    // Parsear la expresión
    NodoAST *ast_expr = parsear_expresion(parser_expr);

    if (parser_tiene_error(parser_expr) || !ast_expr)
    {
        // Si hay error de parsing, devolver la expresión original
        parser_destruir(parser_expr);
        lexer_destruir(lexer_expr);
        return strdup(expr);
    }

    // Evaluar la expresión
    Valor resultado = evaluar_nodo(ast_expr, ctx);

    // Convertir resultado a string
    char buffer[256];
    switch (resultado.tipo)
    {
    case VALOR_ENTERO:
        snprintf(buffer, sizeof(buffer), "%lld", resultado.datos.entero);
        break;
    case VALOR_ENTERO_SIN_SIGNO:
        snprintf(buffer, sizeof(buffer), "%llu", resultado.datos.entero_sin_signo);
        break;
    case VALOR_DECIMAL:
        snprintf(buffer, sizeof(buffer), "%.15g", resultado.datos.decimal);
        break;
    case VALOR_DECIMAL_SIN_SIGNO:
        snprintf(buffer, sizeof(buffer), "%.15g", resultado.datos.decimal_sin_signo);
        break;
    case VALOR_TEXTO:
    case VALOR_TEXTO_EXTENSO:
        snprintf(buffer, sizeof(buffer), "%s", resultado.datos.texto ? resultado.datos.texto : "");
        break;
    case VALOR_LOGICA:
        snprintf(buffer, sizeof(buffer), "%s", resultado.datos.logica ? "VERDADERO" : "FALSO");
        break;
    case VALOR_CARACTER:
    case VALOR_CARACTER_SIN_SIGNO:
        snprintf(buffer, sizeof(buffer), "%s", resultado.datos.texto ? resultado.datos.texto : "");
        break;
    default:
        buffer[0] = '\0';
        break;
    }

    // Limpiar
    valor_destruir(&resultado);
    liberar_nodo(ast_expr);
    parser_destruir(parser_expr);
    lexer_destruir(lexer_expr);

    return strdup(buffer);
}

static Valor evaluar_bloque(NodoAST* bloque, Contexto* ctx);
static void evaluar_sentencia(NodoAST* nodo, Contexto* ctx);

// ============================================================
// EVALUACIÓN DE NODOS PRINCIPAL
// ============================================================
Valor evaluar_nodo(NodoAST* nodo, Contexto* ctx) {
    if (!nodo || ctx->hay_error) return valor_crear_vacio();

    ctx->linea_actual = nodo->linea;
    
    switch (nodo->tipo) {
        // --------------------------------------------------------
        // LITERALES
        // --------------------------------------------------------
        case AST_LITERAL_NUMERO:
        {
        // 1. Intentar usar el string original (el camino más seguro)
        if (nodo->datos.literal_numero.valor_str)
        {
            const char *str = nodo->datos.literal_numero.valor_str;
            if (strchr(str, '.') == NULL && strchr(str, 'e') == NULL && strchr(str, 'E') == NULL)
            {
                char *endptr;
                unsigned long long ull_val = strtoull(str, &endptr, 10);

                // Ignorar espacios en blanco o saltos de línea finales (crucial para evitar el fallback)
                while (*endptr == ' ' || *endptr == '\t' || *endptr == '\n' || *endptr == '\r')
                {
                    endptr++;
                }

                if (endptr > str && *endptr == '\0')
                {
                    if (ull_val <= LLONG_MAX)
                    {
                        return valor_crear_entero((long long)ull_val);
                    }
                    else
                    {
                        // Es un entero grande (> 2^63-1), usar sin signo directamente
                        return valor_crear_entero_sin_signo(ull_val);
                    }
                }
            }
        }

        // 2. Fallback seguro cuando el string está corrupto o no se pudo parsear
        double val = nodo->datos.literal_numero.valor;

        // Interceptamos los valores que caen en la "zona de desbordamiento"
        // de la conversión double -> long long, antes de que la comparación (double)LLONG_MAX falle.
        if (val >= 9223372036854775808.0)
        {
            if (val == 9223372036854775808.0)
            {
                return valor_crear_entero_sin_signo(9223372036854775808ULL);
            }
            if (val >= 18446744073709551615.0)
            {
                return valor_crear_entero_sin_signo(18446744073709551615ULL);
            }
            // Para cualquier otro valor grande en este rango, convertir directamente a unsigned
            return valor_crear_entero_sin_signo((unsigned long long)val);
        }

        // Fallback estándar para números normales
        if (val >= 0.0 && val <= (double)LLONG_MAX && val == floor(val))
        {
            return valor_crear_entero((long long)val);
        }

        return valor_crear_decimal(val);
        }
        case AST_LITERAL_TEXTO:
            return valor_crear_texto(nodo->datos.literal_texto.valor);

        case AST_LITERAL_CARACTER:
            return valor_crear_caracter(nodo->datos.literal_caracter.valor);

        case AST_LITERAL_LOGICO:
            return valor_crear_logica(nodo->datos.literal_logico.valor);
        
        // --------------------------------------------------------
        // VARIABLES
        // --------------------------------------------------------
        case AST_VARIABLE:
        {
            const char *nombre = nodo->datos.variable.nombre;
            Valor *valor = tabla_simbolos_buscar(ctx->tabla_actual, nombre);

            if (!valor)
            {
                contexto_set_error(ctx, "Variable no definida");
                return valor_crear_vacio();
            }

            // Hacer copia profunda para evitar double free
            if (valor->tipo == VALOR_TEXTO || valor->tipo == VALOR_TEXTO_EXTENSO)
            {
                return valor_crear_texto(valor->datos.texto ? valor->datos.texto : "");
            }
            else if (valor->tipo == VALOR_ARCHIVO)
            {
                // No copiar el archivo, solo la referencia
                Valor copia = *valor;
                return copia;
            }
            else
            {
                return *valor;
            }
        }
        case AST_ACCESO_LISTA:
        {
            const char *nombre_lista = nodo->datos.acceso_lista.nombre_lista;
            NodoAST *indice_nodo = nodo->datos.acceso_lista.indice;

            // Buscar la lista en la tabla de símbolos
            Valor *lista = tabla_simbolos_buscar(ctx->tabla_actual, nombre_lista);
            if (!lista || lista->tipo != VALOR_LISTA)
            {
                contexto_set_error(ctx, "Lista no encontrada o no es una lista");
                return valor_crear_vacio();
            }

            // Evaluar el índice
            Valor indice = evaluar_nodo(indice_nodo, ctx);
            if (ctx->hay_error)
                return valor_crear_vacio();

            // Aceptar cualquier tipo numérico como índice
            long long idx = 0;
            if (indice.tipo == VALOR_ENTERO)
            {
                idx = indice.datos.entero;
            }
            else if (indice.tipo == VALOR_ENTERO_SIN_SIGNO)
            {
                idx = (int)indice.datos.entero_sin_signo;
            }
            else if (indice.tipo == VALOR_DECIMAL)
            {
                idx = (int)indice.datos.decimal;
            }
            else if (indice.tipo == VALOR_DECIMAL_SIN_SIGNO)
            {
                idx = (int)indice.datos.decimal_sin_signo;
            }
            else
            {
                contexto_set_error(ctx, "Índice de lista debe ser numérico");
                valor_destruir(&indice);
                return valor_crear_vacio();
            }

            // Verificar bounds
            if (idx < 0 || idx >= lista->tamano)
            {
                char msg[128];
                snprintf(msg, sizeof(msg), "Índice %lld fuera de rango (0-%d)", idx, lista->tamano - 1);
                contexto_set_error(ctx, msg);
                valor_destruir(&indice);
                return valor_crear_vacio();
            }

            // Devolver una copia del valor
            Valor elemento = lista->datos.lista[idx];
            valor_destruir(&indice);

            // Hacer copia profunda si es texto para evitar double free
            if (elemento.tipo == VALOR_TEXTO || elemento.tipo == VALOR_TEXTO_EXTENSO)
            {
                return valor_crear_texto(elemento.datos.texto ? elemento.datos.texto : "");
            }
            return elemento;
        }

        case AST_ACCESO_MATRIZ:
        {
            const char *nombre_matriz = nodo->datos.acceso_matriz.nombre_matriz;
            NodoAST *fila_nodo = nodo->datos.acceso_matriz.fila;
            NodoAST *col_nodo = nodo->datos.acceso_matriz.columna;

            // Buscar la matriz en la tabla de símbolos
            Valor *matriz = tabla_simbolos_buscar(ctx->tabla_actual, nombre_matriz);
            if (!matriz || matriz->tipo != VALOR_MATRIZ)
            {
                contexto_set_error(ctx, "Matriz no encontrada o no es una matriz");
                return valor_crear_vacio();
            }

            // Evaluar fila
            Valor fila_v = evaluar_nodo(fila_nodo, ctx);
            if (ctx->hay_error)
                return valor_crear_vacio();

            // Evaluar columna
            Valor col_v = evaluar_nodo(col_nodo, ctx);
            if (ctx->hay_error)
            {
                valor_destruir(&fila_v);
                return valor_crear_vacio();
            }

            // Convertir a int (aceptando cualquier tipo numérico)
            int fila = 0, col = 0;
            if (fila_v.tipo == VALOR_ENTERO)
                fila = fila_v.datos.entero;
            else if (fila_v.tipo == VALOR_ENTERO_SIN_SIGNO)
                fila = (int)fila_v.datos.entero_sin_signo;
            else if (fila_v.tipo == VALOR_DECIMAL)
                fila = (int)fila_v.datos.decimal;
            else if (fila_v.tipo == VALOR_DECIMAL_SIN_SIGNO)
                fila = (int)fila_v.datos.decimal_sin_signo;

            if (col_v.tipo == VALOR_ENTERO)
                col = col_v.datos.entero;
            else if (col_v.tipo == VALOR_ENTERO_SIN_SIGNO)
                col = (int)col_v.datos.entero_sin_signo;
            else if (col_v.tipo == VALOR_DECIMAL)
                col = (int)col_v.datos.decimal;
            else if (col_v.tipo == VALOR_DECIMAL_SIN_SIGNO)
                col = (int)col_v.datos.decimal_sin_signo;

            valor_destruir(&fila_v);
            valor_destruir(&col_v);

            // Verificar bounds
            if (fila < 0 || fila >= matriz->filas || col < 0 || col >= matriz->columnas)
            {
                char msg[128];
                snprintf(msg, sizeof(msg), "Índice [%d][%d] fuera de rango", fila, col);
                contexto_set_error(ctx, msg);
                return valor_crear_vacio();
            }

            // Devolver una copia del valor
            Valor elemento = matriz->datos.matriz[fila][col];
            if (elemento.tipo == VALOR_TEXTO || elemento.tipo == VALOR_TEXTO_EXTENSO)
            {
                return valor_crear_texto(elemento.datos.texto ? elemento.datos.texto : "");
            }
            return elemento;
        }

        case AST_ACCESO_MATRIZ3D:
        {
            const char *nombre_matriz = nodo->datos.acceso_matriz3d.nombre_matriz;
            NodoAST *indice1_nodo = nodo->datos.acceso_matriz3d.indice1;
            NodoAST *indice2_nodo = nodo->datos.acceso_matriz3d.indice2;
            NodoAST *indice3_nodo = nodo->datos.acceso_matriz3d.indice3;

            // Buscar la matriz 3D en la tabla de símbolos
            Valor *matriz3d = tabla_simbolos_buscar(ctx->tabla_actual, nombre_matriz);
            if (!matriz3d || matriz3d->tipo != VALOR_MATRIZ3D)
            {
                contexto_set_error(ctx, "Matriz 3D no encontrada o no es una matriz 3D");
                return valor_crear_vacio();
            }

            // Evaluar índice 1
            Valor indice1_v = evaluar_nodo(indice1_nodo, ctx);
            if (ctx->hay_error)
                return valor_crear_vacio();

            // Evaluar índice 2
            Valor indice2_v = evaluar_nodo(indice2_nodo, ctx);
            if (ctx->hay_error)
            {
                valor_destruir(&indice1_v);
                return valor_crear_vacio();
            }

            // Evaluar índice 3
            Valor indice3_v = evaluar_nodo(indice3_nodo, ctx);
            if (ctx->hay_error)
            {
                valor_destruir(&indice1_v);
                valor_destruir(&indice2_v);
                return valor_crear_vacio();
            }

            // Convertir a int (aceptando cualquier tipo numérico)
            int idx1 = 0, idx2 = 0, idx3 = 0;

            if (indice1_v.tipo == VALOR_ENTERO)
                idx1 = indice1_v.datos.entero;
            else if (indice1_v.tipo == VALOR_ENTERO_SIN_SIGNO)
                idx1 = (int)indice1_v.datos.entero_sin_signo;
            else if (indice1_v.tipo == VALOR_DECIMAL)
                idx1 = (int)indice1_v.datos.decimal;
            else if (indice1_v.tipo == VALOR_DECIMAL_SIN_SIGNO)
                idx1 = (int)indice1_v.datos.decimal_sin_signo;

            if (indice2_v.tipo == VALOR_ENTERO)
                idx2 = indice2_v.datos.entero;
            else if (indice2_v.tipo == VALOR_ENTERO_SIN_SIGNO)
                idx2 = (int)indice2_v.datos.entero_sin_signo;
            else if (indice2_v.tipo == VALOR_DECIMAL)
                idx2 = (int)indice2_v.datos.decimal;
            else if (indice2_v.tipo == VALOR_DECIMAL_SIN_SIGNO)
                idx2 = (int)indice2_v.datos.decimal_sin_signo;

            if (indice3_v.tipo == VALOR_ENTERO)
                idx3 = indice3_v.datos.entero;
            else if (indice3_v.tipo == VALOR_ENTERO_SIN_SIGNO)
                idx3 = (int)indice3_v.datos.entero_sin_signo;
            else if (indice3_v.tipo == VALOR_DECIMAL)
                idx3 = (int)indice3_v.datos.decimal;
            else if (indice3_v.tipo == VALOR_DECIMAL_SIN_SIGNO)
                idx3 = (int)indice3_v.datos.decimal_sin_signo;

            valor_destruir(&indice1_v);
            valor_destruir(&indice2_v);
            valor_destruir(&indice3_v);

            // Verificar bounds
            if (idx1 < 0 || idx1 >= matriz3d->filas ||
                idx2 < 0 || idx2 >= matriz3d->columnas ||
                idx3 < 0 || idx3 >= matriz3d->profundidad)
            {
                char msg[128];
                snprintf(msg, sizeof(msg), "Índice [%d][%d][%d] fuera de rango", idx1, idx2, idx3);
                contexto_set_error(ctx, msg);
                return valor_crear_vacio();
            }

            // Devolver una copia del valor
            Valor elemento = matriz3d->datos.matriz3d[idx1][idx2][idx3];
            if (elemento.tipo == VALOR_TEXTO || elemento.tipo == VALOR_TEXTO_EXTENSO)
            {
                return valor_crear_texto(elemento.datos.texto ? elemento.datos.texto : "");
            }
            return elemento;
        }

        // --------------------------------------------------------
        // OPERADORES
        // --------------------------------------------------------
        case AST_OPERADOR_BINARIO: {
            Valor izq = evaluar_nodo(nodo->datos.operador_binario.izquierdo, ctx);
            if (ctx->hay_error) return valor_crear_vacio();
            
            Valor der = evaluar_nodo(nodo->datos.operador_binario.derecho, ctx);
            if (ctx->hay_error) return valor_crear_vacio();
            
            return evaluar_operador_binario(
                nodo->datos.operador_binario.operador, izq, der, ctx
            );
        }
        
        case AST_OPERADOR_UNARIO: {
            Valor operando = evaluar_nodo(nodo->datos.operador_unario.operando, ctx);
            if (ctx->hay_error) return valor_crear_vacio();
            
            return evaluar_operador_unario(
                nodo->datos.operador_unario.operador, operando, ctx
            );
        }

        case AST_LEER:
        {
            const char *nombre = nodo->datos.leer.variable;
            char buffer[4096];

            // Restaurar terminal a modo normal para leer entrada
#ifndef _WIN32
            if (teclado_modo_raw)
            {
                tcsetattr(STDIN_FILENO, TCSANOW, &teclado_config_original);
            }
#endif

            if (!fgets(buffer, sizeof(buffer), stdin))
            {
                buffer[0] = '\0';
            }

            // Volver a modo raw después de leer
#ifndef _WIN32
            if (teclado_modo_raw)
            {
                struct termios newt = teclado_config_original;
                cfmakeraw(&newt);
                newt.c_cc[VMIN] = 0;
                newt.c_cc[VTIME] = 0;
                newt.c_oflag |= OPOST | ONLCR; // Mantener post-procesamiento de salida
                tcsetattr(STDIN_FILENO, TCSANOW, &newt);
            }
#endif

            // Quitar salto de línea
            size_t len = strlen(buffer);
            if (len > 0 && buffer[len - 1] == '\n')
                buffer[len - 1] = '\0';
            if (len > 1 && buffer[len - 2] == '\r')
                buffer[len - 2] = '\0';

            // Buscar la variable existente para saber su tipo
            Valor *existente = tabla_simbolos_buscar(ctx->tabla_actual, nombre);

            if (existente)
            {
                switch (existente->tipo)
                {
                case VALOR_ENTERO:
                    tabla_simbolos_definir(ctx->tabla_actual, nombre,
                                           valor_crear_entero(atoi(buffer)), false);
                    break;
                case VALOR_DECIMAL:
                    tabla_simbolos_definir(ctx->tabla_actual, nombre,
                                           valor_crear_decimal(atof(buffer)), false);
                    break;
                case VALOR_CARACTER:
                {
                    char buffer_caracter[2] = {buffer[0], '\0'};
                    tabla_simbolos_definir(ctx->tabla_actual, nombre,
                                           valor_crear_caracter(buffer_caracter), false);
                    break;
                }
                case VALOR_TEXTO:
                case VALOR_TEXTO_EXTENSO:
                    tabla_simbolos_definir(ctx->tabla_actual, nombre,
                                           valor_crear_texto(buffer), false);
                    break;
                default:
                    tabla_simbolos_definir(ctx->tabla_actual, nombre,
                                           valor_crear_texto(buffer), false);
                    break;
                }
            }
            else
            {
                // Variable no declarada, asumir texto
                tabla_simbolos_definir(ctx->tabla_actual, nombre,
                                       valor_crear_texto(buffer), false);
            }

            return valor_crear_vacio();
        }

        case AST_LEERCARACTER:
        {
            const char *nombre = nodo->datos.leercaracter.variable;

            // Restaurar terminal a modo normal para leer carácter
#ifndef _WIN32
            if (teclado_modo_raw)
            {
                tcsetattr(STDIN_FILENO, TCSANOW, &teclado_config_original);
            }
#endif

            int c = getchar();

            // Si hay un newline después del carácter, consumirlo
            if (c != '\n' && c != EOF)
            {
                int next = getchar();
                if (next != '\n' && next != EOF)
                {
                    ungetc(next, stdin);
                }
            }

            // Volver a modo raw después de leer
#ifndef _WIN32
            if (teclado_modo_raw)
            {
                struct termios newt = teclado_config_original;
                cfmakeraw(&newt);
                newt.c_cc[VMIN] = 0;
                newt.c_cc[VTIME] = 0;
                newt.c_oflag |= OPOST | ONLCR;
                tcsetattr(STDIN_FILENO, TCSANOW, &newt);
            }
#endif

            // Convertir char a string para UTF-8
            char buffer_caracter[2] = {(char)c, '\0'};
            tabla_simbolos_definir(ctx->tabla_actual, nombre,
                                   valor_crear_caracter(buffer_caracter), false);

            return valor_crear_vacio();
        }

        case AST_LEERHASTA:
        {
            const char *var_name = nodo->datos.leerhasta.variable;
            Valor delimitador_val = evaluar_nodo(nodo->datos.leerhasta.delimitador, ctx);

            if (delimitador_val.tipo != VALOR_TEXTO)
            {
                contexto_set_error(ctx, "LEERHASTA: el delimitador debe ser texto");
                return valor_crear_vacio();
            }

            const char *delimitador = delimitador_val.datos.texto;
            char buffer[4096] = {0};
            size_t pos = 0;

            // Restaurar terminal a modo normal para leer
#ifndef _WIN32
            if (teclado_modo_raw)
            {
                tcsetattr(STDIN_FILENO, TCSANOW, &teclado_config_original);
            }
#endif

            // Leer línea por línea hasta encontrar el delimitador
            char linea[1024];
            while (fgets(linea, sizeof(linea), stdin))
            {
                // Remover salto de línea
                size_t len = strlen(linea);
                if (len > 0 && linea[len - 1] == '\n')
                {
                    linea[len - 1] = '\0';
                    len--;
                }

                // Buscar el delimitador DENTRO de la línea
                char *pos_delim = strstr(linea, delimitador);

                if (pos_delim != NULL)
                {
                    // Delimitador encontrado en esta línea
                    size_t longitud_texto = pos_delim - linea;

                    // Copiar solo la parte antes del delimitador
                    if (pos + longitud_texto < sizeof(buffer))
                    {
                        if (pos > 0 && longitud_texto > 0)
                        {
                            buffer[pos++] = '\n';
                        }
                        strncpy(buffer + pos, linea, longitud_texto);
                        pos += longitud_texto;
                        buffer[pos] = '\0';
                    }
                    break; // Detener la lectura
                }
                else
                {
                    // Delimitador no encontrado, agregar línea completa
                    if (pos + len + 1 < sizeof(buffer))
                    {
                        if (pos > 0)
                        {
                            buffer[pos++] = '\n';
                        }
                        strcpy(buffer + pos, linea);
                        pos += len;
                    }
                }
            }

            // Volver a modo raw después de leer
#ifndef _WIN32
            if (teclado_modo_raw)
            {
                struct termios newt = teclado_config_original;
                cfmakeraw(&newt);
                newt.c_cc[VMIN] = 0;
                newt.c_cc[VTIME] = 0;
                newt.c_oflag |= OPOST | ONLCR;
                tcsetattr(STDIN_FILENO, TCSANOW, &newt);
            }
#endif

            // Guardar en la variable
            Valor resultado = valor_crear_texto(strdup(buffer));
            tabla_simbolos_asignar(ctx->tabla_actual, var_name, resultado);

            valor_destruir(&delimitador_val);
            return valor_crear_vacio();
        }

        case AST_COLISIONRECTANGULOS:
        {
            Valor x1_v = evaluar_nodo(nodo->datos.colisionrectangulos.x1, ctx);
            Valor y1_v = evaluar_nodo(nodo->datos.colisionrectangulos.y1, ctx);
            Valor a1_v = evaluar_nodo(nodo->datos.colisionrectangulos.ancho1, ctx);
            Valor h1_v = evaluar_nodo(nodo->datos.colisionrectangulos.alto1, ctx);
            Valor x2_v = evaluar_nodo(nodo->datos.colisionrectangulos.x2, ctx);
            Valor y2_v = evaluar_nodo(nodo->datos.colisionrectangulos.y2, ctx);
            Valor a2_v = evaluar_nodo(nodo->datos.colisionrectangulos.ancho2, ctx);
            Valor h2_v = evaluar_nodo(nodo->datos.colisionrectangulos.alto2, ctx);

            int x1 = (x1_v.tipo == VALOR_ENTERO) ? x1_v.datos.entero : (int)x1_v.datos.decimal;
            int y1 = (y1_v.tipo == VALOR_ENTERO) ? y1_v.datos.entero : (int)y1_v.datos.decimal;
            int a1 = (a1_v.tipo == VALOR_ENTERO) ? a1_v.datos.entero : (int)a1_v.datos.decimal;
            int h1 = (h1_v.tipo == VALOR_ENTERO) ? h1_v.datos.entero : (int)h1_v.datos.decimal;
            int x2 = (x2_v.tipo == VALOR_ENTERO) ? x2_v.datos.entero : (int)x2_v.datos.decimal;
            int y2 = (y2_v.tipo == VALOR_ENTERO) ? y2_v.datos.entero : (int)y2_v.datos.decimal;
            int a2 = (a2_v.tipo == VALOR_ENTERO) ? a2_v.datos.entero : (int)a2_v.datos.decimal;
            int h2 = (h2_v.tipo == VALOR_ENTERO) ? h2_v.datos.entero : (int)h2_v.datos.decimal;


            // Algoritmo AABB (Axis-Aligned Bounding Box)
            int colision = (x1 < x2 + a2 && x1 + a1 > x2 &&
                            y1 < y2 + h2 && y1 + h1 > y2)
                               ? 1
                               : 0;


            // Definir/actualizar variable con el resultado
            const char *var_name = nodo->datos.colisionrectangulos.variable_resultado;

            tabla_simbolos_definir(ctx->tabla_actual, var_name, valor_crear_entero(colision), false);

            return valor_crear_vacio();
        }

        case AST_LEERTECLA:
        {
            const char *variable = nodo->datos.leertecla.variable;
            int resultado = 0;

            teclado_actualizar_buffer();

            if (teclado_buffer_len > 0)
            {
                // Buscar teclas especiales primero (codificadas en 2 bytes)
                for (int i = 0; i < teclado_buffer_len - 1; i++)
                {
                    if (teclado_buffer[i] >= 3 && teclado_buffer[i] <= 4)
                    {
                        resultado = (teclado_buffer[i] << 8) | teclado_buffer[i + 1];
                        // Remover los 2 bytes del buffer
                        for (int j = i; j < teclado_buffer_len - 2; j++)
                            teclado_buffer[j] = teclado_buffer[j + 2];
                        teclado_buffer_len -= 2;
                        goto leertecla_asignar;
                    }
                }

                // Si no hay tecla especial, tomar el primer byte normal
                resultado = teclado_buffer[0];
                for (int i = 0; i < teclado_buffer_len - 1; i++)
                    teclado_buffer[i] = teclado_buffer[i + 1];
                teclado_buffer_len--;
            }

        leertecla_asignar:
            tabla_simbolos_definir(ctx->tabla_actual, variable,
                                   valor_crear_entero(resultado), false);
            return valor_crear_vacio();
        }

        case AST_COLORTEXTO:
        {
            const char *color = nodo->datos.colortexto.color;
            int codigo = -1;

            if (strcasecmp(color, "negro") == 0)
                codigo = 30;
            else if (strcasecmp(color, "rojo") == 0)
                codigo = 31;
            else if (strcasecmp(color, "verde") == 0)
                codigo = 32;
            else if (strcasecmp(color, "amarillo") == 0)
                codigo = 33;
            else if (strcasecmp(color, "azul") == 0)
                codigo = 34;
            else if (strcasecmp(color, "magenta") == 0)
                codigo = 35;
            else if (strcasecmp(color, "cyan") == 0)
                codigo = 36;
            else if (strcasecmp(color, "blanco") == 0)
                codigo = 37;
            else if (strcasecmp(color, "gris") == 0)
                codigo = 90;
            else if (strcasecmp(color, "rojoclaro") == 0)
                codigo = 91;
            else if (strcasecmp(color, "verdeclaro") == 0)
                codigo = 92;
            else if (strcasecmp(color, "amarilloclaro") == 0)
                codigo = 93;
            else if (strcasecmp(color, "azulclaro") == 0)
                codigo = 94;
            else if (strcasecmp(color, "magentaclaro") == 0)
                codigo = 95;
            else if (strcasecmp(color, "cyanclaro") == 0)
                codigo = 96;
            else if (strcasecmp(color, "blancoclaro") == 0)
                codigo = 97;

            if (codigo >= 0)
                printf("\033[%dm", codigo);
            else
                contexto_set_error(ctx, "Color de texto no válido");
            fflush(stdout);
            return valor_crear_vacio();
        }
        case AST_COLORFONDO:
        {
            const char *color = nodo->datos.colorfondo.color;
            int codigo = -1;

            if (strcasecmp(color, "negro") == 0)
                codigo = 40;
            else if (strcasecmp(color, "rojo") == 0)
                codigo = 41;
            else if (strcasecmp(color, "verde") == 0)
                codigo = 42;
            else if (strcasecmp(color, "amarillo") == 0)
                codigo = 43;
            else if (strcasecmp(color, "azul") == 0)
                codigo = 44;
            else if (strcasecmp(color, "magenta") == 0)
                codigo = 45;
            else if (strcasecmp(color, "cyan") == 0)
                codigo = 46;
            else if (strcasecmp(color, "blanco") == 0)
                codigo = 47;
            else if (strcasecmp(color, "gris") == 0)
                codigo = 100;
            else if (strcasecmp(color, "rojoclaro") == 0)
                codigo = 101;
            else if (strcasecmp(color, "verdeclaro") == 0)
                codigo = 102;
            else if (strcasecmp(color, "amarilloclaro") == 0)
                codigo = 103;
            else if (strcasecmp(color, "azulclaro") == 0)
                codigo = 104;
            else if (strcasecmp(color, "magentaclaro") == 0)
                codigo = 105;
            else if (strcasecmp(color, "cyanclaro") == 0)
                codigo = 106;
            else if (strcasecmp(color, "blancoclaro") == 0)
                codigo = 107;

            if (codigo >= 0)
                printf("\033[%dm", codigo);
            else
                contexto_set_error(ctx, "Color de fondo no válido");
            fflush(stdout);
            return valor_crear_vacio();
        }
        case AST_TEXTONEGRITA:
            printf("\033[1m"); fflush(stdout);
            return valor_crear_vacio();

        case AST_TEXTOCURSIVA:
            printf("\033[3m"); fflush(stdout);
            return valor_crear_vacio();

        case AST_TEXTOSUBRAYADO:
            printf("\033[4m"); fflush(stdout);
            return valor_crear_vacio();

        case AST_RESETTEXTO:
        case AST_RESETCOLOR:
            printf("\033[0m"); fflush(stdout);
            return valor_crear_vacio();

        case AST_DIBUJARLINEA:
        {
            Valor x1_v = evaluar_nodo(nodo->datos.dibujarlinea.x1, ctx);
            Valor y1_v = evaluar_nodo(nodo->datos.dibujarlinea.y1, ctx);
            Valor x2_v = evaluar_nodo(nodo->datos.dibujarlinea.x2, ctx);
            Valor y2_v = evaluar_nodo(nodo->datos.dibujarlinea.y2, ctx);
            Valor patron_v = evaluar_nodo(nodo->datos.dibujarlinea.patron, ctx);

            int x1 = (x1_v.tipo == VALOR_ENTERO) ? x1_v.datos.entero : (int)x1_v.datos.decimal;
            int y1 = (y1_v.tipo == VALOR_ENTERO) ? y1_v.datos.entero : (int)y1_v.datos.decimal;
            int x2 = (x2_v.tipo == VALOR_ENTERO) ? x2_v.datos.entero : (int)x2_v.datos.decimal;
            int y2 = (y2_v.tipo == VALOR_ENTERO) ? y2_v.datos.entero : (int)y2_v.datos.decimal;

            // Obtener el patrón como string
            const char *patron = "#";
            if (patron_v.tipo == VALOR_TEXTO || patron_v.tipo == VALOR_CARACTER)
            {
                patron = patron_v.datos.texto;
            }

            // Algoritmo de Bresenham
            int dx = abs(x2 - x1);
            int dy = abs(y2 - y1);
            int sx = (x1 < x2) ? 1 : -1;
            int sy = (y1 < y2) ? 1 : -1;
            int err = dx - dy;

            while (1)
            {
                printf("\033[%d;%dH%s", y1, x1, patron);
                if (x1 == x2 && y1 == y2)
                    break;
                int e2 = 2 * err;
                if (e2 > -dy)
                {
                    err -= dy;
                    x1 += sx;
                }
                if (e2 < dx)
                {
                    err += dx;
                    y1 += sy;
                }
            }
            fflush(stdout);

            // NO liberar patron_v porque puede apuntar a datos en la tabla de símbolos
            return valor_crear_vacio();
        }

        case AST_DIBUJARCIRCULO:
        {
            Valor cx_v = evaluar_nodo(nodo->datos.dibujarcirculo.centro_x, ctx);
            Valor cy_v = evaluar_nodo(nodo->datos.dibujarcirculo.centro_y, ctx);
            Valor radio_v = evaluar_nodo(nodo->datos.dibujarcirculo.radio, ctx);
            Valor patron_v = evaluar_nodo(nodo->datos.dibujarcirculo.patron, ctx);

            int cx = (cx_v.tipo == VALOR_ENTERO) ? cx_v.datos.entero : (int)cx_v.datos.decimal;
            int cy = (cy_v.tipo == VALOR_ENTERO) ? cy_v.datos.entero : (int)cy_v.datos.decimal;
            int radio = (radio_v.tipo == VALOR_ENTERO) ? radio_v.datos.entero : (int)radio_v.datos.decimal;

            const char *patron = "#";
            if (patron_v.tipo == VALOR_TEXTO || patron_v.tipo == VALOR_CARACTER)
            {
                patron = patron_v.datos.texto;
            }

            const double ASPECT_RATIO = 1.75;

            for (int angle = 0; angle < 360; angle++)
            {
                double rad = angle * 3.14159265358979323846 / 180.0;
                int x = (int)round(cx + radio * cos(rad) * ASPECT_RATIO);
                int y = (int)round(cy + radio * sin(rad));

                printf("\033[%d;%dH%s", y, x, patron);
            }

            fflush(stdout);
            return valor_crear_vacio();
        }

        case AST_RELLENARRECTANGULO:
        {
            Valor x1_v = evaluar_nodo(nodo->datos.rellenarrectangulo.x1, ctx);
            Valor y1_v = evaluar_nodo(nodo->datos.rellenarrectangulo.y1, ctx);
            Valor x2_v = evaluar_nodo(nodo->datos.rellenarrectangulo.x2, ctx);
            Valor y2_v = evaluar_nodo(nodo->datos.rellenarrectangulo.y2, ctx);
            Valor patron_v = evaluar_nodo(nodo->datos.rellenarrectangulo.patron, ctx);

            int x1 = (x1_v.tipo == VALOR_ENTERO) ? x1_v.datos.entero : (int)x1_v.datos.decimal;
            int y1 = (y1_v.tipo == VALOR_ENTERO) ? y1_v.datos.entero : (int)y1_v.datos.decimal;
            int x2 = (x2_v.tipo == VALOR_ENTERO) ? x2_v.datos.entero : (int)x2_v.datos.decimal;
            int y2 = (y2_v.tipo == VALOR_ENTERO) ? y2_v.datos.entero : (int)y2_v.datos.decimal;

            const char *patron = "";
            if (patron_v.tipo == VALOR_TEXTO && patron_v.datos.texto)
            {
                patron = patron_v.datos.texto;
            }

            // Rellenar el rectángulo
            for (int y = y1; y <= y2; y++)
            {
                printf("\033[%d;%dH", y, x1); // Mover cursor
                for (int x = x1; x <= x2; x++)
                {
                    printf("%s", patron);
                }
            }
            printf("\033[0m"); // Reset colores

            fflush(stdout);
            return valor_crear_vacio();
        }

        case AST_CURSOR:
        case AST_POSICIONAR: {
            NodoAST* fila_nodo = (nodo->tipo == AST_CURSOR) ? 
            nodo->datos.cursor.fila : nodo->datos.posicionar.fila;
            NodoAST* col_nodo = (nodo->tipo == AST_CURSOR) ? 
            nodo->datos.cursor.columna : nodo->datos.posicionar.columna;
    
            Valor fila_v = evaluar_nodo(fila_nodo, ctx);
            Valor col_v = evaluar_nodo(col_nodo, ctx);
    
            int fila = (fila_v.tipo == VALOR_ENTERO) ? fila_v.datos.entero : (int)fila_v.datos.decimal;
            int col = (col_v.tipo == VALOR_ENTERO) ? col_v.datos.entero : (int)col_v.datos.decimal;

#ifdef _WIN32
                HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
                COORD coord = { (SHORT)(col - 1), (SHORT)(fila - 1) };
                SetConsoleCursorPosition(hConsole, coord);
#else
        printf("\033[%d;%dH", fila, col);
#endif
            fflush(stdout);
            return valor_crear_vacio();
        }

        case AST_OCULTARCURSOR:
#ifdef _WIN32
            {
                HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
                CONSOLE_CURSOR_INFO cci;
                GetConsoleCursorInfo(hConsole, &cci);
                cci.bVisible = FALSE;
                SetConsoleCursorInfo(hConsole, &cci);
            }
#else
        printf("\033[?25l");
#endif
            fflush(stdout);
            return valor_crear_vacio();

        case AST_MOSTRARCURSOR:
#ifdef _WIN32
            {
                HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
                CONSOLE_CURSOR_INFO cci;
                GetConsoleCursorInfo(hConsole, &cci);
                cci.bVisible = TRUE;
                SetConsoleCursorInfo(hConsole, &cci);
            }
#else
        printf("\033[?25h");
#endif
            fflush(stdout);
            return valor_crear_vacio();

        case AST_ANCHOTERMINAL:
        case AST_ALTOTERMINAL: {
            const char* nombre = (nodo->tipo == AST_ANCHOTERMINAL) ? 
            nodo->datos.anchoterminal.variable : nodo->datos.altoterminal.variable;
    
            int valor = 80; // default
#ifdef _WIN32
                CONSOLE_SCREEN_BUFFER_INFO csbi;
                if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
                    valor = (nodo->tipo == AST_ANCHOTERMINAL) ? 
                    (csbi.srWindow.Right - csbi.srWindow.Left + 1) :
                    (csbi.srWindow.Bottom - csbi.srWindow.Top + 1);
                }
#else
        struct winsize ws;
        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0)
        {
            valor = (nodo->tipo == AST_ANCHOTERMINAL) ? ws.ws_col : ws.ws_row;
        }
            #endif
    
            tabla_simbolos_definir(ctx->tabla_actual, nombre, 
            valor_crear_entero(valor), false);
            return valor_crear_vacio();
        }
 
        case AST_ESPERAR: {
            Valor cantidad = evaluar_nodo(nodo->datos.esperar.cantidad, ctx);
            const char* unidad = nodo->datos.esperar.unidad;
    
            double val = (cantidad.tipo == VALOR_ENTERO) ? 
            (double)cantidad.datos.entero : cantidad.datos.decimal;
    
            long long microsegundos = 0;
    
            if (strcmp(unidad, "MICROSEGUNDOS") == 0 || strcmp(unidad, "MICROS") == 0 || strcmp(unidad, "US") == 0) {
                microsegundos = (long long)val;
            } else if (strcmp(unidad, "MILISEGUNDOS") == 0 || strcmp(unidad, "MS") == 0) {
                microsegundos = (long long)(val * 1000);
            } else if (strcmp(unidad, "SEGUNDOS") == 0 || strcmp(unidad, "S") == 0) {
                microsegundos = (long long)(val * 1000000);
            } else if (strcmp(unidad, "MINUTOS") == 0 || strcmp(unidad, "MIN") == 0) {
                microsegundos = (long long)(val * 60000000);
            }
    
            if (microsegundos > 0) {
                #ifdef _WIN32
                    Sleep((DWORD)(microsegundos / 1000));
                #else
                    // Esperar en intervalos cortos para verificar Ctrl+C
                    long long restante = microsegundos;
                    while (restante > 0) {                       
                        // Dormir 10ms o lo que reste
                        useconds_t dormir = (restante > 10000) ? 10000 : (useconds_t)restante;
                        usleep(dormir);
                        restante -= dormir;
                    }
                #endif
            }
            return valor_crear_vacio();
        }

        case AST_TIEMPOMS: {
            const char* nombre = nodo->datos.tiempoms.variable;
    
            long long ms = 0;
            #ifdef _WIN32
                ms = (long long)GetTickCount64();
            #else
                struct timespec ts;
                if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
                    ms = (long long)(ts.tv_sec * 1000LL + ts.tv_nsec / 1000000LL);
                }
            #endif
    
            tabla_simbolos_definir(ctx->tabla_actual, nombre, 
            valor_crear_entero((int)ms), false);
            return valor_crear_vacio();
        }
        
        case AST_SISTEMA: {
            const char* comando = nodo->datos.sistema.comando;
            if (comando) {
                int ret = system(comando);
                (void)ret;
            }
            return valor_crear_vacio();
        }
        case AST_ETIQUETA:
        {
            // Las etiquetas son solo marcadores
            return valor_crear_vacio();
        }

        case AST_SALTAR_A:
        {
            const char *nombre_etiqueta = nodo->datos.saltar_a.nombre_etiqueta;

            // Verificar que la etiqueta existe
            Etiqueta *etiqueta = buscar_etiqueta(ctx, nombre_etiqueta);
            if (!etiqueta)
            {
                char msg[128];
                snprintf(msg, sizeof(msg), "Etiqueta '%s' no encontrada", nombre_etiqueta);
                contexto_set_error(ctx, msg);
                return valor_crear_vacio();
            }

            // Establecer el estado de flujo para saltar
            ctx->estado_flujo = FLUJO_SALTAR;
            if (ctx->etiqueta_salto)
                free(ctx->etiqueta_salto);
            ctx->etiqueta_salto = strdup(nombre_etiqueta);
            return valor_crear_vacio();
        }

        case AST_ABRIRARCHIVO:
        case AST_USARARCHIVO: {
            const char* var = (nodo->tipo == AST_ABRIRARCHIVO) ? 
            nodo->datos.abrirarchivo.variable : nodo->datos.usararchivo.variable;
            const char* ruta = (nodo->tipo == AST_ABRIRARCHIVO) ? 
            nodo->datos.abrirarchivo.ruta : nodo->datos.usararchivo.ruta;
            int modo = (nodo->tipo == AST_ABRIRARCHIVO) ? 
            nodo->datos.abrirarchivo.modo : nodo->datos.usararchivo.modo;
    
            const char* modo_c = "r";
            switch (modo) {
                case 0: modo_c = "w"; break;
                case 1: modo_c = "a"; break;
                case 2: modo_c = "r"; break;
                case 3: modo_c = "r+"; break;
            }
    
            FILE* archivo = fopen(ruta, modo_c);
            if (!archivo) {
                contexto_set_error(ctx, "No se pudo abrir el archivo");
                return valor_crear_vacio();
            }
    
            Valor v;
            v.tipo = VALOR_ARCHIVO;
            v.datos.archivo = archivo;
            v.modo_archivo = modo;
            v.tamano = 0;
            v.filas = 0;
            v.columnas = 0;
    
            tabla_simbolos_definir(ctx->tabla_actual, var, v, false);
            return valor_crear_vacio();
        }

        case AST_CERRARARCHIVO: {
            const char* var = nodo->datos.cerrararchivo.variable;
            Valor* v = tabla_simbolos_buscar(ctx->tabla_actual, var);
    
            if (v && v->tipo == VALOR_ARCHIVO && v->datos.archivo) {
                fclose(v->datos.archivo);
                v->datos.archivo = NULL;
                v->tipo = VALOR_VACIO;
            }
            return valor_crear_vacio();
        }

        case AST_ESCRIBIRARCHIVO: {
            const char* var = nodo->datos.escribirarchivo.variable;
            Valor* v = tabla_simbolos_buscar(ctx->tabla_actual, var);
    
            if (!v || v->tipo != VALOR_ARCHIVO || !v->datos.archivo) {
                contexto_set_error(ctx, "Archivo no abierto");
                return valor_crear_vacio();
            }
    
            Valor contenido = evaluar_nodo(nodo->datos.escribirarchivo.contenido, ctx);
    
            if (contenido.tipo == VALOR_TEXTO) {
                fprintf(v->datos.archivo, "%s\n", contenido.datos.texto);
            } else {
                char buffer[256];
                if (contenido.tipo == VALOR_ENTERO) snprintf(buffer, sizeof(buffer), "%lld", contenido.datos.entero);
                else if (contenido.tipo == VALOR_DECIMAL) snprintf(buffer, sizeof(buffer), "%g", contenido.datos.decimal);
                else buffer[0] = '\0';
                fprintf(v->datos.archivo, "%s\n", buffer);
            }
    
            fflush(v->datos.archivo);
            valor_destruir(&contenido);
            return valor_crear_vacio();
        }

        case AST_LEERARCHIVO: {
            const char* var_arch = nodo->datos.leerarchivo.variable_archivo;
            const char* var_dest = nodo->datos.leerarchivo.variable_destino;
    
            Valor* v = tabla_simbolos_buscar(ctx->tabla_actual, var_arch);
            if (!v || v->tipo != VALOR_ARCHIVO || !v->datos.archivo) {
                contexto_set_error(ctx, "Archivo no abierto");
                return valor_crear_vacio();
            }
    
            char buffer[4096];
            if (fgets(buffer, sizeof(buffer), v->datos.archivo)) {
                size_t len = strlen(buffer);
                if (len > 0 && buffer[len-1] == '\n') buffer[len-1] = '\0';
                tabla_simbolos_definir(ctx->tabla_actual, var_dest, valor_crear_texto(buffer), false);
            } else {
                tabla_simbolos_definir(ctx->tabla_actual, var_dest, valor_crear_texto(""), false);
            }
            return valor_crear_vacio();
        }

        // --------------------------------------------------------
        // LLAMADA A FUNCIÓN
        // --------------------------------------------------------
        case AST_LLAMADA_FUNCION:
        {
            const char *nombre = nodo->datos.llamada_funcion.nombre_funcion;

            Valor args[20];
            int num_args = 0;
            NodoAST *arg = nodo->datos.llamada_funcion.argumentos ? nodo->datos.llamada_funcion.argumentos->datos.bloque.primera : NULL;

            while (arg && num_args < 20)
            {
                args[num_args++] = evaluar_nodo(arg, ctx);
                if (ctx->hay_error)
                    return valor_crear_vacio();
                arg = arg->siguiente;
            }

            DefinicionFuncion *func = contexto_buscar_funcion(ctx, nombre);

            if (func)
            {
                // 1. INICIALIZAR TODO AL PRINCIPIO para evitar warnings y corrupción por goto
                Valor resultado = valor_crear_vacio();
                TablaSimbolos *tabla_anterior = ctx->tabla_actual;
                EstadoFlujo estado_anterior = ctx->estado_flujo;
                Valor retorno_anterior = ctx->valor_retorno;
                TablaSimbolos *tabla_local = NULL; // <--- CLAVE: Inicializar en NULL

                if (func->num_parametros != num_args)
                {
                    char msg[256];
                    snprintf(msg, sizeof(msg), "Función '%s' espera %d parámetros, se pasaron %d", nombre, func->num_parametros, num_args);
                    contexto_set_error(ctx, msg);
                    goto limpiar_funcion_usuario;
                }

                if (ctx->profundidad_llamada > 7500)
                {
                    contexto_set_error(ctx, "Recursión demasiado profunda (posible bucle infinito)");
                    goto limpiar_funcion_usuario;
                }

                // 2. AHORA SÍ, es seguro crear la tabla local
                tabla_local = malloc(sizeof(TablaSimbolos));
                tabla_local->primera = NULL;
                tabla_local->padre = ctx->tabla_global;

                ctx->tabla_actual = tabla_local;
                ctx->estado_flujo = FLUJO_NORMAL;
                ctx->valor_retorno = valor_crear_vacio();
                ctx->profundidad_llamada++;

                for (int i = 0; i < num_args; i++)
                {
                    tabla_simbolos_definir(ctx->tabla_actual, func->parametros[i].nombre, args[i], false);
                }

                if (func->bloque)
                {
                    evaluar_bloque(func->bloque, ctx);
                }

                resultado = ctx->valor_retorno;

                if (ctx->hay_error)
                {
                    goto limpiar_funcion_usuario;
                }

                // ===== VALIDACIÓN DE TIPO DE RETORNO =====
                if (func->tipo_retorno != TIPO_VACIO && resultado.tipo == VALOR_VACIO)
                {
                    char msg[256];
                    snprintf(msg, sizeof(msg), "La función '%s' debe retornar un valor pero no se ejecutó RETORNAR.", nombre);
                    contexto_set_error(ctx, msg);
                    valor_destruir(&resultado);
                    goto limpiar_funcion_usuario;
                }

                if (func->tipo_retorno == TIPO_VACIO && resultado.tipo != VALOR_VACIO)
                {
                    char msg[256];
                    snprintf(msg, sizeof(msg), "La función '%s' no debe retornar un valor pero se ejecutó RETORNAR con un valor.", nombre);
                    contexto_set_error(ctx, msg);
                    valor_destruir(&resultado);
                    goto limpiar_funcion_usuario;
                }

                if (func->tipo_retorno != TIPO_VACIO && resultado.tipo != VALOR_VACIO)
                {
                    if (!tipo_valor_compatible(func->tipo_retorno, resultado.tipo))
                    {
                        char msg[256];
                        snprintf(msg, sizeof(msg), "La función '%s' retornó un valor de tipo incompatible.", nombre);
                        contexto_set_error(ctx, msg);
                        valor_destruir(&resultado);
                        goto limpiar_funcion_usuario;
                    }

                    if (func->tipo_retorno == TIPO_ENTERO_SIN_SIGNO)
                    {
                        if ((resultado.tipo == VALOR_ENTERO && resultado.datos.entero < 0) ||
                            (resultado.tipo == VALOR_DECIMAL && resultado.datos.decimal < 0.0))
                        {
                            char msg[256];
                            snprintf(msg, sizeof(msg), "La función '%s' retorna ENTERA SIN SIGNO pero se retornó un valor negativo.", nombre);
                            contexto_set_error(ctx, msg);
                            valor_destruir(&resultado);
                            goto limpiar_funcion_usuario;
                        }
                    }
                    else if (func->tipo_retorno == TIPO_DECIMAL_SIN_SIGNO)
                    {
                        if ((resultado.tipo == VALOR_DECIMAL && resultado.datos.decimal < 0.0) ||
                            (resultado.tipo == VALOR_ENTERO && resultado.datos.entero < 0))
                        {
                            char msg[256];
                            snprintf(msg, sizeof(msg), "La función '%s' retorna DECIMAL SIN SIGNO pero se retornó un valor negativo.", nombre);
                            contexto_set_error(ctx, msg);
                            valor_destruir(&resultado);
                            goto limpiar_funcion_usuario;
                        }
                    }

                    if (func->tipo_retorno == TIPO_ENTERO || func->tipo_retorno == TIPO_ENTERO_SIN_SIGNO)
                    {
                        if (resultado.tipo == VALOR_DECIMAL || resultado.tipo == VALOR_DECIMAL_SIN_SIGNO)
                        {
                            char msg[256];
                            snprintf(msg, sizeof(msg), "La función '%s' retorna ENTERA pero se retornó un valor decimal.", nombre);
                            contexto_set_error(ctx, msg);
                            valor_destruir(&resultado);
                            goto limpiar_funcion_usuario;
                        }
                    }
                }

            limpiar_funcion_usuario:
                // 3. RESTAURAR CONTEXTO (Ahora es 100% seguro, las variables están inicializadas)
                ctx->profundidad_llamada--;
                ctx->tabla_actual = tabla_anterior;
                ctx->estado_flujo = estado_anterior;
                ctx->valor_retorno = retorno_anterior;

                // 4. LIMPIEZA SEGURA: Solo liberamos si la tabla fue creada
                if (tabla_local != NULL)
                {
                    Simbolo *sim = tabla_local->primera;
                    while (sim != NULL)
                    {
                        Simbolo *siguiente = sim->siguiente;
                        if (sim->nombre)
                            free((void *)sim->nombre);

                        if (sim->valor.tipo == VALOR_TEXTO || sim->valor.tipo == VALOR_TEXTO_EXTENSO)
                        {
                            if (sim->valor.datos.texto)
                                free((void *)sim->valor.datos.texto);
                        }
                        // Nota: No liberamos LISTA/MATRIZ aquí porque son compartidas (shallow copy)
                        // y el bloque principal es el dueño de esa memoria.

                        free(sim);
                        sim = siguiente;
                    }
                    free(tabla_local);
                }

                return resultado;
            }

            // ========================================================================
            // FUNCIONES BUILT-IN (Se mantiene exactamente igual a tu versión anterior)
            // ========================================================================
            if (num_args == 1)
            {
                double v = 0;
                if (args[0].tipo == VALOR_ENTERO)
                    v = args[0].datos.entero;
                else if (args[0].tipo == VALOR_DECIMAL)
                    v = args[0].datos.decimal;
                else if (args[0].tipo == VALOR_ENTERO_SIN_SIGNO)
                    v = (double)args[0].datos.entero_sin_signo;
                else if (args[0].tipo == VALOR_DECIMAL_SIN_SIGNO)
                    v = args[0].datos.decimal_sin_signo;

                if (strcmp(nombre, "SENO") == 0)
                    return valor_crear_decimal(sin(v));
                if (strcmp(nombre, "COSENO") == 0)
                    return valor_crear_decimal(cos(v));
                if (strcmp(nombre, "TANGENTE") == 0)
                    return valor_crear_decimal(tan(v));
                if (strcmp(nombre, "RAIZ") == 0)
                    return valor_crear_decimal(sqrt(v));
                if (strcmp(nombre, "ABSOLUTO") == 0)
                    return valor_crear_decimal(fabs(v));
                if (strcmp(nombre, "EXPONENCIAL") == 0)
                    return valor_crear_decimal(exp(v));
                if (strcmp(nombre, "LOGNATURAL") == 0)
                    return valor_crear_decimal(log(v));
                if (strcmp(nombre, "LOGBASE10") == 0)
                    return valor_crear_decimal(log10(v));
                if (strcmp(nombre, "LOGBASE2") == 0)
                    return valor_crear_decimal(log2(v));
                if (strcmp(nombre, "RAIZCUBICA") == 0)
                    return valor_crear_decimal(cbrt(v));
                if (strcmp(nombre, "SIGMOIDE") == 0)
                    return valor_crear_decimal(1.0 / (1.0 + exp(-v)));
                if (strcmp(nombre, "REDONDEAR") == 0)
                    return valor_crear_decimal(round(v));
                if (strcmp(nombre, "QUITARDECIMAL") == 0)
                    return valor_crear_decimal(trunc(v));
                if (strcmp(nombre, "ARCOSENO") == 0)
                    return valor_crear_decimal(asin(v));
                if (strcmp(nombre, "ARCOCOSENO") == 0)
                    return valor_crear_decimal(acos(v));
                if (strcmp(nombre, "ARCOTANGENTE") == 0)
                    return valor_crear_decimal(atan(v));
                if (strcmp(nombre, "DOSALAX") == 0)
                    return valor_crear_decimal(pow(2.0, v));
                if (strcmp(nombre, "BITNO") == 0)
                {
                    unsigned long long val = 0;
                    if (args[0].tipo == VALOR_ENTERO)
                        val = (unsigned long long)args[0].datos.entero;
                    else if (args[0].tipo == VALOR_ENTERO_SIN_SIGNO)
                        val = args[0].datos.entero_sin_signo;
                    else if (args[0].tipo == VALOR_DECIMAL)
                        val = (unsigned long long)args[0].datos.decimal;
                    else if (args[0].tipo == VALOR_DECIMAL_SIN_SIGNO)
                        val = (unsigned long long)args[0].datos.decimal_sin_signo;
                    return valor_crear_entero_sin_signo(~val);
                }
            }

            if (num_args == 2)
            {
                double v1 = 0, v2 = 0;
                if (args[0].tipo == VALOR_ENTERO)
                    v1 = args[0].datos.entero;
                else if (args[0].tipo == VALOR_DECIMAL)
                    v1 = args[0].datos.decimal;
                else if (args[0].tipo == VALOR_ENTERO_SIN_SIGNO)
                    v1 = (double)args[0].datos.entero_sin_signo;
                else if (args[0].tipo == VALOR_DECIMAL_SIN_SIGNO)
                    v1 = args[0].datos.decimal_sin_signo;

                if (args[1].tipo == VALOR_ENTERO)
                    v2 = args[1].datos.entero;
                else if (args[1].tipo == VALOR_DECIMAL)
                    v2 = args[1].datos.decimal;
                else if (args[1].tipo == VALOR_ENTERO_SIN_SIGNO)
                    v2 = (double)args[1].datos.entero_sin_signo;
                else if (args[1].tipo == VALOR_DECIMAL_SIN_SIGNO)
                    v2 = args[1].datos.decimal_sin_signo;

                if (strcmp(nombre, "POTENCIA") == 0)
                    return valor_crear_decimal(pow(v1, v2));
                if (strcmp(nombre, "MODULO") == 0)
                    return valor_crear_decimal(fmod(v1, v2));
                if (strcmp(nombre, "MAXIMO") == 0)
                    return valor_crear_decimal(v1 > v2 ? v1 : v2);
                if (strcmp(nombre, "MINIMO") == 0)
                    return valor_crear_decimal(v1 < v2 ? v1 : v2);
                if (strcmp(nombre, "LOGARITMO") == 0)
                    return valor_crear_decimal(log(v1) / log(v2));
                if (strcmp(nombre, "DOSALAX") == 0)
                    return valor_crear_decimal(pow(2.0, v2));

                if (strcmp(nombre, "REDONDEAR") == 0)
                {
                    if (args[0].tipo == VALOR_TEXTO)
                    {
                        double v = 0;
                        if (args[1].tipo == VALOR_ENTERO)
                            v = args[1].datos.entero;
                        else if (args[1].tipo == VALOR_DECIMAL)
                            v = args[1].datos.decimal;

                        if (strcmp(args[0].datos.texto, "ARRIBA") == 0)
                            return valor_crear_decimal(ceil(v));
                        else if (strcmp(args[0].datos.texto, "ABAJO") == 0)
                            return valor_crear_decimal(floor(v));
                        else if (strcmp(args[0].datos.texto, "ENTERO") == 0)
                            return valor_crear_decimal(round(v));
                    }
                    contexto_set_error(ctx, "REDONDEAR requiere modo (ARRIBA, ABAJO, ENTERO) y número");
                    return valor_crear_vacio();
                }

                if (strcmp(nombre, "LEERBIT") == 0)
                {
                    unsigned long long val = 0;
                    int pos = 0;
                    if (args[0].tipo == VALOR_ENTERO)
                        val = (unsigned long long)args[0].datos.entero;
                    else if (args[0].tipo == VALOR_ENTERO_SIN_SIGNO)
                        val = args[0].datos.entero_sin_signo;
                    else if (args[0].tipo == VALOR_DECIMAL)
                        val = (unsigned long long)args[0].datos.decimal;
                    else if (args[0].tipo == VALOR_DECIMAL_SIN_SIGNO)
                        val = (unsigned long long)args[0].datos.decimal_sin_signo;

                    if (args[1].tipo == VALOR_ENTERO)
                        pos = args[1].datos.entero;
                    else if (args[1].tipo == VALOR_ENTERO_SIN_SIGNO)
                        pos = (int)args[1].datos.entero_sin_signo;
                    else if (args[1].tipo == VALOR_DECIMAL)
                        pos = (int)args[1].datos.decimal;
                    else if (args[1].tipo == VALOR_DECIMAL_SIN_SIGNO)
                        pos = (int)args[1].datos.decimal_sin_signo;

                    return valor_crear_entero((int)((val >> pos) & 1));
                }

                if (strcmp(nombre, "ACTIVARBIT") == 0)
                {
                    unsigned long long val = 0, pos = 0;
                    if (args[0].tipo == VALOR_ENTERO)
                        val = (unsigned long long)args[0].datos.entero;
                    else if (args[0].tipo == VALOR_ENTERO_SIN_SIGNO)
                        val = args[0].datos.entero_sin_signo;
                    else if (args[0].tipo == VALOR_DECIMAL)
                        val = (unsigned long long)args[0].datos.decimal;
                    else if (args[0].tipo == VALOR_DECIMAL_SIN_SIGNO)
                        val = (unsigned long long)args[0].datos.decimal_sin_signo;

                    if (args[1].tipo == VALOR_ENTERO)
                        pos = (unsigned long long)args[1].datos.entero;
                    else if (args[1].tipo == VALOR_ENTERO_SIN_SIGNO)
                        pos = args[1].datos.entero_sin_signo;
                    else if (args[1].tipo == VALOR_DECIMAL)
                        pos = (unsigned long long)args[1].datos.decimal;
                    else if (args[1].tipo == VALOR_DECIMAL_SIN_SIGNO)
                        pos = args[1].datos.decimal_sin_signo;

                    return valor_crear_entero_sin_signo(val | (1ULL << pos));
                }

                if (strcmp(nombre, "DESACTIVARBIT") == 0)
                {
                    unsigned long long val = 0, pos = 0;
                    if (args[0].tipo == VALOR_ENTERO)
                        val = (unsigned long long)args[0].datos.entero;
                    else if (args[0].tipo == VALOR_ENTERO_SIN_SIGNO)
                        val = args[0].datos.entero_sin_signo;
                    else if (args[0].tipo == VALOR_DECIMAL)
                        val = (unsigned long long)args[0].datos.decimal;
                    else if (args[0].tipo == VALOR_DECIMAL_SIN_SIGNO)
                        val = (unsigned long long)args[0].datos.decimal_sin_signo;

                    if (args[1].tipo == VALOR_ENTERO)
                        pos = (unsigned long long)args[1].datos.entero;
                    else if (args[1].tipo == VALOR_ENTERO_SIN_SIGNO)
                        pos = args[1].datos.entero_sin_signo;
                    else if (args[1].tipo == VALOR_DECIMAL)
                        pos = (unsigned long long)args[1].datos.decimal;
                    else if (args[1].tipo == VALOR_DECIMAL_SIN_SIGNO)
                        pos = args[1].datos.decimal_sin_signo;

                    return valor_crear_entero_sin_signo(val & ~(1ULL << pos));
                }

                if (strcmp(nombre, "ROTARIZQUIERDA") == 0)
                {
                    unsigned long long val = 0, shift = 0;
                    if (args[0].tipo == VALOR_ENTERO)
                        val = (unsigned long long)args[0].datos.entero;
                    else if (args[0].tipo == VALOR_ENTERO_SIN_SIGNO)
                        val = args[0].datos.entero_sin_signo;
                    else if (args[0].tipo == VALOR_DECIMAL)
                        val = (unsigned long long)args[0].datos.decimal;
                    else if (args[0].tipo == VALOR_DECIMAL_SIN_SIGNO)
                        val = (unsigned long long)args[0].datos.decimal_sin_signo;

                    if (args[1].tipo == VALOR_ENTERO)
                        shift = (unsigned long long)args[1].datos.entero;
                    else if (args[1].tipo == VALOR_ENTERO_SIN_SIGNO)
                        shift = args[1].datos.entero_sin_signo;
                    else if (args[1].tipo == VALOR_DECIMAL)
                        shift = (unsigned long long)args[1].datos.decimal;
                    else if (args[1].tipo == VALOR_DECIMAL_SIN_SIGNO)
                        shift = args[1].datos.decimal_sin_signo;

                    shift = shift % 64;
                    return valor_crear_entero_sin_signo((val << shift) | (val >> (64 - shift)));
                }

                if (strcmp(nombre, "ROTARDERECHA") == 0)
                {
                    unsigned long long val = 0, shift = 0;
                    if (args[0].tipo == VALOR_ENTERO)
                        val = (unsigned long long)args[0].datos.entero;
                    else if (args[0].tipo == VALOR_ENTERO_SIN_SIGNO)
                        val = args[0].datos.entero_sin_signo;
                    else if (args[0].tipo == VALOR_DECIMAL)
                        val = (unsigned long long)args[0].datos.decimal;
                    else if (args[0].tipo == VALOR_DECIMAL_SIN_SIGNO)
                        val = (unsigned long long)args[0].datos.decimal_sin_signo;

                    if (args[1].tipo == VALOR_ENTERO)
                        shift = (unsigned long long)args[1].datos.entero;
                    else if (args[1].tipo == VALOR_ENTERO_SIN_SIGNO)
                        shift = args[1].datos.entero_sin_signo;
                    else if (args[1].tipo == VALOR_DECIMAL)
                        shift = (unsigned long long)args[1].datos.decimal;
                    else if (args[1].tipo == VALOR_DECIMAL_SIN_SIGNO)
                        shift = args[1].datos.decimal_sin_signo;

                    shift = shift % 64;
                    return valor_crear_entero_sin_signo((val >> shift) | (val << (64 - shift)));
                }

                if (strcmp(nombre, "DESPLAZARIZQUIERDA") == 0)
                {
                    unsigned long long val = 0, shift = 0;
                    if (args[0].tipo == VALOR_ENTERO)
                        val = (unsigned long long)args[0].datos.entero;
                    else if (args[0].tipo == VALOR_ENTERO_SIN_SIGNO)
                        val = args[0].datos.entero_sin_signo;
                    else if (args[0].tipo == VALOR_DECIMAL)
                        val = (unsigned long long)args[0].datos.decimal;
                    else if (args[0].tipo == VALOR_DECIMAL_SIN_SIGNO)
                        val = (unsigned long long)args[0].datos.decimal_sin_signo;

                    if (args[1].tipo == VALOR_ENTERO)
                        shift = (unsigned long long)args[1].datos.entero;
                    else if (args[1].tipo == VALOR_ENTERO_SIN_SIGNO)
                        shift = args[1].datos.entero_sin_signo;
                    else if (args[1].tipo == VALOR_DECIMAL)
                        shift = (unsigned long long)args[1].datos.decimal;
                    else if (args[1].tipo == VALOR_DECIMAL_SIN_SIGNO)
                        shift = args[1].datos.decimal_sin_signo;

                    return valor_crear_entero_sin_signo(val << shift);
                }

                if (strcmp(nombre, "DESPLAZARDERECHA") == 0)
                {
                    unsigned long long val = 0, shift = 0;
                    if (args[0].tipo == VALOR_ENTERO)
                        val = (unsigned long long)args[0].datos.entero;
                    else if (args[0].tipo == VALOR_ENTERO_SIN_SIGNO)
                        val = args[0].datos.entero_sin_signo;
                    else if (args[0].tipo == VALOR_DECIMAL)
                        val = (unsigned long long)args[0].datos.decimal;
                    else if (args[0].tipo == VALOR_DECIMAL_SIN_SIGNO)
                        val = (unsigned long long)args[0].datos.decimal_sin_signo;

                    if (args[1].tipo == VALOR_ENTERO)
                        shift = (unsigned long long)args[1].datos.entero;
                    else if (args[1].tipo == VALOR_ENTERO_SIN_SIGNO)
                        shift = args[1].datos.entero_sin_signo;
                    else if (args[1].tipo == VALOR_DECIMAL)
                        shift = (unsigned long long)args[1].datos.decimal;
                    else if (args[1].tipo == VALOR_DECIMAL_SIN_SIGNO)
                        shift = args[1].datos.decimal_sin_signo;

                    return valor_crear_entero_sin_signo(val >> shift);
                }

                if (strcmp(nombre, "BITY") == 0)
                {
                    unsigned long long val1 = 0, val2 = 0;
                    if (args[0].tipo == VALOR_ENTERO)
                        val1 = (unsigned long long)args[0].datos.entero;
                    else if (args[0].tipo == VALOR_ENTERO_SIN_SIGNO)
                        val1 = args[0].datos.entero_sin_signo;
                    else if (args[0].tipo == VALOR_DECIMAL)
                        val1 = (unsigned long long)args[0].datos.decimal;
                    else if (args[0].tipo == VALOR_DECIMAL_SIN_SIGNO)
                        val1 = (unsigned long long)args[0].datos.decimal_sin_signo;

                    if (args[1].tipo == VALOR_ENTERO)
                        val2 = (unsigned long long)args[1].datos.entero;
                    else if (args[1].tipo == VALOR_ENTERO_SIN_SIGNO)
                        val2 = args[1].datos.entero_sin_signo;
                    else if (args[1].tipo == VALOR_DECIMAL)
                        val2 = (unsigned long long)args[1].datos.decimal;
                    else if (args[1].tipo == VALOR_DECIMAL_SIN_SIGNO)
                        val2 = (unsigned long long)args[1].datos.decimal_sin_signo;

                    return valor_crear_entero_sin_signo(val1 & val2);
                }

                if (strcmp(nombre, "BITO") == 0)
                {
                    unsigned long long val1 = 0, val2 = 0;
                    if (args[0].tipo == VALOR_ENTERO)
                        val1 = (unsigned long long)args[0].datos.entero;
                    else if (args[0].tipo == VALOR_ENTERO_SIN_SIGNO)
                        val1 = args[0].datos.entero_sin_signo;
                    else if (args[0].tipo == VALOR_DECIMAL)
                        val1 = (unsigned long long)args[0].datos.decimal;
                    else if (args[0].tipo == VALOR_DECIMAL_SIN_SIGNO)
                        val1 = (unsigned long long)args[0].datos.decimal_sin_signo;

                    if (args[1].tipo == VALOR_ENTERO)
                        val2 = (unsigned long long)args[1].datos.entero;
                    else if (args[1].tipo == VALOR_ENTERO_SIN_SIGNO)
                        val2 = args[1].datos.entero_sin_signo;
                    else if (args[1].tipo == VALOR_DECIMAL)
                        val2 = (unsigned long long)args[1].datos.decimal;
                    else if (args[1].tipo == VALOR_DECIMAL_SIN_SIGNO)
                        val2 = (unsigned long long)args[1].datos.decimal_sin_signo;

                    return valor_crear_entero_sin_signo(val1 | val2);
                }

                if (strcmp(nombre, "BITXOR") == 0)
                {
                    unsigned long long val1 = 0, val2 = 0;
                    if (args[0].tipo == VALOR_ENTERO)
                        val1 = (unsigned long long)args[0].datos.entero;
                    else if (args[0].tipo == VALOR_ENTERO_SIN_SIGNO)
                        val1 = args[0].datos.entero_sin_signo;
                    else if (args[0].tipo == VALOR_DECIMAL)
                        val1 = (unsigned long long)args[0].datos.decimal;
                    else if (args[0].tipo == VALOR_DECIMAL_SIN_SIGNO)
                        val1 = (unsigned long long)args[0].datos.decimal_sin_signo;

                    if (args[1].tipo == VALOR_ENTERO)
                        val2 = (unsigned long long)args[1].datos.entero;
                    else if (args[1].tipo == VALOR_ENTERO_SIN_SIGNO)
                        val2 = args[1].datos.entero_sin_signo;
                    else if (args[1].tipo == VALOR_DECIMAL)
                        val2 = (unsigned long long)args[1].datos.decimal;
                    else if (args[1].tipo == VALOR_DECIMAL_SIN_SIGNO)
                        val2 = (unsigned long long)args[1].datos.decimal_sin_signo;

                    return valor_crear_entero_sin_signo(val1 ^ val2);
                }
            }

            if (num_args == 0)
            {
                if (strcmp(nombre, "NUMEROPI") == 0)
                    return valor_crear_decimal(3.14159265358979);
                if (strcmp(nombre, "NUMEROEULER") == 0)
                    return valor_crear_decimal(2.71828182845905);
                if (strcmp(nombre, "RAIZDEUNMEDIO") == 0)
                    return valor_crear_decimal(0.70710678118655);
                if (strcmp(nombre, "LOGNATURALDE2") == 0)
                    return valor_crear_decimal(0.6931471805599453);
                if (strcmp(nombre, "LOGNATURALDE10") == 0)
                    return valor_crear_decimal(2.302585092994046);
            }

            if (num_args == 1)
            {
                if (strcmp(nombre, "LONGITUDTEXTO") == 0)
                {
                    if (args[0].tipo != VALOR_TEXTO && args[0].tipo != VALOR_TEXTO_EXTENSO)
                    {
                        contexto_set_error(ctx, "LONGITUDTEXTO requiere un texto");
                        return valor_crear_vacio();
                    }
                    return valor_crear_entero(strlen(args[0].datos.texto ? args[0].datos.texto : ""));
                }
                if (strcmp(nombre, "LONGITUDLISTA") == 0)
                {
                    if (args[0].tipo != VALOR_LISTA)
                    {
                        contexto_set_error(ctx, "LONGITUDLISTA requiere una lista");
                        return valor_crear_vacio();
                    }
                    return valor_crear_entero(args[0].tamano);
                }
                if (strcmp(nombre, "TEXTOVACIO") == 0)
                {
                    if (args[0].tipo != VALOR_TEXTO && args[0].tipo != VALOR_TEXTO_EXTENSO)
                    {
                        contexto_set_error(ctx, "TEXTOVACIO requiere un texto");
                        return valor_crear_vacio();
                    }
                    return valor_crear_entero(strlen(args[0].datos.texto ? args[0].datos.texto : "") == 0 ? 1 : 0);
                }
                if (strcmp(nombre, "TEXTOAENTERO") == 0)
                {
                    if (args[0].tipo != VALOR_TEXTO && args[0].tipo != VALOR_TEXTO_EXTENSO)
                    {
                        contexto_set_error(ctx, "TEXTOAENTERO requiere un texto");
                        return valor_crear_vacio();
                    }
                    return valor_crear_entero(atoll(args[0].datos.texto ? args[0].datos.texto : "0"));
                }
                if (strcmp(nombre, "TEXTOADECIMAL") == 0)
                {
                    if (args[0].tipo != VALOR_TEXTO && args[0].tipo != VALOR_TEXTO_EXTENSO)
                    {
                        contexto_set_error(ctx, "TEXTOADECIMAL requiere un texto");
                        return valor_crear_vacio();
                    }
                    return valor_crear_decimal(atof(args[0].datos.texto ? args[0].datos.texto : "0"));
                }
                if (strcmp(nombre, "TEXTOACARACTER") == 0)
                {
                    if (args[0].tipo != VALOR_TEXTO && args[0].tipo != VALOR_TEXTO_EXTENSO)
                    {
                        contexto_set_error(ctx, "TEXTOACARACTER requiere un texto");
                        return valor_crear_vacio();
                    }
                    return valor_crear_entero(args[0].datos.texto ? (int)args[0].datos.texto[0] : 0);
                }
                if (strcmp(nombre, "MAYUSCULAS") == 0)
                {
                    if (args[0].tipo != VALOR_TEXTO && args[0].tipo != VALOR_TEXTO_EXTENSO)
                    {
                        contexto_set_error(ctx, "MAYUSCULAS requiere un texto");
                        return valor_crear_vacio();
                    }
                    char *texto = args[0].datos.texto ? args[0].datos.texto : "";
                    size_t len = strlen(texto);
                    char *resultado = malloc(len + 1);
                    for (size_t i = 0; i < len; i++)
                        resultado[i] = toupper((unsigned char)texto[i]);
                    resultado[len] = '\0';
                    return valor_crear_texto(resultado);
                }
                if (strcmp(nombre, "MINUSCULAS") == 0)
                {
                    if (args[0].tipo != VALOR_TEXTO && args[0].tipo != VALOR_TEXTO_EXTENSO)
                    {
                        contexto_set_error(ctx, "MINUSCULAS requiere un texto");
                        return valor_crear_vacio();
                    }
                    char *texto = args[0].datos.texto ? args[0].datos.texto : "";
                    size_t len = strlen(texto);
                    char *resultado = malloc(len + 1);
                    for (size_t i = 0; i < len; i++)
                        resultado[i] = tolower((unsigned char)texto[i]);
                    resultado[len] = '\0';
                    return valor_crear_texto(resultado);
                }
                if (strcmp(nombre, "RECORTARTEXTO") == 0)
                {
                    if (args[0].tipo != VALOR_TEXTO && args[0].tipo != VALOR_TEXTO_EXTENSO)
                    {
                        contexto_set_error(ctx, "RECORTARTEXTO requiere un texto");
                        return valor_crear_vacio();
                    }
                    char *texto = args[0].datos.texto ? args[0].datos.texto : "";
                    size_t len = strlen(texto);
                    size_t inicio = 0, fin = len;
                    while (inicio < len && isspace((unsigned char)texto[inicio]))
                        inicio++;
                    while (fin > inicio && isspace((unsigned char)texto[fin - 1]))
                        fin--;
                    size_t nueva_len = fin - inicio;
                    char *resultado = malloc(nueva_len + 1);
                    strncpy(resultado, texto + inicio, nueva_len);
                    resultado[nueva_len] = '\0';
                    return valor_crear_texto(resultado);
                }
            }

            if (num_args == 2)
            {
                if (strcmp(nombre, "BUSCARTEXTO") == 0)
                {
                    if (args[0].tipo != VALOR_TEXTO && args[0].tipo != VALOR_TEXTO_EXTENSO)
                    {
                        contexto_set_error(ctx, "BUSCARTEXTO requiere texto como primer argumento");
                        return valor_crear_vacio();
                    }
                    if (args[1].tipo != VALOR_TEXTO && args[1].tipo != VALOR_TEXTO_EXTENSO)
                    {
                        contexto_set_error(ctx, "BUSCARTEXTO requiere texto como segundo argumento");
                        return valor_crear_vacio();
                    }
                    char *haystack = args[0].datos.texto ? args[0].datos.texto : "";
                    char *needle = args[1].datos.texto ? args[1].datos.texto : "";
                    char *encontrado = strstr(haystack, needle);
                    return valor_crear_entero(encontrado ? (encontrado - haystack) : -1);
                }
                if (strcmp(nombre, "BUSCARCARACTER") == 0)
                {
                    if (args[0].tipo != VALOR_TEXTO && args[0].tipo != VALOR_TEXTO_EXTENSO)
                    {
                        contexto_set_error(ctx, "BUSCARCARACTER requiere texto como primer argumento");
                        return valor_crear_vacio();
                    }
                    char *texto = args[0].datos.texto ? args[0].datos.texto : "";
                    char c = '\0';
                    if (args[1].tipo == VALOR_CARACTER && args[1].datos.texto)
                        c = args[1].datos.texto[0];
                    else if (args[1].tipo == VALOR_TEXTO && args[1].datos.texto)
                        c = args[1].datos.texto[0];
                    else
                    {
                        contexto_set_error(ctx, "BUSCARCARACTER requiere carácter como segundo argumento");
                        return valor_crear_vacio();
                    }
                    char *encontrado = strchr(texto, c);
                    return valor_crear_entero(encontrado ? (encontrado - texto) : -1);
                }
                if (strcmp(nombre, "COMPARARTEXTO") == 0)
                {
                    if (args[0].tipo != VALOR_TEXTO && args[0].tipo != VALOR_TEXTO_EXTENSO)
                    {
                        contexto_set_error(ctx, "COMPARARTEXTO requiere texto como primer argumento");
                        return valor_crear_vacio();
                    }
                    if (args[1].tipo != VALOR_TEXTO && args[1].tipo != VALOR_TEXTO_EXTENSO)
                    {
                        contexto_set_error(ctx, "COMPARARTEXTO requiere texto como segundo argumento");
                        return valor_crear_vacio();
                    }
                    return valor_crear_entero(strcmp(args[0].datos.texto ? args[0].datos.texto : "", args[1].datos.texto ? args[1].datos.texto : ""));
                }
            }

            if (num_args == 3)
            {
                if (strcmp(nombre, "REPETIRTEXTO") == 0)
                {
                    if (args[0].tipo != VALOR_TEXTO && args[0].tipo != VALOR_TEXTO_EXTENSO)
                    {
                        contexto_set_error(ctx, "REPETIRTEXTO requiere texto como primer argumento");
                        return valor_crear_vacio();
                    }
                    int veces = 0;
                    if (args[1].tipo == VALOR_ENTERO)
                        veces = (int)args[1].datos.entero;
                    else if (args[1].tipo == VALOR_DECIMAL)
                        veces = (int)args[1].datos.decimal;
                    else
                    {
                        contexto_set_error(ctx, "REPETIRTEXTO requiere número como segundo argumento");
                        return valor_crear_vacio();
                    }

                    char *texto = args[0].datos.texto ? args[0].datos.texto : "";
                    size_t len = strlen(texto);
                    size_t total_len = len * veces;
                    char *resultado = malloc(total_len + 1);
                    resultado[0] = '\0';
                    for (int i = 0; i < veces; i++)
                        strcat(resultado, texto);
                    return valor_crear_texto(resultado);
                }
                if (strcmp(nombre, "EXTRAERTEXTO") == 0)
                {
                    if (args[0].tipo != VALOR_TEXTO && args[0].tipo != VALOR_TEXTO_EXTENSO)
                    {
                        contexto_set_error(ctx, "EXTRAERTEXTO requiere texto como primer argumento");
                        return valor_crear_vacio();
                    }
                    int inicio = 0, longitud = 0;
                    if (args[1].tipo == VALOR_ENTERO)
                        inicio = (int)args[1].datos.entero;
                    else if (args[1].tipo == VALOR_DECIMAL)
                        inicio = (int)args[1].datos.decimal;
                    if (args[2].tipo == VALOR_ENTERO)
                        longitud = (int)args[2].datos.entero;
                    else if (args[2].tipo == VALOR_DECIMAL)
                        longitud = (int)args[2].datos.decimal;

                    char *texto = args[0].datos.texto ? args[0].datos.texto : "";
                    size_t len = strlen(texto);
                    if (inicio < 0)
                        inicio = 0;
                    if (inicio >= (int)len)
                        return valor_crear_texto("");
                    if (inicio + longitud > (int)len)
                        longitud = (int)len - inicio;

                    char *resultado = malloc(longitud + 1);
                    strncpy(resultado, texto + inicio, longitud);
                    resultado[longitud] = '\0';
                    return valor_crear_texto(resultado);
                }
                if (strcmp(nombre, "ALEATORIO") == 0)
                {
                    char *tipo = (args[0].tipo == VALOR_TEXTO || args[0].tipo == VALOR_TEXTO_EXTENSO) ? args[0].datos.texto : NULL;
                    if (!tipo)
                    {
                        contexto_set_error(ctx, "ALEATORIO: primer argumento debe ser texto");
                        return valor_crear_vacio();
                    }

                    if (strcmp(tipo, "ENTERO") == 0)
                    {
                        long long min = (args[1].tipo == VALOR_ENTERO) ? args[1].datos.entero : (long long)args[1].datos.decimal;
                        long long max = (args[2].tipo == VALOR_ENTERO) ? args[2].datos.entero : (long long)args[2].datos.decimal;
                        if (min > max)
                        {
                            long long temp = min;
                            min = max;
                            max = temp;
                        }
                        return valor_crear_entero(min + (rand() % (max - min + 1)));
                    }
                    else if (strcmp(tipo, "DECIMAL") == 0)
                    {
                        double min = (args[1].tipo == VALOR_DECIMAL) ? args[1].datos.decimal : (double)args[1].datos.entero;
                        double max = (args[2].tipo == VALOR_DECIMAL) ? args[2].datos.decimal : (double)args[2].datos.entero;
                        if (min > max)
                        {
                            double temp = min;
                            min = max;
                            max = temp;
                        }
                        return valor_crear_decimal(min + ((double)rand() / RAND_MAX) * (max - min));
                    }
                    contexto_set_error(ctx, "ALEATORIO: tipo debe ser ENTERO o DECIMAL");
                    return valor_crear_vacio();
                }
                if (strcmp(nombre, "ALEATORIOSINSIGNO") == 0)
                {
                    char *tipo = (args[0].tipo == VALOR_TEXTO || args[0].tipo == VALOR_TEXTO_EXTENSO) ? args[0].datos.texto : NULL;
                    if (!tipo)
                    {
                        contexto_set_error(ctx, "ALEATORIOSINSIGNO: primer argumento debe ser texto");
                        return valor_crear_vacio();
                    }

                    if (strcmp(tipo, "ENTERO") == 0)
                    {
                        unsigned long long min = (args[1].tipo == VALOR_ENTERO) ? (unsigned long long)args[1].datos.entero : (unsigned long long)args[1].datos.decimal;
                        unsigned long long max = (args[2].tipo == VALOR_ENTERO) ? (unsigned long long)args[2].datos.entero : (unsigned long long)args[2].datos.decimal;
                        if (min > max)
                        {
                            unsigned long long temp = min;
                            min = max;
                            max = temp;
                        }
                        return valor_crear_entero((long long)(min + (rand() % (max - min + 1))));
                    }
                    else if (strcmp(tipo, "DECIMAL") == 0)
                    {
                        double min = (args[1].tipo == VALOR_DECIMAL) ? args[1].datos.decimal : (double)args[1].datos.entero;
                        double max = (args[2].tipo == VALOR_DECIMAL) ? args[2].datos.decimal : (double)args[2].datos.entero;
                        if (min > max)
                        {
                            double temp = min;
                            min = max;
                            max = temp;
                        }
                        return valor_crear_decimal(min + ((double)rand() / RAND_MAX) * (max - min));
                    }
                    contexto_set_error(ctx, "ALEATORIOSINSIGNO: tipo debe ser ENTERO o DECIMAL");
                    return valor_crear_vacio();
                }
            }

            if (strcmp(nombre, "CONECTARBD") == 0)
            {
                if (num_args != 1 || args[0].tipo != VALOR_TEXTO)
                {
                    contexto_set_error(ctx, "CONECTARBD requiere un argumento de tipo texto");
                    return valor_crear_vacio();
                }
                sqlite3 *db = NULL;
                int rc = sqlite3_open(args[0].datos.texto, &db);
                if (rc != SQLITE_OK)
                {
                    contexto_set_error(ctx, sqlite3_errmsg(db));
                    sqlite3_close(db);
                    return valor_crear_vacio();
                }
                ctx->sqlite_db = db;
                return valor_crear_entero(1);
            }

            if (strcmp(nombre, "EJECUTARBD") == 0)
            {
                if (num_args < 1 || args[0].tipo != VALOR_TEXTO)
                {
                    contexto_set_error(ctx, "EJECUTARBD requiere al menos un argumento de tipo texto");
                    return valor_crear_vacio();
                }
                if (!ctx->sqlite_db)
                {
                    contexto_set_error(ctx, "No hay conexión activa a base de datos");
                    return valor_crear_vacio();
                }

                if (num_args == 1)
                {
                    char *err_msg = NULL;
                    int rc = sqlite3_exec((sqlite3 *)ctx->sqlite_db, args[0].datos.texto, NULL, NULL, &err_msg);
                    if (rc != SQLITE_OK)
                    {
                        contexto_set_error(ctx, err_msg);
                        sqlite3_free(err_msg);
                        return valor_crear_vacio();
                    }
                    return valor_crear_entero(1);
                }
                else
                {
                    sqlite3_stmt *stmt = NULL;
                    int rc = sqlite3_prepare_v2((sqlite3 *)ctx->sqlite_db, args[0].datos.texto, -1, &stmt, NULL);
                    if (rc != SQLITE_OK)
                    {
                        contexto_set_error(ctx, sqlite3_errmsg((sqlite3 *)ctx->sqlite_db));
                        return valor_crear_vacio();
                    }
                    for (int i = 1; i < num_args; i++)
                    {
                        if (args[i].tipo == VALOR_TEXTO || args[i].tipo == VALOR_TEXTO_EXTENSO)
                            sqlite3_bind_text(stmt, i, args[i].datos.texto, -1, SQLITE_TRANSIENT);
                        else if (args[i].tipo == VALOR_ENTERO)
                            sqlite3_bind_int64(stmt, i, args[i].datos.entero);
                        else if (args[i].tipo == VALOR_DECIMAL)
                            sqlite3_bind_double(stmt, i, args[i].datos.decimal);
                        else if (args[i].tipo == VALOR_LOGICA)
                            sqlite3_bind_int(stmt, i, args[i].datos.logica ? 1 : 0);
                        else
                            sqlite3_bind_null(stmt, i);
                    }
                    rc = sqlite3_step(stmt);
                    sqlite3_finalize(stmt);
                    if (rc != SQLITE_DONE)
                    {
                        contexto_set_error(ctx, sqlite3_errmsg((sqlite3 *)ctx->sqlite_db));
                        return valor_crear_vacio();
                    }
                    return valor_crear_entero(1);
                }
            }

            if (strcmp(nombre, "CONSULTARBD") == 0)
            {
                if (num_args < 1 || args[0].tipo != VALOR_TEXTO)
                {
                    contexto_set_error(ctx, "CONSULTARBD requiere al menos un argumento de tipo texto");
                    return valor_crear_vacio();
                }
                if (!ctx->sqlite_db)
                {
                    contexto_set_error(ctx, "No hay conexión activa a base de datos");
                    return valor_crear_vacio();
                }
                if (ctx->sqlite_stmt)
                {
                    sqlite3_finalize((sqlite3_stmt *)ctx->sqlite_stmt);
                    ctx->sqlite_stmt = NULL;
                }

                sqlite3_stmt *stmt = NULL;
                int rc = sqlite3_prepare_v2((sqlite3 *)ctx->sqlite_db, args[0].datos.texto, -1, &stmt, NULL);
                if (rc != SQLITE_OK)
                {
                    contexto_set_error(ctx, sqlite3_errmsg((sqlite3 *)ctx->sqlite_db));
                    return valor_crear_vacio();
                }

                if (num_args > 1)
                {
                    for (int i = 1; i < num_args; i++)
                    {
                        if (args[i].tipo == VALOR_TEXTO || args[i].tipo == VALOR_TEXTO_EXTENSO)
                            sqlite3_bind_text(stmt, i, args[i].datos.texto, -1, SQLITE_TRANSIENT);
                        else if (args[i].tipo == VALOR_ENTERO)
                            sqlite3_bind_int64(stmt, i, args[i].datos.entero);
                        else if (args[i].tipo == VALOR_DECIMAL)
                            sqlite3_bind_double(stmt, i, args[i].datos.decimal);
                        else if (args[i].tipo == VALOR_LOGICA)
                            sqlite3_bind_int(stmt, i, args[i].datos.logica ? 1 : 0);
                        else
                            sqlite3_bind_null(stmt, i);
                    }
                }
                ctx->sqlite_stmt = stmt;
                ctx->sqlite_columnas = sqlite3_column_count(stmt);
                if (ctx->sqlite_columnas_nombre)
                    free(ctx->sqlite_columnas_nombre);
                ctx->sqlite_columnas_nombre = malloc(sizeof(char *) * ctx->sqlite_columnas);
                for (int i = 0; i < ctx->sqlite_columnas; i++)
                    ctx->sqlite_columnas_nombre[i] = strdup(sqlite3_column_name(stmt, i));
                return valor_crear_entero(1);
            }

            if (strcmp(nombre, "CERRARCONSULTABD") == 0)
            {
                if (ctx->sqlite_stmt)
                {
                    sqlite3_finalize((sqlite3_stmt *)ctx->sqlite_stmt);
                    ctx->sqlite_stmt = NULL;
                    ctx->sqlite_columnas = 0;
                    if (ctx->sqlite_columnas_nombre)
                    {
                        free(ctx->sqlite_columnas_nombre);
                        ctx->sqlite_columnas_nombre = NULL;
                    }
                }
                return valor_crear_entero(1);
            }

            if (strcmp(nombre, "CERRARBD") == 0)
            {
                if (ctx->sqlite_stmt)
                {
                    sqlite3_finalize((sqlite3_stmt *)ctx->sqlite_stmt);
                    ctx->sqlite_stmt = NULL;
                }
                if (ctx->sqlite_db)
                {
                    sqlite3_close((sqlite3 *)ctx->sqlite_db);
                    ctx->sqlite_db = NULL;
                    ctx->sqlite_columnas = 0;
                    if (ctx->sqlite_columnas_nombre)
                    {
                        free(ctx->sqlite_columnas_nombre);
                        ctx->sqlite_columnas_nombre = NULL;
                    }
                }
                return valor_crear_entero(1);
            }

            if (strcmp(nombre, "SIGUIENTEFILABD") == 0)
            {
                if (!ctx->sqlite_stmt)
                    return valor_crear_logica(false);
                int rc = sqlite3_step((sqlite3_stmt *)ctx->sqlite_stmt);
                if (rc == SQLITE_ROW)
                {
                    for (int i = 0; i < ctx->sqlite_columnas; i++)
                    {
                        char var_name[32];
                        snprintf(var_name, sizeof(var_name), "BDCOL%d", i + 1);
                        int tipo = sqlite3_column_type((sqlite3_stmt *)ctx->sqlite_stmt, i);
                        Valor valor;
                        if (tipo == SQLITE_INTEGER)
                            valor = valor_crear_entero(sqlite3_column_int64((sqlite3_stmt *)ctx->sqlite_stmt, i));
                        else if (tipo == SQLITE_FLOAT)
                            valor = valor_crear_decimal(sqlite3_column_double((sqlite3_stmt *)ctx->sqlite_stmt, i));
                        else if (tipo == SQLITE_TEXT)
                            valor = valor_crear_texto(strdup((const char *)sqlite3_column_text((sqlite3_stmt *)ctx->sqlite_stmt, i)));
                        else
                            valor = valor_crear_texto(strdup(""));
                        tabla_simbolos_definir(ctx->tabla_actual, var_name, valor, false);
                    }
                    return valor_crear_logica(true);
                }
                return valor_crear_logica(false);
            }

            if (strcmp(nombre, "INICIARTRANSACCION") == 0)
            {
                if (!ctx->sqlite_db)
                {
                    contexto_set_error(ctx, "No hay conexión activa a base de datos");
                    return valor_crear_vacio();
                }
                int rc = sqlite3_exec((sqlite3 *)ctx->sqlite_db, "BEGIN TRANSACTION", NULL, NULL, NULL);
                if (rc != SQLITE_OK)
                {
                    contexto_set_error(ctx, "Error al iniciar transacción");
                    return valor_crear_vacio();
                }
                return valor_crear_entero(1);
            }

            if (strcmp(nombre, "CONFIRMARTRANSACCION") == 0)
            {
                if (!ctx->sqlite_db)
                {
                    contexto_set_error(ctx, "No hay conexión activa a base de datos");
                    return valor_crear_vacio();
                }
                int rc = sqlite3_exec((sqlite3 *)ctx->sqlite_db, "COMMIT", NULL, NULL, NULL);
                if (rc != SQLITE_OK)
                {
                    contexto_set_error(ctx, "Error al confirmar transacción");
                    return valor_crear_vacio();
                }
                return valor_crear_entero(1);
            }

            if (strcmp(nombre, "DESHACERTRANSACCION") == 0)
            {
                if (!ctx->sqlite_db)
                {
                    contexto_set_error(ctx, "No hay conexión activa a base de datos");
                    return valor_crear_vacio();
                }
                int rc = sqlite3_exec((sqlite3 *)ctx->sqlite_db, "ROLLBACK", NULL, NULL, NULL);
                if (rc != SQLITE_OK)
                {
                    contexto_set_error(ctx, "Error al deshacer transacción");
                    return valor_crear_vacio();
                }
                return valor_crear_entero(1);
            }

            if (strcmp(nombre, "INICIARSERVER") == 0)
            {
                int puerto = 8080;
                if (num_args == 1)
                {
                    if (args[0].tipo == VALOR_ENTERO)
                        puerto = (int)args[0].datos.entero;
                    else if (args[0].tipo == VALOR_DECIMAL)
                        puerto = (int)args[0].datos.decimal;
                }
                cmd_iniciarserver(ctx, puerto);
                return valor_crear_entero(1);
            }

            if (strcmp(nombre, "DETENERSERVER") == 0)
            {
                cmd_detenerserver(ctx);
                return valor_crear_entero(1);
            }

            contexto_set_error(ctx, "Función no reconocida");
            return valor_crear_vacio();
        }
        case AST_LLAMADA_FUNCION_MODIFICADORA:
        {
            const char *nombre = nodo->datos.llamada_funcion_modificadora.nombre_funcion;

            // Recolectar argumentos como AST (sin evaluar)
            NodoAST *args_ast[20];
            int num_args = 0;
            NodoAST *arg = nodo->datos.llamada_funcion_modificadora.argumentos ? nodo->datos.llamada_funcion_modificadora.argumentos->datos.bloque.primera : NULL;

            while (arg && num_args < 20)
            {
                args_ast[num_args++] = arg;
                arg = arg->siguiente;
            }

            // COPIARTEXTO(destino, origen)
            if (strcmp(nombre, "COPIARTEXTO") == 0 && num_args == 2)
            {
                Valor origen = evaluar_nodo(args_ast[1], ctx);
                if (ctx->hay_error)
                    return valor_crear_vacio();

                char *texto_origen = "";
                if (origen.tipo == VALOR_TEXTO || origen.tipo == VALOR_TEXTO_EXTENSO)
                {
                    texto_origen = origen.datos.texto ? origen.datos.texto : "";
                }

                asignar_texto_destino(args_ast[0], texto_origen, ctx);
                valor_destruir(&origen);
                return valor_crear_vacio();
            }

            // CONCATENARTEXTO(destino, texto)
            if (strcmp(nombre, "CONCATENARTEXTO") == 0 && num_args == 2)
            {
                Valor texto_agregar = evaluar_nodo(args_ast[1], ctx);
                if (ctx->hay_error)
                    return valor_crear_vacio();

                char *texto = "";
                if (texto_agregar.tipo == VALOR_TEXTO || texto_agregar.tipo == VALOR_TEXTO_EXTENSO)
                {
                    texto = texto_agregar.datos.texto ? texto_agregar.datos.texto : "";
                }

                // Obtener texto actual del destino
                char *texto_actual = obtener_texto_destino(args_ast[0], ctx);

                if (texto_actual)
                {
                    size_t len_actual = strlen(texto_actual);
                    size_t len_agregar = strlen(texto);
                    char *nuevo = malloc(len_actual + len_agregar + 1);
                    strcpy(nuevo, texto_actual);
                    strcat(nuevo, texto);

                    // Asignar el nuevo texto al destino
                    asignar_texto_destino(args_ast[0], nuevo, ctx);
                    free(nuevo);
                }
                valor_destruir(&texto_agregar);
                return valor_crear_vacio();
            }

            // MAYUSCULAS(variable)
            if (strcmp(nombre, "MAYUSCULAS") == 0 && num_args == 1)
            {
                char *nombre_var = extraer_nombre_variable(args_ast[0]);
                if (!nombre_var)
                {
                    contexto_set_error(ctx, "MAYUSCULAS: argumento debe ser variable");
                    return valor_crear_vacio();
                }

                Simbolo *sim = tabla_simbolos_buscar_simbolo(ctx->tabla_actual, nombre_var);
                if (sim && sim->valor.datos.texto)
                {
                    for (char *p = sim->valor.datos.texto; *p; p++)
                    {
                        *p = toupper((unsigned char)*p);
                    }
                }
                return valor_crear_vacio();
            }

            // MINUSCULAS(variable)
            if (strcmp(nombre, "MINUSCULAS") == 0 && num_args == 1)
            {
                char *nombre_var = extraer_nombre_variable(args_ast[0]);
                if (!nombre_var)
                {
                    contexto_set_error(ctx, "MINUSCULAS: argumento debe ser variable");
                    return valor_crear_vacio();
                }

                Simbolo *sim = tabla_simbolos_buscar_simbolo(ctx->tabla_actual, nombre_var);
                if (sim && sim->valor.datos.texto)
                {
                    for (char *p = sim->valor.datos.texto; *p; p++)
                    {
                        *p = tolower((unsigned char)*p);
                    }
                }
                return valor_crear_vacio();
            }

            // RECORTARTEXTO(variable)
            if (strcmp(nombre, "RECORTARTEXTO") == 0 && num_args == 1)
            {
                char *nombre_var = extraer_nombre_variable(args_ast[0]);
                if (!nombre_var)
                {
                    contexto_set_error(ctx, "RECORTARTEXTO: argumento debe ser variable");
                    return valor_crear_vacio();
                }

                Simbolo *sim = tabla_simbolos_buscar_simbolo(ctx->tabla_actual, nombre_var);
                if (sim && sim->valor.datos.texto)
                {
                    char *texto = sim->valor.datos.texto;
                    size_t len = strlen(texto);
                    size_t inicio = 0, fin = len;
                    while (inicio < len && isspace((unsigned char)texto[inicio]))
                        inicio++;
                    while (fin > inicio && isspace((unsigned char)texto[fin - 1]))
                        fin--;
                    size_t nueva_len = fin - inicio;
                    char *resultado = malloc(nueva_len + 1);
                    strncpy(resultado, texto + inicio, nueva_len);
                    resultado[nueva_len] = '\0';
                    free(sim->valor.datos.texto);
                    sim->valor.datos.texto = resultado;
                }
                return valor_crear_vacio();
            }

            // REEMPLAZARTEXTO(destino, buscar, reemplazar)
            if (strcmp(nombre, "REEMPLAZARTEXTO") == 0 && num_args == 3)
            {
                char *nombre_destino = extraer_nombre_variable(args_ast[0]);
                if (!nombre_destino)
                {
                    contexto_set_error(ctx, "REEMPLAZARTEXTO: primer argumento debe ser variable");
                    return valor_crear_vacio();
                }

                Valor buscar_val = evaluar_nodo(args_ast[1], ctx);
                if (ctx->hay_error)
                    return valor_crear_vacio();
                Valor reemplazar_val = evaluar_nodo(args_ast[2], ctx);
                if (ctx->hay_error)
                    return valor_crear_vacio();

                char *buscar = buscar_val.datos.texto ? buscar_val.datos.texto : "";
                char *reemplazar = reemplazar_val.datos.texto ? reemplazar_val.datos.texto : "";

                Simbolo *sim = tabla_simbolos_buscar_simbolo(ctx->tabla_actual, nombre_destino);
                if (sim && sim->valor.datos.texto)
                {
                    char *texto = sim->valor.datos.texto;
                    size_t len_buscar = strlen(buscar);
                    size_t len_reemplazar = strlen(reemplazar);

                    if (len_buscar == 0)
                    {
                        valor_destruir(&buscar_val);
                        valor_destruir(&reemplazar_val);
                        return valor_crear_vacio();
                    }

                    int count = 0;
                    char *tmp = texto;
                    while ((tmp = strstr(tmp, buscar)) != NULL)
                    {
                        count++;
                        tmp += len_buscar;
                    }

                    if (count == 0)
                    {
                        valor_destruir(&buscar_val);
                        valor_destruir(&reemplazar_val);
                        return valor_crear_vacio();
                    }

                    size_t nueva_len = strlen(texto) + count * (len_reemplazar - len_buscar);
                    char *resultado = malloc(nueva_len + 1);
                    char *dest = resultado;

                    while (*texto)
                    {
                        if (strncmp(texto, buscar, len_buscar) == 0)
                        {
                            strcpy(dest, reemplazar);
                            dest += len_reemplazar;
                            texto += len_buscar;
                        }
                        else
                        {
                            *dest++ = *texto++;
                        }
                    }
                    *dest = '\0';

                    free(sim->valor.datos.texto);
                    sim->valor.datos.texto = resultado;
                }
                valor_destruir(&buscar_val);
                valor_destruir(&reemplazar_val);
                return valor_crear_vacio();
            }

            // REPETIRTEXTO(texto, veces, destino)
            if (strcmp(nombre, "REPETIRTEXTO") == 0 && num_args == 3)
            {
                Valor texto_val = evaluar_nodo(args_ast[0], ctx);
                if (ctx->hay_error)
                    return valor_crear_vacio();

                char *texto = texto_val.datos.texto ? texto_val.datos.texto : "";

                Valor veces_val = evaluar_nodo(args_ast[1], ctx);
                if (ctx->hay_error)
                    return valor_crear_vacio();

                int veces = 0;
                if (veces_val.tipo == VALOR_ENTERO)
                    veces = (int)veces_val.datos.entero;
                else if (veces_val.tipo == VALOR_DECIMAL)
                    veces = (int)veces_val.datos.decimal;

                char *nombre_destino = extraer_nombre_variable(args_ast[2]);
                if (!nombre_destino)
                {
                    contexto_set_error(ctx, "REPETIRTEXTO: tercer argumento debe ser variable");
                    valor_destruir(&texto_val);
                    valor_destruir(&veces_val);
                    return valor_crear_vacio();
                }

                size_t len = strlen(texto);
                size_t total_len = len * veces;
                char *resultado = malloc(total_len + 1);
                resultado[0] = '\0';
                for (int i = 0; i < veces; i++)
                {
                    strcat(resultado, texto);
                }

                Simbolo *sim = tabla_simbolos_buscar_simbolo(ctx->tabla_actual, nombre_destino);
                if (sim)
                {
                    free(sim->valor.datos.texto);
                    sim->valor.datos.texto = resultado;
                }
                else
                {
                    free(resultado);
                }
                valor_destruir(&texto_val);
                valor_destruir(&veces_val);
                return valor_crear_vacio();
            }

            // EXTRAERTEXTO(origen, inicio, longitud, destino)
            if (strcmp(nombre, "EXTRAERTEXTO") == 0 && num_args == 4)
            {
                Valor texto_val = evaluar_nodo(args_ast[0], ctx);
                if (ctx->hay_error)
                    return valor_crear_vacio();

                char *texto = texto_val.datos.texto ? texto_val.datos.texto : "";

                Valor inicio_val = evaluar_nodo(args_ast[1], ctx);
                if (ctx->hay_error)
                    return valor_crear_vacio();
                Valor longitud_val = evaluar_nodo(args_ast[2], ctx);
                if (ctx->hay_error)
                    return valor_crear_vacio();

                int inicio = 0, longitud = 0;
                if (inicio_val.tipo == VALOR_ENTERO)
                    inicio = (int)inicio_val.datos.entero;
                else if (inicio_val.tipo == VALOR_DECIMAL)
                    inicio = (int)inicio_val.datos.decimal;
                if (longitud_val.tipo == VALOR_ENTERO)
                    longitud = (int)longitud_val.datos.entero;
                else if (longitud_val.tipo == VALOR_DECIMAL)
                    longitud = (int)longitud_val.datos.decimal;

                char *nombre_destino = extraer_nombre_variable(args_ast[3]);
                if (!nombre_destino)
                {
                    contexto_set_error(ctx, "EXTRAERTEXTO: cuarto argumento debe ser variable");
                    valor_destruir(&texto_val);
                    valor_destruir(&inicio_val);
                    valor_destruir(&longitud_val);
                    return valor_crear_vacio();
                }

                size_t len = strlen(texto);
                if (inicio < 0)
                    inicio = 0;
                if (inicio >= (int)len)
                {
                    Simbolo *sim = tabla_simbolos_buscar_simbolo(ctx->tabla_actual, nombre_destino);
                    if (sim)
                    {
                        free(sim->valor.datos.texto);
                        sim->valor.datos.texto = strdup("");
                    }
                    valor_destruir(&texto_val);
                    valor_destruir(&inicio_val);
                    valor_destruir(&longitud_val);
                    return valor_crear_vacio();
                }
                if (inicio + longitud > (int)len)
                {
                    longitud = (int)len - inicio;
                }

                char *resultado = malloc(longitud + 1);
                strncpy(resultado, texto + inicio, longitud);
                resultado[longitud] = '\0';

                Simbolo *sim = tabla_simbolos_buscar_simbolo(ctx->tabla_actual, nombre_destino);
                if (sim)
                {
                    free(sim->valor.datos.texto);
                    sim->valor.datos.texto = resultado;
                }
                else
                {
                    free(resultado);
                }
                valor_destruir(&texto_val);
                valor_destruir(&inicio_val);
                valor_destruir(&longitud_val);
                return valor_crear_vacio();
            }

            // DIVIDIRTEXTO(origen, separador, indice, destino)
            if (strcmp(nombre, "DIVIDIRTEXTO") == 0 && num_args == 4)
            {
                Valor texto_val = evaluar_nodo(args_ast[0], ctx);
                if (ctx->hay_error)
                    return valor_crear_vacio();

                char *texto = texto_val.datos.texto ? texto_val.datos.texto : "";

                Valor separador_val = evaluar_nodo(args_ast[1], ctx);
                if (ctx->hay_error)
                    return valor_crear_vacio();

                char separador;
                if (separador_val.tipo == VALOR_CARACTER)
                {
                    separador = separador_val.datos.texto ? separador_val.datos.texto[0] : '\0';
                }
                else if (separador_val.tipo == VALOR_TEXTO && separador_val.datos.texto)
                {
                    separador = separador_val.datos.texto[0];
                }

                else
                {
                    contexto_set_error(ctx, "DIVIDIRTEXTO: segundo argumento debe ser carácter");
                    valor_destruir(&texto_val);
                    valor_destruir(&separador_val);
                    return valor_crear_vacio();
                }

                Valor indice_val = evaluar_nodo(args_ast[2], ctx);
                if (ctx->hay_error)
                    return valor_crear_vacio();

                int indice = 0;
                if (indice_val.tipo == VALOR_ENTERO)
                    indice = (int)indice_val.datos.entero;
                else if (indice_val.tipo == VALOR_DECIMAL)
                    indice = (int)indice_val.datos.decimal;

                char *nombre_destino = extraer_nombre_variable(args_ast[3]);
                if (!nombre_destino)
                {
                    contexto_set_error(ctx, "DIVIDIRTEXTO: cuarto argumento debe ser variable");
                    valor_destruir(&texto_val);
                    valor_destruir(&separador_val);
                    valor_destruir(&indice_val);
                    return valor_crear_vacio();
                }

                char *inicio = texto;
                int actual = 0;
                char *resultado = NULL;

                while (*inicio)
                {
                    char *fin = strchr(inicio, separador);
                    if (actual == indice)
                    {
                        if (fin)
                        {
                            size_t len = fin - inicio;
                            resultado = malloc(len + 1);
                            strncpy(resultado, inicio, len);
                            resultado[len] = '\0';
                        }
                        else
                        {
                            resultado = strdup(inicio);
                        }
                        break;
                    }
                    if (!fin)
                        break;
                    inicio = fin + 1;
                    actual++;
                }

                Simbolo *sim = tabla_simbolos_buscar_simbolo(ctx->tabla_actual, nombre_destino);
                if (sim)
                {
                    free(sim->valor.datos.texto);
                    sim->valor.datos.texto = resultado ? resultado : strdup("");
                }
                else if (resultado)
                {
                    free(resultado);
                }
                valor_destruir(&texto_val);
                valor_destruir(&separador_val);
                valor_destruir(&indice_val);
                return valor_crear_vacio();
            }

            // ENTEROATEXTO(numero, destino)
            if (strcmp(nombre, "ENTEROATEXTO") == 0 && num_args == 2)
            {
                Valor num_val = evaluar_nodo(args_ast[0], ctx);
                if (ctx->hay_error)
                    return valor_crear_vacio();

                long long num = 0;
                if (num_val.tipo == VALOR_ENTERO)
                    num = num_val.datos.entero;
                else if (num_val.tipo == VALOR_DECIMAL)
                    num = (long long)num_val.datos.decimal;

                char *nombre_destino = extraer_nombre_variable(args_ast[1]);
                if (!nombre_destino)
                {
                    contexto_set_error(ctx, "ENTEROATEXTO: segundo argumento debe ser variable");
                    valor_destruir(&num_val);
                    return valor_crear_vacio();
                }

                char buffer[64];
                snprintf(buffer, sizeof(buffer), "%lld", num);

                Simbolo *sim = tabla_simbolos_buscar_simbolo(ctx->tabla_actual, nombre_destino);
                if (sim)
                {
                    free(sim->valor.datos.texto);
                    sim->valor.datos.texto = strdup(buffer);
                }
                valor_destruir(&num_val);
                return valor_crear_vacio();
            }

            // DECIMALATEXTO(numero, destino)
            if (strcmp(nombre, "DECIMALATEXTO") == 0 && num_args == 2)
            {
                Valor num_val = evaluar_nodo(args_ast[0], ctx);
                if (ctx->hay_error)
                    return valor_crear_vacio();

                double num = 0;
                if (num_val.tipo == VALOR_DECIMAL)
                    num = num_val.datos.decimal;
                else if (num_val.tipo == VALOR_ENTERO)
                    num = (double)num_val.datos.entero;

                char *nombre_destino = extraer_nombre_variable(args_ast[1]);
                if (!nombre_destino)
                {
                    contexto_set_error(ctx, "DECIMALATEXTO: segundo argumento debe ser variable");
                    valor_destruir(&num_val);
                    return valor_crear_vacio();
                }

                char buffer[64];
                snprintf(buffer, sizeof(buffer), "%g", num);

                Simbolo *sim = tabla_simbolos_buscar_simbolo(ctx->tabla_actual, nombre_destino);
                if (sim)
                {
                    free(sim->valor.datos.texto);
                    sim->valor.datos.texto = strdup(buffer);
                }
                valor_destruir(&num_val);
                return valor_crear_vacio();
            }

            // CARACTERATEXTO(caracter, destino)
            if (strcmp(nombre, "CARACTERATEXTO") == 0 && num_args == 2)
            {
                Valor car_val = evaluar_nodo(args_ast[0], ctx);
                if (ctx->hay_error)
                    return valor_crear_vacio();

                char c = '\0';
                if (car_val.tipo == VALOR_CARACTER)
                {
                    c = car_val.datos.texto ? car_val.datos.texto[0] : '\0';
                }
                else if (car_val.tipo == VALOR_TEXTO && car_val.datos.texto)
                {
                    c = car_val.datos.texto[0];
                }
                else if (car_val.tipo == VALOR_ENTERO)
                {
                    c = (char)car_val.datos.entero;
                }

                char *nombre_destino = extraer_nombre_variable(args_ast[1]);
                if (!nombre_destino)
                {
                    contexto_set_error(ctx, "CARACTERATEXTO: segundo argumento debe ser variable");
                    valor_destruir(&car_val);
                    return valor_crear_vacio();
                }

                char buffer[2] = {c, '\0'};

                Simbolo *sim = tabla_simbolos_buscar_simbolo(ctx->tabla_actual, nombre_destino);
                if (sim)
                {
                    free(sim->valor.datos.texto);
                    sim->valor.datos.texto = strdup(buffer);
                }
                valor_destruir(&car_val);
                return valor_crear_vacio();
            }

            // INVERTIRTEXTO(texto, destino)
            if (strcmp(nombre, "INVERTIRTEXTO") == 0 && num_args == 2)
            {
                Valor texto_val = evaluar_nodo(args_ast[0], ctx);
                if (ctx->hay_error)
                    return valor_crear_vacio();

                if (texto_val.tipo != VALOR_TEXTO && texto_val.tipo != VALOR_TEXTO_EXTENSO)
                {
                    contexto_set_error(ctx, "INVERTIRTEXTO requiere texto como primer argumento");
                    valor_destruir(&texto_val);
                    return valor_crear_vacio();
                }

                const char *texto = texto_val.datos.texto ? texto_val.datos.texto : "";
                size_t len = strlen(texto);

                // Crear string invertido
                char *invertido = malloc(len + 1);
                if (!invertido)
                {
                    contexto_set_error(ctx, "INVERTIRTEXTO: no se pudo asignar memoria");
                    valor_destruir(&texto_val);
                    return valor_crear_vacio();
                }

                // Invertir byte por byte
                for (size_t i = 0; i < len; i++)
                {
                    invertido[i] = texto[len - 1 - i];
                }
                invertido[len] = '\0';

                // Obtener nombre de variable destino (NO liberar este puntero)
                char *nombre_destino = extraer_nombre_variable(args_ast[1]);
                if (!nombre_destino)
                {
                    contexto_set_error(ctx, "INVERTIRTEXTO: segundo argumento debe ser variable");
                    free(invertido);
                    valor_destruir(&texto_val);
                    return valor_crear_vacio();
                }

                // Buscar símbolo y asignar directamente (mismo patrón que REPETIRTEXTO)
                Simbolo *sim = tabla_simbolos_buscar_simbolo(ctx->tabla_actual, nombre_destino);
                if (sim)
                {
                    free(sim->valor.datos.texto);
                    sim->valor.datos.texto = invertido;
                }
                else
                {
                    free(invertido);
                }

                valor_destruir(&texto_val);
                return valor_crear_vacio();
            }

            contexto_set_error(ctx, "Función modificadora no reconocida");
            return valor_crear_vacio();
        }

        // --------------------------------------------------------
        // BLOQUE
        // --------------------------------------------------------
        case AST_BLOQUE:
            return evaluar_bloque(nodo, ctx);
        
        // --------------------------------------------------------
        // DECLARACIONES
        // --------------------------------------------------------
        case AST_DECLARACION_VARIABLE:
        {
            const char *nombre = nodo->datos.declaracion_variable.nombre;
            Valor valor_inicial;
            if (nodo->datos.declaracion_variable.valor_inicial)
            {
                valor_inicial = evaluar_nodo(nodo->datos.declaracion_variable.valor_inicial, ctx);
                if (ctx->hay_error)
                    return valor_crear_vacio();

                // Procesar interpolación si es un texto
                if (valor_inicial.tipo == VALOR_TEXTO || valor_inicial.tipo == VALOR_TEXTO_EXTENSO)
                {
                    // Simplemente copiar el texto sin interpolación por ahora
                    char *texto_copia = strdup(valor_inicial.datos.texto ? valor_inicial.datos.texto : "");
                    valor_destruir(&valor_inicial);
                    valor_inicial = valor_crear_texto(texto_copia);
                    free(texto_copia);
                }

                // VALIDACIÓN INLINE PARA TIPOS SIN SIGNO
                TipoDato tipo_declarado = nodo->datos.declaracion_variable.tipo_dato;

                if (tipo_declarado == TIPO_ENTERO_SIN_SIGNO)
                {
                    if (valor_inicial.tipo == VALOR_ENTERO && valor_inicial.datos.entero < 0)
                    {
                        char msg[256];
                        snprintf(msg, sizeof(msg),
                                 "No se puede asignar el valor negativo %lld a la variable ENTERA SIN SIGNO '$%s'",
                                 valor_inicial.datos.entero, nombre);
                        contexto_set_error(ctx, msg);
                        valor_destruir(&valor_inicial);
                        return valor_crear_vacio();
                    }
                    // Convertir a ENTERO_SIN_SIGNO si es ENTERO positivo
                    if (valor_inicial.tipo == VALOR_ENTERO)
                    {
                        unsigned long long val = (unsigned long long)valor_inicial.datos.entero;
                        valor_destruir(&valor_inicial);
                        valor_inicial = valor_crear_entero_sin_signo(val);
                    }
                }
                else if (tipo_declarado == TIPO_DECIMAL_SIN_SIGNO)
                {
                    if (valor_inicial.tipo == VALOR_DECIMAL && valor_inicial.datos.decimal < 0.0)
                    {
                        char msg[256];
                        snprintf(msg, sizeof(msg),
                                 "No se puede asignar el valor negativo %g a la variable DECIMAL SIN SIGNO '$%s'",
                                 valor_inicial.datos.decimal, nombre);
                        contexto_set_error(ctx, msg);
                        valor_destruir(&valor_inicial);
                        return valor_crear_vacio();
                    }
                    if (valor_inicial.tipo == VALOR_ENTERO && valor_inicial.datos.entero < 0)
                    {
                        char msg[256];
                        snprintf(msg, sizeof(msg),
                                 "No se puede asignar el valor negativo %lld a la variable DECIMAL SIN SIGNO '$%s'",
                                 valor_inicial.datos.entero, nombre);
                        contexto_set_error(ctx, msg);
                        valor_destruir(&valor_inicial);
                        return valor_crear_vacio();
                    }
                    // Convertir a DECIMAL_SIN_SIGNO
                    if (valor_inicial.tipo == VALOR_DECIMAL)
                    {
                        double val = valor_inicial.datos.decimal;
                        valor_destruir(&valor_inicial);
                        valor_inicial = valor_crear_decimal_sin_signo(val);
                    }
                    else if (valor_inicial.tipo == VALOR_ENTERO)
                    {
                        double val = (double)valor_inicial.datos.entero;
                        valor_destruir(&valor_inicial);
                        valor_inicial = valor_crear_decimal_sin_signo(val);
                    }
                }
            }
            else
            {
                // Inicializar con valor por defecto según el tipo
                switch (nodo->datos.declaracion_variable.tipo_dato)
                {
                case TIPO_ENTERO:
                    valor_inicial = valor_crear_entero(0);
                    break;
                case TIPO_ENTERO_SIN_SIGNO:
                    valor_inicial = valor_crear_entero_sin_signo(0);
                    break;
                case TIPO_DECIMAL:
                    valor_inicial = valor_crear_decimal(0.0);
                    break;
                case TIPO_DECIMAL_SIN_SIGNO:
                    valor_inicial = valor_crear_decimal_sin_signo(0.0);
                    break;
                case TIPO_CARACTER:
                case TIPO_CARACTER_SIN_SIGNO:
                    valor_inicial = valor_crear_caracter("\0");
                    break;
                case TIPO_TEXTO:
                case TIPO_TEXTO_EXTENSO:
                    valor_inicial = valor_crear_texto("");
                    break;
                case TIPO_LOGICA:
                    valor_inicial = valor_crear_logica(false);
                    break;
                case TIPO_ARCHIVO:
                    valor_inicial.tipo = VALOR_ARCHIVO;
                    valor_inicial.datos.archivo = NULL;
                    valor_inicial.modo_archivo = 0;
                    valor_inicial.tamano = 0;
                    valor_inicial.filas = 0;
                    valor_inicial.columnas = 0;
                    break;
                default:
                    valor_inicial = valor_crear_vacio();
                    break;
                }
            }

            tabla_simbolos_definir(ctx->tabla_actual, nombre, valor_inicial, false);
            // Registrar el tipo declarado de la variable
            Simbolo *sim = tabla_simbolos_buscar_simbolo(ctx->tabla_actual, nombre);
            if (sim)
            {
                sim->tipo_declarado = nodo->datos.declaracion_variable.tipo_dato;
            }
            return valor_crear_vacio();
        }

        case AST_DECLARACION_CONSTANTE:
        {
            const char *nombre = nodo->datos.declaracion_constante.nombre;
            TipoDato tipo_declarado = nodo->datos.declaracion_constante.tipo_dato;
            Valor valor = evaluar_nodo(nodo->datos.declaracion_constante.valor, ctx);
            if (ctx->hay_error)
                return valor_crear_vacio();

            // VALIDAR COMPATIBILIDAD DE TIPOS
            if (!tipo_valor_compatible(tipo_declarado, valor.tipo))
            {
                char msg[256];
                snprintf(msg, sizeof(msg),
                         "No puede asignarse un valor de tipo incompatible a la constante '$%s'.",
                         nombre);
                contexto_set_error(ctx, msg);
                valor_destruir(&valor);
                return valor_crear_vacio();
            }

            // VALIDAR SIGNO NEGATIVO PARA TIPOS SIN SIGNO
            if (tipo_declarado == TIPO_ENTERO_SIN_SIGNO)
            {
                if (valor.tipo == VALOR_ENTERO && valor.datos.entero < 0)
                {
                    char msg[256];
                    snprintf(msg, sizeof(msg),
                             "La constante '$%s' es ENTERA SIN SIGNO y no admite valores negativos.",
                             nombre);
                    contexto_set_error(ctx, msg);
                    valor_destruir(&valor);
                    return valor_crear_vacio();
                }
                if (valor.tipo == VALOR_DECIMAL && valor.datos.decimal < 0.0)
                {
                    char msg[256];
                    snprintf(msg, sizeof(msg),
                             "La constante '$%s' es ENTERA SIN SIGNO y no admite valores negativos.",
                             nombre);
                    contexto_set_error(ctx, msg);
                    valor_destruir(&valor);
                    return valor_crear_vacio();
                }
            }
            else if (tipo_declarado == TIPO_DECIMAL_SIN_SIGNO)
            {
                if (valor.tipo == VALOR_DECIMAL && valor.datos.decimal < 0.0)
                {
                    char msg[256];
                    snprintf(msg, sizeof(msg),
                             "La constante '$%s' es DECIMAL SIN SIGNO y no admite valores negativos.",
                             nombre);
                    contexto_set_error(ctx, msg);
                    valor_destruir(&valor);
                    return valor_crear_vacio();
                }
                if (valor.tipo == VALOR_ENTERO && valor.datos.entero < 0)
                {
                    char msg[256];
                    snprintf(msg, sizeof(msg),
                             "La constante '$%s' es DECIMAL SIN SIGNO y no admite valores negativos.",
                             nombre);
                    contexto_set_error(ctx, msg);
                    valor_destruir(&valor);
                    return valor_crear_vacio();
                }
            }

            // VALIDAR COMPATIBILIDAD ENTERO/DECIMAL
            if (tipo_declarado == TIPO_ENTERO || tipo_declarado == TIPO_ENTERO_SIN_SIGNO)
            {
                if (valor.tipo == VALOR_DECIMAL || valor.tipo == VALOR_DECIMAL_SIN_SIGNO)
                {
                    char msg[256];
                    snprintf(msg, sizeof(msg),
                             "La constante '$%s' es ENTERA y no admite valores decimales.",
                             nombre);
                    contexto_set_error(ctx, msg);
                    valor_destruir(&valor);
                    return valor_crear_vacio();
                }
            }

            tabla_simbolos_definir(ctx->tabla_actual, nombre, valor, true);
            return valor_crear_vacio();
        }

        case AST_DECLARACION_LISTA:
        {
            const char *nombre = nodo->datos.declaracion_lista.nombre;
            TipoDato tipo = nodo->datos.declaracion_lista.tipo_elemento;
            int tamano = nodo->datos.declaracion_lista.tamano;
            NodoAST *valores_iniciales = nodo->datos.declaracion_lista.valores_iniciales;

            // Crear valor de tipo lista
            Valor lista;
            lista.tipo = VALOR_LISTA;
            lista.tamano = tamano;
            lista.filas = 0;
            lista.columnas = 0;
            lista.tipo_elemento = tipo;

            // Asignar array de valores
            lista.datos.lista = malloc(sizeof(Valor) * tamano);
            if (!lista.datos.lista)
            {
                contexto_set_error(ctx, "No se pudo asignar memoria para la lista");
                return valor_crear_vacio();
            }

            // Inicializar elementos según el tipo
            for (int i = 0; i < tamano; i++)
            {
                switch (tipo)
                {
                case TIPO_ENTERO:
                    lista.datos.lista[i] = valor_crear_entero(0);
                    break;
                case TIPO_ENTERO_SIN_SIGNO:
                    lista.datos.lista[i] = valor_crear_entero_sin_signo(0);
                    break;
                case TIPO_DECIMAL:
                    lista.datos.lista[i] = valor_crear_decimal(0.0);
                    break;
                case TIPO_DECIMAL_SIN_SIGNO:
                    lista.datos.lista[i] = valor_crear_decimal_sin_signo(0.0);
                    break;
                case TIPO_CARACTER:
                {
                    char buffer[2] = {' ', '\0'};
                    lista.datos.lista[i] = valor_crear_caracter(buffer);
                    break;
                }

                case TIPO_CARACTER_SIN_SIGNO:
                    lista.datos.lista[i] = valor_crear_caracter_sin_signo(0);
                    break;

                case TIPO_TEXTO:
                    lista.datos.lista[i] = valor_crear_texto("");
                    break;
                
                case TIPO_TEXTO_EXTENSO:
                    lista.datos.lista[i] = valor_crear_texto("");
                    break;
                
                case TIPO_LOGICA:
                    lista.datos.lista[i] = valor_crear_logica(0);
                    break;

                default:
                    lista.datos.lista[i] = valor_crear_entero(0);
                    break;
                }
            }

            // Si hay valores iniciales, asignarlos
            if (valores_iniciales && valores_iniciales->tipo == AST_BLOQUE)
            {
                NodoAST *actual = valores_iniciales->datos.bloque.primera;
                int idx = 0;
                while (actual && idx < tamano)
                {
                    Valor val = evaluar_nodo(actual, ctx);
                    if (ctx->hay_error)
                    {
                        free(lista.datos.lista);
                        return valor_crear_vacio();
                    }

                    // VALIDAR SIGNO NEGATIVO PARA TIPOS SIN SIGNO
                    if (tipo == TIPO_ENTERO_SIN_SIGNO)
                    {
                        if (val.tipo == VALOR_ENTERO && val.datos.entero < 0)
                        {
                            char msg[256];
                            snprintf(msg, sizeof(msg),
                                     "La lista '%s' es ENTERA SIN SIGNO y no admite valores negativos en la posición %d.",
                                     nombre, idx);
                            contexto_set_error(ctx, msg);
                            free(lista.datos.lista);
                            return valor_crear_vacio();
                        }
                        if (val.tipo == VALOR_DECIMAL && val.datos.decimal < 0.0)
                        {
                            char msg[256];
                            snprintf(msg, sizeof(msg),
                                     "La lista '%s' es ENTERA SIN SIGNO y no admite valores negativos en la posición %d.",
                                     nombre, idx);
                            contexto_set_error(ctx, msg);
                            free(lista.datos.lista);
                            return valor_crear_vacio();
                        }
                    }
                    else if (tipo == TIPO_DECIMAL_SIN_SIGNO)
                    {
                        if (val.tipo == VALOR_DECIMAL && val.datos.decimal < 0.0)
                        {
                            char msg[256];
                            snprintf(msg, sizeof(msg),
                                     "La lista '%s' es DECIMAL SIN SIGNO y no admite valores negativos en la posición %d.",
                                     nombre, idx);
                            contexto_set_error(ctx, msg);
                            free(lista.datos.lista);
                            return valor_crear_vacio();
                        }
                        if (val.tipo == VALOR_ENTERO && val.datos.entero < 0)
                        {
                            char msg[256];
                            snprintf(msg, sizeof(msg),
                                     "La lista '%s' es DECIMAL SIN SIGNO y no admite valores negativos en la posición %d.",
                                     nombre, idx);
                            contexto_set_error(ctx, msg);
                            free(lista.datos.lista);
                            return valor_crear_vacio();
                        }
                    }

                    // VALIDAR COMPATIBILIDAD DE TIPOS
                    if (tipo == TIPO_ENTERO || tipo == TIPO_ENTERO_SIN_SIGNO)
                    {
                        if (val.tipo == VALOR_DECIMAL || val.tipo == VALOR_DECIMAL_SIN_SIGNO)
                        {
                            char msg[256];
                            snprintf(msg, sizeof(msg),
                                     "La lista '%s' es ENTERA y no admite valores decimales en la posición %d.",
                                     nombre, idx);
                            contexto_set_error(ctx, msg);
                            free(lista.datos.lista);
                            return valor_crear_vacio();
                        }
                    }

                    // Destruir el valor anterior y asignar el nuevo
                    valor_destruir(&lista.datos.lista[idx]);
                    lista.datos.lista[idx] = val;
                    actual = actual->siguiente;
                    idx++;
                }
            }

            // Registrar en la tabla de símbolos
            tabla_simbolos_definir(ctx->tabla_actual, nombre, lista, false);
            return valor_crear_vacio();
        }

        case AST_DECLARACION_MATRIZ:
        {
            const char *nombre = nodo->datos.declaracion_matriz.nombre;
            TipoDato tipo = nodo->datos.declaracion_matriz.tipo_elemento;
            int filas = nodo->datos.declaracion_matriz.filas;
            int columnas = nodo->datos.declaracion_matriz.columnas;
            NodoAST *valores_iniciales = nodo->datos.declaracion_matriz.valores_iniciales;

            // Crear valor de tipo matriz
            Valor matriz;
            matriz.tipo = VALOR_MATRIZ;
            matriz.tamano = 0;
            matriz.filas = filas;
            matriz.columnas = columnas;
            matriz.tipo_elemento = tipo;

            // Asignar array 2D
            matriz.datos.matriz = malloc(sizeof(Valor *) * filas);
            if (!matriz.datos.matriz)
            {
                contexto_set_error(ctx, "No se pudo asignar memoria para la matriz");
                return valor_crear_vacio();
            }

            for (int i = 0; i < filas; i++)
            {
                matriz.datos.matriz[i] = malloc(sizeof(Valor) * columnas);
                if (!matriz.datos.matriz[i])
                {
                    // Liberar lo que ya se asignó
                    for (int j = 0; j < i; j++)
                    {
                        free(matriz.datos.matriz[j]);
                    }
                    free(matriz.datos.matriz);
                    contexto_set_error(ctx, "No se pudo asignar memoria para la matriz");
                    return valor_crear_vacio();
                }

                // Inicializar elementos según el tipo
                for (int j = 0; j < columnas; j++)
                {
                    switch (tipo)
                    {
                    case TIPO_ENTERO:
                        matriz.datos.matriz[i][j] = valor_crear_entero(0);
                        break;
                    case TIPO_ENTERO_SIN_SIGNO:
                        matriz.datos.matriz[i][j] = valor_crear_entero_sin_signo(0);
                        break;
                    case TIPO_DECIMAL:
                        matriz.datos.matriz[i][j] = valor_crear_decimal(0.0);
                        break;
                    case TIPO_DECIMAL_SIN_SIGNO:
                        matriz.datos.matriz[i][j] = valor_crear_decimal_sin_signo(0.0);
                        break;
                    case TIPO_CARACTER:
                    {
                        char buffer[2] = {' ', '\0'};
                        matriz.datos.matriz[i][j] = valor_crear_caracter(buffer);
                        break;
                    }
                    case TIPO_CARACTER_SIN_SIGNO:
                        matriz.datos.matriz[i][j] = valor_crear_caracter_sin_signo(0);
                        break;
                    case TIPO_TEXTO:
                        matriz.datos.matriz[i][j] = valor_crear_texto("");
                        break;
                    case TIPO_TEXTO_EXTENSO:
                        matriz.datos.matriz[i][j] = valor_crear_texto("");
                        break;
                    case TIPO_LOGICA:
                        matriz.datos.matriz[i][j] = valor_crear_logica(0);
                        break;
                    default:
                        matriz.datos.matriz[i][j] = valor_crear_entero(0);
                        break;
                    }
                }
            }

            // Si hay valores iniciales, asignarlos
            if (valores_iniciales && valores_iniciales->tipo == AST_BLOQUE)
            {
                NodoAST *fila_nodo = valores_iniciales->datos.bloque.primera;
                int fila_idx = 0;

                while (fila_nodo && fila_idx < filas)
                {
                    if (fila_nodo->tipo == AST_BLOQUE)
                    {
                        NodoAST *col_nodo = fila_nodo->datos.bloque.primera;
                        int col_idx = 0;

                        while (col_nodo && col_idx < columnas)
                        {
                            Valor val = evaluar_nodo(col_nodo, ctx);
                            if (ctx->hay_error)
                            {
                                // Liberar todo
                                for (int i = 0; i < filas; i++)
                                {
                                    free(matriz.datos.matriz[i]);
                                }
                                free(matriz.datos.matriz);
                                return valor_crear_vacio();
                            }

                            // VALIDAR SIGNO NEGATIVO PARA TIPOS SIN SIGNO
                            if (tipo == TIPO_ENTERO_SIN_SIGNO)
                            {
                                if (val.tipo == VALOR_ENTERO && val.datos.entero < 0)
                                {
                                    char msg[256];
                                    snprintf(msg, sizeof(msg),
                                             "La matriz '%s' es ENTERA SIN SIGNO y no admite valores negativos en [%d][%d].",
                                             nombre, fila_idx, col_idx);
                                    contexto_set_error(ctx, msg);
                                    for (int i = 0; i < filas; i++)
                                    {
                                        free(matriz.datos.matriz[i]);
                                    }
                                    free(matriz.datos.matriz);
                                    return valor_crear_vacio();
                                }
                                if (val.tipo == VALOR_DECIMAL && val.datos.decimal < 0.0)
                                {
                                    char msg[256];
                                    snprintf(msg, sizeof(msg),
                                             "La matriz '%s' es ENTERA SIN SIGNO y no admite valores negativos en [%d][%d].",
                                             nombre, fila_idx, col_idx);
                                    contexto_set_error(ctx, msg);
                                    for (int i = 0; i < filas; i++)
                                    {
                                        free(matriz.datos.matriz[i]);
                                    }
                                    free(matriz.datos.matriz);
                                    return valor_crear_vacio();
                                }
                            }
                            else if (tipo == TIPO_DECIMAL_SIN_SIGNO)
                            {
                                if (val.tipo == VALOR_DECIMAL && val.datos.decimal < 0.0)
                                {
                                    char msg[256];
                                    snprintf(msg, sizeof(msg),
                                             "La matriz '%s' es DECIMAL SIN SIGNO y no admite valores negativos en [%d][%d].",
                                             nombre, fila_idx, col_idx);
                                    contexto_set_error(ctx, msg);
                                    for (int i = 0; i < filas; i++)
                                    {
                                        free(matriz.datos.matriz[i]);
                                    }
                                    free(matriz.datos.matriz);
                                    return valor_crear_vacio();
                                }
                                if (val.tipo == VALOR_ENTERO && val.datos.entero < 0)
                                {
                                    char msg[256];
                                    snprintf(msg, sizeof(msg),
                                             "La matriz '%s' es DECIMAL SIN SIGNO y no admite valores negativos en [%d][%d].",
                                             nombre, fila_idx, col_idx);
                                    contexto_set_error(ctx, msg);
                                    for (int i = 0; i < filas; i++)
                                    {
                                        free(matriz.datos.matriz[i]);
                                    }
                                    free(matriz.datos.matriz);
                                    return valor_crear_vacio();
                                }
                            }

                            // VALIDAR COMPATIBILIDAD DE TIPOS
                            if (tipo == TIPO_ENTERO || tipo == TIPO_ENTERO_SIN_SIGNO)
                            {
                                if (val.tipo == VALOR_DECIMAL || val.tipo == VALOR_DECIMAL_SIN_SIGNO)
                                {
                                    char msg[256];
                                    snprintf(msg, sizeof(msg),
                                             "La matriz '%s' es ENTERA y no admite valores decimales en [%d][%d].",
                                             nombre, fila_idx, col_idx);
                                    contexto_set_error(ctx, msg);
                                    for (int i = 0; i < filas; i++)
                                    {
                                        free(matriz.datos.matriz[i]);
                                    }
                                    free(matriz.datos.matriz);
                                    return valor_crear_vacio();
                                }
                            }

                            valor_destruir(&matriz.datos.matriz[fila_idx][col_idx]);
                            matriz.datos.matriz[fila_idx][col_idx] = val;
                            col_nodo = col_nodo->siguiente;
                            col_idx++;
                        }
                    }
                    fila_nodo = fila_nodo->siguiente;
                    fila_idx++;
                }
            }

            // Registrar en la tabla de símbolos
            tabla_simbolos_definir(ctx->tabla_actual, nombre, matriz, false);
            return valor_crear_vacio();
        }

        case AST_DECLARACION_MATRIZ3D:
        {
            const char *nombre = nodo->datos.declaracion_matriz3d.nombre;
            TipoDato tipo = nodo->datos.declaracion_matriz3d.tipo_elemento;
            int dim1 = nodo->datos.declaracion_matriz3d.dim1;
            int dim2 = nodo->datos.declaracion_matriz3d.dim2;
            int dim3 = nodo->datos.declaracion_matriz3d.dim3;
            NodoAST *valores_iniciales = nodo->datos.declaracion_matriz3d.valores_iniciales;

            // Crear valor de tipo matriz 3D
            Valor matriz3d;
            matriz3d.tipo = VALOR_MATRIZ3D;
            matriz3d.tamano = 0;
            matriz3d.filas = dim1;
            matriz3d.columnas = dim2;
            matriz3d.profundidad = dim3;
            matriz3d.tipo_elemento = tipo;

            // Asignar array 3D
            matriz3d.datos.matriz3d = malloc(sizeof(Valor **) * dim1);
            if (!matriz3d.datos.matriz3d)
            {
                contexto_set_error(ctx, "No se pudo asignar memoria para la matriz 3D");
                return valor_crear_vacio();
            }

            for (int i = 0; i < dim1; i++)
            {
                matriz3d.datos.matriz3d[i] = malloc(sizeof(Valor *) * dim2);
                if (!matriz3d.datos.matriz3d[i])
                {
                    for (int j = 0; j < i; j++)
                    {
                        for (int k = 0; k < dim2; k++)
                        {
                            free(matriz3d.datos.matriz3d[j][k]);
                        }
                        free(matriz3d.datos.matriz3d[j]);
                    }
                    free(matriz3d.datos.matriz3d);
                    contexto_set_error(ctx, "No se pudo asignar memoria para la matriz 3D");
                    return valor_crear_vacio();
                }

                for (int j = 0; j < dim2; j++)
                {
                    matriz3d.datos.matriz3d[i][j] = malloc(sizeof(Valor) * dim3);
                    if (!matriz3d.datos.matriz3d[i][j])
                    {
                        for (int ii = 0; ii < dim1; ii++)
                        {
                            for (int jj = 0; jj < dim2; jj++)
                            {
                                if (matriz3d.datos.matriz3d[ii][jj])
                                {
                                    free(matriz3d.datos.matriz3d[ii][jj]);
                                }
                            }
                            free(matriz3d.datos.matriz3d[ii]);
                        }
                        free(matriz3d.datos.matriz3d);
                        contexto_set_error(ctx, "No se pudo asignar memoria para la matriz 3D");
                        return valor_crear_vacio();
                    }

                    // Inicializar elementos según el tipo
                    for (int k = 0; k < dim3; k++)
                    {
                        switch (tipo)
                        {
                        case TIPO_ENTERO:
                            matriz3d.datos.matriz3d[i][j][k] = valor_crear_entero(0);
                            break;
                        case TIPO_ENTERO_SIN_SIGNO:
                            matriz3d.datos.matriz3d[i][j][k] = valor_crear_entero_sin_signo(0);
                            break;
                        case TIPO_DECIMAL:
                            matriz3d.datos.matriz3d[i][j][k] = valor_crear_decimal(0.0);
                            break;
                        case TIPO_DECIMAL_SIN_SIGNO:
                            matriz3d.datos.matriz3d[i][j][k] = valor_crear_decimal_sin_signo(0.0);
                            break;
                        case TIPO_CARACTER:
                        {
                            char buffer[2] = {' ', '\0'};
                            matriz3d.datos.matriz3d[i][j][k] = valor_crear_caracter(buffer);
                            break;
                        }
                        case TIPO_CARACTER_SIN_SIGNO:
                            matriz3d.datos.matriz3d[i][j][k] = valor_crear_caracter_sin_signo(0);
                            break;
                        case TIPO_TEXTO:
                            matriz3d.datos.matriz3d[i][j][k] = valor_crear_texto("");
                            break;
                        case TIPO_TEXTO_EXTENSO:
                            matriz3d.datos.matriz3d[i][j][k] = valor_crear_texto("");
                            break;
                        case TIPO_LOGICA:
                            matriz3d.datos.matriz3d[i][j][k] = valor_crear_logica(0);
                            break;
                        default:
                            matriz3d.datos.matriz3d[i][j][k] = valor_crear_entero(0);
                            break;
                        }
                    }
                }
            }

            // Si hay valores iniciales, asignarlos
            if (valores_iniciales && valores_iniciales->tipo == AST_BLOQUE)
            {
                NodoAST *plano_nodo = valores_iniciales->datos.bloque.primera;
                int plano_idx = 0;

                while (plano_nodo && plano_idx < dim1)
                {
                    if (plano_nodo->tipo == AST_BLOQUE)
                    {
                        NodoAST *fila_nodo = plano_nodo->datos.bloque.primera;
                        int fila_idx = 0;

                        while (fila_nodo && fila_idx < dim2)
                        {
                            if (fila_nodo->tipo == AST_BLOQUE)
                            {
                                NodoAST *col_nodo = fila_nodo->datos.bloque.primera;
                                int col_idx = 0;

                                while (col_nodo && col_idx < dim3)
                                {
                                    Valor val = evaluar_nodo(col_nodo, ctx);
                                    if (ctx->hay_error)
                                    {
                                        // Liberar todo
                                        for (int i = 0; i < dim1; i++)
                                        {
                                            for (int j = 0; j < dim2; j++)
                                            {
                                                free(matriz3d.datos.matriz3d[i][j]);
                                            }
                                            free(matriz3d.datos.matriz3d[i]);
                                        }
                                        free(matriz3d.datos.matriz3d);
                                        return valor_crear_vacio();
                                    }

                                    // VALIDAR SIGNO NEGATIVO PARA TIPOS SIN SIGNO
                                    if (tipo == TIPO_ENTERO_SIN_SIGNO)
                                    {
                                        if (val.tipo == VALOR_ENTERO && val.datos.entero < 0)
                                        {
                                            char msg[256];
                                            snprintf(msg, sizeof(msg),
                                                     "La matriz 3D '%s' es ENTERA SIN SIGNO y no admite valores negativos en [%d][%d][%d].",
                                                     nombre, plano_idx, fila_idx, col_idx);
                                            contexto_set_error(ctx, msg);
                                            for (int i = 0; i < dim1; i++)
                                            {
                                                for (int j = 0; j < dim2; j++)
                                                {
                                                    free(matriz3d.datos.matriz3d[i][j]);
                                                }
                                                free(matriz3d.datos.matriz3d[i]);
                                            }
                                            free(matriz3d.datos.matriz3d);
                                            return valor_crear_vacio();
                                        }
                                        if (val.tipo == VALOR_DECIMAL && val.datos.decimal < 0.0)
                                        {
                                            char msg[256];
                                            snprintf(msg, sizeof(msg),
                                                     "La matriz 3D '%s' es ENTERA SIN SIGNO y no admite valores negativos en [%d][%d][%d].",
                                                     nombre, plano_idx, fila_idx, col_idx);
                                            contexto_set_error(ctx, msg);
                                            for (int i = 0; i < dim1; i++)
                                            {
                                                for (int j = 0; j < dim2; j++)
                                                {
                                                    free(matriz3d.datos.matriz3d[i][j]);
                                                }
                                                free(matriz3d.datos.matriz3d[i]);
                                            }
                                            free(matriz3d.datos.matriz3d);
                                            return valor_crear_vacio();
                                        }
                                    }
                                    else if (tipo == TIPO_DECIMAL_SIN_SIGNO)
                                    {
                                        if (val.tipo == VALOR_DECIMAL && val.datos.decimal < 0.0)
                                        {
                                            char msg[256];
                                            snprintf(msg, sizeof(msg),
                                                     "La matriz 3D '%s' es DECIMAL SIN SIGNO y no admite valores negativos en [%d][%d][%d].",
                                                     nombre, plano_idx, fila_idx, col_idx);
                                            contexto_set_error(ctx, msg);
                                            for (int i = 0; i < dim1; i++)
                                            {
                                                for (int j = 0; j < dim2; j++)
                                                {
                                                    free(matriz3d.datos.matriz3d[i][j]);
                                                }
                                                free(matriz3d.datos.matriz3d[i]);
                                            }
                                            free(matriz3d.datos.matriz3d);
                                            return valor_crear_vacio();
                                        }
                                        if (val.tipo == VALOR_ENTERO && val.datos.entero < 0)
                                        {
                                            char msg[256];
                                            snprintf(msg, sizeof(msg),
                                                     "La matriz 3D '%s' es DECIMAL SIN SIGNO y no admite valores negativos en [%d][%d][%d].",
                                                     nombre, plano_idx, fila_idx, col_idx);
                                            contexto_set_error(ctx, msg);
                                            for (int i = 0; i < dim1; i++)
                                            {
                                                for (int j = 0; j < dim2; j++)
                                                {
                                                    free(matriz3d.datos.matriz3d[i][j]);
                                                }
                                                free(matriz3d.datos.matriz3d[i]);
                                            }
                                            free(matriz3d.datos.matriz3d);
                                            return valor_crear_vacio();
                                        }
                                    }

                                    // VALIDAR COMPATIBILIDAD DE TIPOS
                                    if (tipo == TIPO_ENTERO || tipo == TIPO_ENTERO_SIN_SIGNO)
                                    {
                                        if (val.tipo == VALOR_DECIMAL || val.tipo == VALOR_DECIMAL_SIN_SIGNO)
                                        {
                                            char msg[256];
                                            snprintf(msg, sizeof(msg),
                                                     "La matriz 3D '%s' es ENTERA y no admite valores decimales en [%d][%d][%d].",
                                                     nombre, plano_idx, fila_idx, col_idx);
                                            contexto_set_error(ctx, msg);
                                            for (int i = 0; i < dim1; i++)
                                            {
                                                for (int j = 0; j < dim2; j++)
                                                {
                                                    free(matriz3d.datos.matriz3d[i][j]);
                                                }
                                                free(matriz3d.datos.matriz3d[i]);
                                            }
                                            free(matriz3d.datos.matriz3d);
                                            return valor_crear_vacio();
                                        }
                                    }

                                    valor_destruir(&matriz3d.datos.matriz3d[plano_idx][fila_idx][col_idx]);
                                    matriz3d.datos.matriz3d[plano_idx][fila_idx][col_idx] = val;
                                    col_nodo = col_nodo->siguiente;
                                    col_idx++;
                                }
                            }
                            fila_nodo = fila_nodo->siguiente;
                            fila_idx++;
                        }
                    }
                    plano_nodo = plano_nodo->siguiente;
                    plano_idx++;
                }
            }

            // Registrar en la tabla de símbolos
            tabla_simbolos_definir(ctx->tabla_actual, nombre, matriz3d, false);
            return valor_crear_vacio();
        }

        case AST_DECLARACION_TEXTO_EXTENSO: {
            const char* nombre = nodo->datos.declaracion_texto_extenso.nombre;
            Valor valor = valor_crear_texto(
                nodo->datos.declaracion_texto_extenso.valor_inicial ? 
                nodo->datos.declaracion_texto_extenso.valor_inicial : ""
            );
            valor.tipo = VALOR_TEXTO_EXTENSO;
            tabla_simbolos_definir(ctx->tabla_actual, nombre, valor, false);
            return valor_crear_vacio();
        }
        
        // --------------------------------------------------------
        // ESTRUCTURAS DE CONTROL
        // --------------------------------------------------------
        case AST_SI: {
            Valor condicion = evaluar_nodo(nodo->datos.si.condicion, ctx);
            if (ctx->hay_error) return valor_crear_vacio();
            
            bool es_verdadero = false;
            if (condicion.tipo == VALOR_LOGICA) {
                es_verdadero = condicion.datos.logica;
            } else if (condicion.tipo == VALOR_ENTERO) {
                es_verdadero = condicion.datos.entero != 0;
            } else if (condicion.tipo == VALOR_DECIMAL) {
                es_verdadero = condicion.datos.decimal != 0.0;
            }

            if (es_verdadero) {
                return evaluar_bloque(nodo->datos.si.bloque_si, ctx);
            }
            
            // Probar cadenas SINO SI
            NodoAST* sino_si = nodo->datos.si.sino_si;
            while (sino_si) {
                Valor cond_sino_si = evaluar_nodo(sino_si->datos.sino_si.condicion, ctx);
                if (ctx->hay_error) return valor_crear_vacio();
                
                bool es_verd_sino = false;
                if (cond_sino_si.tipo == VALOR_LOGICA) {
                    es_verd_sino = cond_sino_si.datos.logica;
                } else if (cond_sino_si.tipo == VALOR_ENTERO) {
                    es_verd_sino = cond_sino_si.datos.entero != 0;
                } else if (cond_sino_si.tipo == VALOR_DECIMAL) {
                    es_verd_sino = cond_sino_si.datos.decimal != 0.0;
                }
                
                if (es_verd_sino) {
                    return evaluar_bloque(sino_si->datos.sino_si.bloque, ctx);
                }
                
                sino_si = sino_si->datos.sino_si.siguiente_sino_si;
            }
            
            // SINO final
            if (nodo->datos.si.bloque_sino) {
                return evaluar_bloque(nodo->datos.si.bloque_sino, ctx);
            }
            
            return valor_crear_vacio();
        }
        
        case AST_PARA: {
            const char* var = nodo->datos.para.variable;
            
            Valor inicio = evaluar_nodo(nodo->datos.para.inicio, ctx);
            if (ctx->hay_error) return valor_crear_vacio();
            
            Valor fin = evaluar_nodo(nodo->datos.para.fin, ctx);
            if (ctx->hay_error) return valor_crear_vacio();
            
            int paso = 1;
            if (nodo->datos.para.paso) {
                Valor v_paso = evaluar_nodo(nodo->datos.para.paso, ctx);
                if (ctx->hay_error) return valor_crear_vacio();
                if (v_paso.tipo == VALOR_ENTERO) paso = v_paso.datos.entero;
                else if (v_paso.tipo == VALOR_DECIMAL) paso = (int)v_paso.datos.decimal;
            }
            
            int val_inicio = (inicio.tipo == VALOR_ENTERO) ? inicio.datos.entero : (int)inicio.datos.decimal;
            int val_fin = (fin.tipo == VALOR_ENTERO) ? fin.datos.entero : (int)fin.datos.decimal;
            
            // Crear/actualizar variable de control en la tabla
            tabla_simbolos_definir(ctx->tabla_actual, var, valor_crear_entero(val_inicio), false);
            
            // Iterar
            if (paso > 0) {
                for (int i = val_inicio; i <= val_fin && ctx->estado_flujo == FLUJO_NORMAL; i += paso) {
                    tabla_simbolos_definir(ctx->tabla_actual, var, valor_crear_entero(i), false);
                    evaluar_bloque(nodo->datos.para.bloque, ctx);
                    
                    if (ctx->estado_flujo == FLUJO_BREAK) {
                        ctx->estado_flujo = FLUJO_NORMAL;
                        break;
                    }
                    if (ctx->estado_flujo == FLUJO_CONTINUE) {
                        ctx->estado_flujo = FLUJO_NORMAL;
                    }
                    if (ctx->estado_flujo == FLUJO_RETURN) {
                        return ctx->valor_retorno;
                    }
                    if (ctx->estado_flujo == FLUJO_SALTAR)
                    {
                        break;
                    }
                }
            } else if (paso < 0) {
                for (int i = val_inicio; i >= val_fin && ctx->estado_flujo == FLUJO_NORMAL; i += paso) {
                    tabla_simbolos_definir(ctx->tabla_actual, var, valor_crear_entero(i), false);
                    evaluar_bloque(nodo->datos.para.bloque, ctx);
                    
                    if (ctx->estado_flujo == FLUJO_BREAK) {
                        ctx->estado_flujo = FLUJO_NORMAL;
                        break;
                    }
                    if (ctx->estado_flujo == FLUJO_CONTINUE) {
                        ctx->estado_flujo = FLUJO_NORMAL;
                    }
                    if (ctx->estado_flujo == FLUJO_RETURN) {
                        return ctx->valor_retorno;
                    }
                    if (ctx->estado_flujo == FLUJO_SALTAR)
                    {
                        break;
                    }
                }
            }
            
            return valor_crear_vacio();
        }

        case AST_MIENTRAS:
        {
            while (ctx->estado_flujo == FLUJO_NORMAL)
            {

                teclado_limpiar_buffer();

                Valor condicion = evaluar_nodo(nodo->datos.mientras.condicion, ctx);
                if (ctx->hay_error)
                    return valor_crear_vacio();

                bool es_verdadero = false;
                if (condicion.tipo == VALOR_LOGICA)
                {
                    es_verdadero = condicion.datos.logica;
                }
                else if (condicion.tipo == VALOR_ENTERO)
                {
                    es_verdadero = condicion.datos.entero != 0;
                }
                else if (condicion.tipo == VALOR_DECIMAL)
                {
                    es_verdadero = condicion.datos.decimal != 0.0;
                }

                if (!es_verdadero)
                    break;

                evaluar_bloque(nodo->datos.mientras.bloque, ctx);

                if (ctx->estado_flujo == FLUJO_BREAK)
                {
                    ctx->estado_flujo = FLUJO_NORMAL;
                    break;
                }
                if (ctx->estado_flujo == FLUJO_CONTINUE)
                {
                    ctx->estado_flujo = FLUJO_NORMAL;
                }
                if (ctx->estado_flujo == FLUJO_RETURN)
                {
                    return ctx->valor_retorno;
                }
                // Si hay un salto pendiente, salir del bucle
                if (ctx->estado_flujo == FLUJO_SALTAR)
                {
                    break; // Salir y dejar que el bloque padre maneje el salto
                }
            }
            return valor_crear_vacio();
        }

        case AST_REALIZAR: {
            do {
                evaluar_bloque(nodo->datos.realizar.bloque, ctx);
                
                if (ctx->estado_flujo == FLUJO_BREAK) {
                    ctx->estado_flujo = FLUJO_NORMAL;
                    break;
                }
                if (ctx->estado_flujo == FLUJO_CONTINUE) {
                    ctx->estado_flujo = FLUJO_NORMAL;
                }
                if (ctx->estado_flujo == FLUJO_RETURN) {
                    return ctx->valor_retorno;
                }
                if (ctx->estado_flujo == FLUJO_SALTAR)
                {
                    break;
                }

                Valor condicion = evaluar_nodo(nodo->datos.realizar.condicion, ctx);
                if (ctx->hay_error) return valor_crear_vacio();
                
                bool es_verdadero = false;
                if (condicion.tipo == VALOR_LOGICA) {
                    es_verdadero = condicion.datos.logica;
                } else if (condicion.tipo == VALOR_ENTERO) {
                    es_verdadero = condicion.datos.entero != 0;
                } else if (condicion.tipo == VALOR_DECIMAL) {
                    es_verdadero = condicion.datos.decimal != 0.0;
                }
                
                if (!es_verdadero) break;
                
            } while (ctx->estado_flujo == FLUJO_NORMAL);
            
            return valor_crear_vacio();
        }

        case AST_SEGUN_CASO:
        {
            Valor expresion = evaluar_nodo(nodo->datos.segun_caso.expresion, ctx);
            if (ctx->hay_error)
                return valor_crear_vacio();

            int val_expr = 0;
            if (expresion.tipo == VALOR_ENTERO)
                val_expr = expresion.datos.entero;
            else if (expresion.tipo == VALOR_DECIMAL)
                val_expr = (int)expresion.datos.decimal;

            bool caso_encontrado = false;
            NodoAST *caso = nodo->datos.segun_caso.casos ? nodo->datos.segun_caso.casos->datos.bloque.primera : NULL;
            NodoAST *por_defecto = NULL;

            while (caso)
            {
                if (caso->tipo == AST_CASO)
                {
                    if (caso->datos.caso.valor == val_expr)
                    {
                        evaluar_bloque(caso->datos.caso.bloque, ctx);
                        caso_encontrado = true;
                        break;
                    }
                }
                else if (caso->tipo == AST_POR_DEFECTO)
                {
                    por_defecto = caso;
                }
                caso = caso->siguiente;
            }

            if (!caso_encontrado && por_defecto)
            {
                evaluar_bloque(por_defecto->datos.por_defecto.bloque, ctx);
            }

            if (ctx->estado_flujo == FLUJO_BREAK)
            {
                ctx->estado_flujo = FLUJO_NORMAL;
            }

            return valor_crear_vacio();
        }

        case AST_CORTE:
            ctx->estado_flujo = FLUJO_BREAK;
            return valor_crear_vacio();
        
        // --------------------------------------------------------
        // COMANDOS DE E/S
        // --------------------------------------------------------
        case AST_ESCRIBIR:
        {
            Valor expresion = evaluar_nodo(nodo->datos.escribir.expresion, ctx);
            if (ctx->hay_error)
                return valor_crear_vacio();

            if (expresion.tipo == VALOR_TEXTO)
            {
                const char *texto = expresion.datos.texto;
                size_t len = strlen(texto);
                int indice_formato = 0;

                for (size_t i = 0; i < len; i++)
                {
                    if (texto[i] == '$' && i + 1 < len && (isalpha(texto[i + 1]) || texto[i + 1] == '_'))
                    {
                        // Extraer nombre de variable
                        char nombre_var[256];
                        size_t j = 0;
                        i++; // Saltar el '$'

                        while (i < len && (isalnum(texto[i]) || texto[i] == '_') && j < 255)
                        {
                            nombre_var[j++] = texto[i++];
                        }
                        nombre_var[j] = '\0';
                        i--; // Retroceder uno

                        // Verificar si hay acceso directo a lista/matriz/matriz3D: $var[indice] o $var[i][j] o $var[i][j][k]
                        if (i + 1 < len && texto[i + 1] == '[')
                        {
                            // Primer índice
                            size_t inicio_corchete1 = i + 2;
                            size_t fin_corchete1 = inicio_corchete1;
                            int nivel = 1;

                            while (fin_corchete1 < len && nivel > 0)
                            {
                                if (texto[fin_corchete1] == '[')
                                    nivel++;
                                else if (texto[fin_corchete1] == ']')
                                    nivel--;
                                if (nivel > 0)
                                    fin_corchete1++;
                            }

                            if (nivel == 0 && fin_corchete1 < len)
                            {
                                char expr_indice1[256];
                                size_t expr_len1 = fin_corchete1 - inicio_corchete1;
                                if (expr_len1 < sizeof(expr_indice1))
                                {
                                    strncpy(expr_indice1, texto + inicio_corchete1, expr_len1);
                                    expr_indice1[expr_len1] = '\0';

                                    // Verificar segundo índice (matriz 2D)
                                    if (fin_corchete1 + 1 < len && texto[fin_corchete1 + 1] == '[')
                                    {
                                        size_t inicio_corchete2 = fin_corchete1 + 2;
                                        size_t fin_corchete2 = inicio_corchete2;
                                        nivel = 1;

                                        while (fin_corchete2 < len && nivel > 0)
                                        {
                                            if (texto[fin_corchete2] == '[')
                                                nivel++;
                                            else if (texto[fin_corchete2] == ']')
                                                nivel--;
                                            if (nivel > 0)
                                                fin_corchete2++;
                                        }

                                        if (nivel == 0 && fin_corchete2 < len)
                                        {
                                            char expr_indice2[256];
                                            size_t expr_len2 = fin_corchete2 - inicio_corchete2;
                                            if (expr_len2 < sizeof(expr_indice2))
                                            {
                                                strncpy(expr_indice2, texto + inicio_corchete2, expr_len2);
                                                expr_indice2[expr_len2] = '\0';

                                                // Verificar tercer índice (matriz 3D)
                                                if (fin_corchete2 + 1 < len && texto[fin_corchete2 + 1] == '[')
                                                {
                                                    size_t inicio_corchete3 = fin_corchete2 + 2;
                                                    size_t fin_corchete3 = inicio_corchete3;
                                                    nivel = 1;

                                                    while (fin_corchete3 < len && nivel > 0)
                                                    {
                                                        if (texto[fin_corchete3] == '[')
                                                            nivel++;
                                                        else if (texto[fin_corchete3] == ']')
                                                            nivel--;
                                                        if (nivel > 0)
                                                            fin_corchete3++;
                                                    }

                                                    if (nivel == 0 && fin_corchete3 < len)
                                                    {
                                                        char expr_indice3[256];
                                                        size_t expr_len3 = fin_corchete3 - inicio_corchete3;
                                                        if (expr_len3 < sizeof(expr_indice3))
                                                        {
                                                            strncpy(expr_indice3, texto + inicio_corchete3, expr_len3);
                                                            expr_indice3[expr_len3] = '\0';

                                                            // Evaluar los tres índices
                                                            char *indice1_str = evaluar_expresion_completa(expr_indice1, ctx);
                                                            char *indice2_str = evaluar_expresion_completa(expr_indice2, ctx);
                                                            char *indice3_str = evaluar_expresion_completa(expr_indice3, ctx);
                                                            long long idx1 = atoll(indice1_str);
                                                            long long idx2 = atoll(indice2_str);
                                                            long long idx3 = atoll(indice3_str);
                                                            free(indice1_str);
                                                            free(indice2_str);
                                                            free(indice3_str);

                                                            // Buscar matriz 3D
                                                            Valor *matriz3d = tabla_simbolos_buscar(ctx->tabla_actual, nombre_var);
                                                            if (matriz3d && matriz3d->tipo == VALOR_MATRIZ3D &&
                                                                idx1 >= 0 && idx1 < matriz3d->filas &&
                                                                idx2 >= 0 && idx2 < matriz3d->columnas &&
                                                                idx3 >= 0 && idx3 < matriz3d->profundidad)
                                                            {
                                                                Valor elemento = matriz3d->datos.matriz3d[idx1][idx2][idx3];
                                                                int decimales = -1;
                                                                if (indice_formato < nodo->datos.escribir.num_formatos)
                                                                {
                                                                    decimales = nodo->datos.escribir.formatos_decimales[indice_formato];
                                                                    indice_formato++;
                                                                }
                                                                if (decimales >= 0 && (elemento.tipo == VALOR_DECIMAL || elemento.tipo == VALOR_DECIMAL_SIN_SIGNO))
                                                                {
                                                                    printf("%.*f", decimales, elemento.datos.decimal);
                                                                }
                                                                else
                                                                {
                                                                    valor_imprimir(elemento);
                                                                }
                                                            }
                                                            else
                                                            {
                                                                printf("$%s[%s][%s][%s]", nombre_var, expr_indice1, expr_indice2, expr_indice3);
                                                            }
                                                            i = fin_corchete3;
                                                            continue;
                                                        }
                                                    }
                                                }
                                                /*else
                                                {
                                                    // Matriz 2D
                                                    char *indice1_str = evaluar_expresion_completa(expr_indice1, ctx);
                                                    char *indice2_str = evaluar_expresion_completa(expr_indice2, ctx);
                                                    long long idx1 = atoll(indice1_str);
                                                    long long idx2 = atoll(indice2_str);
                                                    free(indice1_str);
                                                    free(indice2_str);

                                                    Valor *matriz = tabla_simbolos_buscar(ctx->tabla_actual, nombre_var);
                                                    if (matriz && matriz->tipo == VALOR_MATRIZ &&
                                                        idx1 >= 0 && idx1 < matriz->filas &&
                                                        idx2 >= 0 && idx2 < matriz->columnas)
                                                    {
                                                        Valor elemento = matriz->datos.matriz[idx1][idx2];
                                                        int decimales = -1;
                                                        if (indice_formato < nodo->datos.escribir.num_formatos)
                                                        {
                                                            decimales = nodo->datos.escribir.formatos_decimales[indice_formato];
                                                            indice_formato++;
                                                        }
                                                        if (decimales >= 0 && (elemento.tipo == VALOR_DECIMAL || elemento.tipo == VALOR_DECIMAL_SIN_SIGNO))
                                                        {
                                                            printf("%.*f", decimales, elemento.datos.decimal);
                                                        }
                                                        else
                                                        {
                                                            valor_imprimir(elemento);
                                                        }
                                                    }
                                                    else
                                                    {
                                                        printf("$%s[%s][%s]", nombre_var, expr_indice1, expr_indice2);
                                                    }
                                                    i = fin_corchete2;
                                                    continue;
                                                }*/

                                                else
                                                {
                                                    // Matriz 2D
                                                    char *indice1_str = evaluar_expresion_completa(expr_indice1, ctx);
                                                    char *indice2_str = evaluar_expresion_completa(expr_indice2, ctx);
                                                    long long idx1 = atoll(indice1_str);
                                                    long long idx2 = atoll(indice2_str);
                                                    free(indice1_str);
                                                    free(indice2_str);

                                                    Valor *matriz = tabla_simbolos_buscar(ctx->tabla_actual, nombre_var);

                                                    if (matriz && matriz->tipo == VALOR_MATRIZ &&
                                                        idx1 >= 0 && idx1 < matriz->filas &&
                                                        idx2 >= 0 && idx2 < matriz->columnas)
                                                    {
                                                        Valor elemento = matriz->datos.matriz[idx1][idx2];
                                                        int decimales = -1;
                                                        if (indice_formato < nodo->datos.escribir.num_formatos)
                                                        {
                                                            decimales = nodo->datos.escribir.formatos_decimales[indice_formato];
                                                            indice_formato++;
                                                        }
                                                        if (decimales >= 0 && (elemento.tipo == VALOR_DECIMAL || elemento.tipo == VALOR_DECIMAL_SIN_SIGNO))
                                                        {
                                                            printf("%.*f", decimales, elemento.datos.decimal);
                                                        }
                                                        else
                                                        {
                                                            valor_imprimir(elemento);
                                                        }
                                                    }
                                                    else
                                                    {
                                                        printf("$%s[%s][%s]", nombre_var, expr_indice1, expr_indice2);
                                                    }
                                                    i = fin_corchete2;
                                                    continue;
                                                }
                                            }
                                        }
                                    }
                                    else
                                    {
                                        // Lista (un solo índice)
                                        char *indice_str = evaluar_expresion_completa(expr_indice1, ctx);
                                        long long idx = atoll(indice_str);
                                        free(indice_str);

                                        Valor *lista = tabla_simbolos_buscar(ctx->tabla_actual, nombre_var);
                                        if (lista && lista->tipo == VALOR_LISTA && idx >= 0 && idx < lista->tamano)
                                        {
                                            Valor elemento = lista->datos.lista[idx];
                                            int decimales = -1;
                                            if (indice_formato < nodo->datos.escribir.num_formatos)
                                            {
                                                decimales = nodo->datos.escribir.formatos_decimales[indice_formato];
                                                indice_formato++;
                                            }
                                            if (decimales >= 0 && (elemento.tipo == VALOR_DECIMAL || elemento.tipo == VALOR_DECIMAL_SIN_SIGNO))
                                            {
                                                printf("%.*f", decimales, elemento.datos.decimal);
                                            }
                                            else
                                            {
                                                valor_imprimir(elemento);
                                            }
                                        }
                                        else
                                        {
                                            printf("$%s[%s]", nombre_var, expr_indice1);
                                        }
                                        i = fin_corchete1;
                                        continue;
                                    }
                                }
                            }
                        }

                        // Si no hay acceso a lista, procesar como variable normal
                        Valor *valor_var = tabla_simbolos_buscar(ctx->tabla_actual, nombre_var);
                        if (valor_var)
                        {
                            int decimales = -1;
                            if (indice_formato < nodo->datos.escribir.num_formatos)
                            {
                                decimales = nodo->datos.escribir.formatos_decimales[indice_formato];
                                indice_formato++;
                            }
                            if (decimales >= 0 && (valor_var->tipo == VALOR_DECIMAL || valor_var->tipo == VALOR_DECIMAL_SIN_SIGNO))
                            {
                                printf("%.*f", decimales, valor_var->datos.decimal);
                            }
                            else
                            {
                                valor_imprimir(*valor_var);
                            }
                        }
                        else
                        {
                            printf("$%s", nombre_var);
                        }
                    }
                    else if (texto[i] == '[' && i + 1 < len && (i == 0 || texto[i - 1] != '\\'))
                    {
                        // Expresión entre corchetes - imprimir corchetes y evaluar contenido
                        char expr_str[1024];
                        char *resultado = NULL;
                        int decimales = -1;

                        printf("["); // Imprimir corchete de apertura

                        size_t inicio = i + 1;
                        size_t fin = inicio;
                        int nivel = 1;

                        // Encontrar el primer corchete de cierre
                        while (fin < len && nivel > 0)
                        {
                            if (texto[fin] == '[')
                                nivel++;
                            else if (texto[fin] == ']')
                                nivel--;
                            if (nivel > 0)
                                fin++;
                        }

                        if (nivel == 0 && fin < len)
                        {
                            size_t expr_len = fin - inicio;
                            if (expr_len < sizeof(expr_str))
                            {
                                strncpy(expr_str, texto + inicio, expr_len);
                                expr_str[expr_len] = '\0';

                                resultado = evaluar_expresion_completa(expr_str, ctx);

                                // Verificar si hay formato para esta expresión
                                if (indice_formato < nodo->datos.escribir.num_formatos)
                                {
                                    decimales = nodo->datos.escribir.formatos_decimales[indice_formato];
                                    indice_formato++;
                                }

                                // Imprimir resultado
                                if (decimales >= 0)
                                {
                                    double val = atof(resultado);
                                    printf("%.*f", decimales, val);
                                }
                                else
                                {
                                    printf("%s", resultado);
                                }
                                free(resultado);
                                resultado = NULL;
                            }

                            printf("]"); // Imprimir corchete de cierre
                            i = fin;

                            // Verificar si hay otro corchete inmediatamente después
                            if (i + 1 < len && texto[i + 1] == '[')
                            {
                                i++;         // Avanzar al siguiente '['
                                printf("["); // Imprimir corchete de apertura

                                // Repetir el proceso para el segundo corchete
                                inicio = i + 1;
                                fin = inicio;
                                nivel = 1;

                                while (fin < len && nivel > 0)
                                {
                                    if (texto[fin] == '[')
                                        nivel++;
                                    else if (texto[fin] == ']')
                                        nivel--;
                                    if (nivel > 0)
                                        fin++;
                                }

                                if (nivel == 0 && fin < len)
                                {
                                    expr_len = fin - inicio;
                                    if (expr_len < sizeof(expr_str))
                                    {
                                        strncpy(expr_str, texto + inicio, expr_len);
                                        expr_str[expr_len] = '\0';

                                        resultado = evaluar_expresion_completa(expr_str, ctx);

                                        // Verificar si hay formato
                                        decimales = -1;
                                        if (indice_formato < nodo->datos.escribir.num_formatos)
                                        {
                                            decimales = nodo->datos.escribir.formatos_decimales[indice_formato];
                                            indice_formato++;
                                        }

                                        if (decimales >= 0)
                                        {
                                            double val = atof(resultado);
                                            printf("%.*f", decimales, val);
                                        }
                                        else
                                        {
                                            printf("%s", resultado);
                                        }
                                        free(resultado);
                                    }

                                    printf("]"); // Imprimir corchete de cierre
                                    i = fin;
                                }
                            }
                        }
                        else
                        {
                            printf("%c", texto[i]);
                        }
                    }
                    else
                    {
                        // Verificar si es un marcador de carácter escapado
                        if (texto[i] == '\x01')
                        {
                            printf("["); // Convertir marcador a corchete literal
                        }
                        else if (texto[i] == '\x02')
                        {
                            printf("]"); // Convertir marcador a corchete literal
                        }
                        else if (texto[i] == '\x03')
                        {
                            printf("$"); // Convertir marcador a dólar literal
                        }
                        else
                        {
                            printf("%c", texto[i]);
                        }
                    }
                }
            }
            else
            {
                valor_imprimir(expresion);
            }

            if (nodo->datos.escribir.salto_linea)
            {
                printf("\n");
            }

            fflush(stdout);
            valor_destruir(&expresion);
            return valor_crear_vacio();
        }
        case AST_LIMPIARPANTALLA:
            printf("\033[2J\033[H");
            fflush(stdout);
            return valor_crear_vacio();
        
        // --------------------------------------------------------
        // COMANDOS DE CÁLCULO
        // --------------------------------------------------------
        case AST_CALCULAR:
        case AST_ASIGNAR:
        {
            NodoAST *destino = (nodo->tipo == AST_CALCULAR) ? nodo->datos.calcular.destino : nodo->datos.asignar.destino;
            NodoAST *expr = (nodo->tipo == AST_CALCULAR) ? nodo->datos.calcular.expresion : nodo->datos.asignar.valor;

            Valor valor = evaluar_nodo(expr, ctx);
            if (ctx->hay_error)
                return valor_crear_vacio();

            // Verificar el tipo de destino
            if (destino->tipo == AST_VARIABLE)
            {
                // Asignación a variable simple
                const char *nombre = destino->datos.variable.nombre;
                Simbolo *sim = tabla_simbolos_buscar_simbolo(ctx->tabla_actual, nombre);
                if (!sim)
                {
                    char msg[256];
                    snprintf(msg, sizeof(msg),
                             "Variable '$%s' no declarada. Debe declararse antes de usarla",
                             nombre);
                    contexto_set_error(ctx, msg);
                    valor_destruir(&valor);
                    return valor_crear_vacio();
                }
                if (!tipo_valor_compatible(sim->tipo_declarado, valor.tipo))
                {
                    char msg[256];
                    snprintf(msg, sizeof(msg),
                             "No puede asignarse un valor de tipo incompatible a la variable '$%s'.",
                             nombre);
                    contexto_set_error(ctx, msg);
                    valor_destruir(&valor);
                    return valor_crear_vacio();
                }
                // VALIDAR QUE NO SE ASIGNEN VALORES NEGATIVOS A VARIABLES SIN SIGNO
                if (sim->tipo_declarado == TIPO_ENTERO_SIN_SIGNO)
                {
                    if (valor.tipo == VALOR_ENTERO && valor.datos.entero < 0)
                    {
                        char msg[256];
                        snprintf(msg, sizeof(msg),
                                 "La variable '$%s' es ENTERA SIN SIGNO y no admite valores negativos.",
                                 nombre);
                        contexto_set_error(ctx, msg);
                        valor_destruir(&valor);
                        return valor_crear_vacio();
                    }
                    if (valor.tipo == VALOR_DECIMAL && valor.datos.decimal < 0.0)
                    {
                        char msg[256];
                        snprintf(msg, sizeof(msg),
                                 "La variable '$%s' es ENTERA SIN SIGNO y no admite valores negativos.",
                                 nombre);
                        contexto_set_error(ctx, msg);
                        valor_destruir(&valor);
                        return valor_crear_vacio();
                    }
                }
                else if (sim->tipo_declarado == TIPO_DECIMAL_SIN_SIGNO)
                {
                    if (valor.tipo == VALOR_DECIMAL && valor.datos.decimal < 0.0)
                    {
                        char msg[256];
                        snprintf(msg, sizeof(msg),
                                 "La variable '$%s' es DECIMAL SIN SIGNO y no admite valores negativos.",
                                 nombre);
                        contexto_set_error(ctx, msg);
                        valor_destruir(&valor);
                        return valor_crear_vacio();
                    }
                    if (valor.tipo == VALOR_ENTERO && valor.datos.entero < 0)
                    {
                        char msg[256];
                        snprintf(msg, sizeof(msg),
                                 "La variable '$%s' es DECIMAL SIN SIGNO y no admite valores negativos.",
                                 nombre);
                        contexto_set_error(ctx, msg);
                        valor_destruir(&valor);
                        return valor_crear_vacio();
                    }
                }

                // ==========================================
                // CONVERSIÓN EXPLÍCITA DE TIPOS
                // ==========================================
                if (sim->tipo_declarado == TIPO_ENTERO || sim->tipo_declarado == TIPO_ENTERO_SIN_SIGNO)
                {
                    if (valor.tipo == VALOR_DECIMAL)
                    {
                        valor.datos.entero = (long long)valor.datos.decimal;
                        valor.tipo = VALOR_ENTERO;
                    }
                    else if (valor.tipo == VALOR_DECIMAL_SIN_SIGNO)
                    {
                        valor.datos.entero = (long long)valor.datos.decimal_sin_signo;
                        valor.tipo = VALOR_ENTERO;
                    }
                }
                else if (sim->tipo_declarado == TIPO_DECIMAL || sim->tipo_declarado == TIPO_DECIMAL_SIN_SIGNO)
                {
                    if (valor.tipo == VALOR_ENTERO)
                    {
                        valor.datos.decimal = (double)valor.datos.entero;
                        valor.tipo = VALOR_DECIMAL;
                    }
                    else if (valor.tipo == VALOR_ENTERO_SIN_SIGNO)
                    {
                        valor.datos.decimal = (double)valor.datos.entero_sin_signo;
                        valor.tipo = VALOR_DECIMAL;
                    }
                }

                tabla_simbolos_asignar(ctx->tabla_actual, nombre, valor);
            }
            else if (destino->tipo == AST_ACCESO_LISTA)
            {
                // Asignación a elemento de lista
                const char *nombre_lista = destino->datos.acceso_lista.nombre_lista;
                NodoAST *indice_nodo = destino->datos.acceso_lista.indice;

                Valor indice = evaluar_nodo(indice_nodo, ctx);
                if (ctx->hay_error)
                {
                    valor_destruir(&valor);
                    return valor_crear_vacio();
                }

                // Aceptar ENTERO, ENTERO_SIN_SIGNO, DECIMAL y DECIMAL_SIN_SIGNO como índice
                long long idx = 0;
                if (indice.tipo == VALOR_ENTERO)
                {
                    idx = indice.datos.entero;
                }
                else if (indice.tipo == VALOR_ENTERO_SIN_SIGNO)
                {
                    idx = (int)indice.datos.entero_sin_signo;
                }
                else if (indice.tipo == VALOR_DECIMAL)
                {
                    idx = (int)indice.datos.decimal;
                }
                else if (indice.tipo == VALOR_DECIMAL_SIN_SIGNO)
                {
                    idx = (int)indice.datos.decimal_sin_signo;
                }
                else
                {
                    contexto_set_error(ctx, "Índice de lista debe ser numérico");
                    valor_destruir(&valor);
                    valor_destruir(&indice);
                    return valor_crear_vacio();
                }

                // Buscar la lista en la tabla de símbolos
                Valor *lista = tabla_simbolos_buscar(ctx->tabla_actual, nombre_lista);
                if (!lista || lista->tipo != VALOR_LISTA)
                {
                    contexto_set_error(ctx, "Lista no encontrada");
                    valor_destruir(&valor);
                    return valor_crear_vacio();
                }

                // Verificar bounds
                if (idx < 0 || idx >= lista->tamano)
                {
                    contexto_set_error(ctx, "Índice fuera de rango");
                    valor_destruir(&valor);
                    return valor_crear_vacio();
                }

                // VALIDAR COMPATIBILIDAD DE TIPOS
                TipoDato tipo_elem = lista->tipo_elemento;

                // Validar signo negativo para tipos SIN SIGNO
                if (tipo_elem == TIPO_ENTERO_SIN_SIGNO || tipo_elem == TIPO_DECIMAL_SIN_SIGNO)
                {
                    if (valor.tipo == VALOR_ENTERO && valor.datos.entero < 0)
                    {
                        char msg[256];
                        snprintf(msg, sizeof(msg),
                                 "La lista '%s' es SIN SIGNO y no admite valores negativos en la posición %lld.",
                                 nombre_lista, idx);
                        contexto_set_error(ctx, msg);
                        valor_destruir(&valor);
                        return valor_crear_vacio();
                    }
                    if (valor.tipo == VALOR_DECIMAL && valor.datos.decimal < 0.0)
                    {
                        char msg[256];
                        snprintf(msg, sizeof(msg),
                                 "La lista '%s' es SIN SIGNO y no admite valores negativos en la posición %lld.",
                                 nombre_lista, idx);
                        contexto_set_error(ctx, msg);
                        valor_destruir(&valor);
                        return valor_crear_vacio();
                    }
                }

                // CONVERSIÓN EXPLÍCITA DE TIPOS
                if (tipo_elem == TIPO_ENTERO || tipo_elem == TIPO_ENTERO_SIN_SIGNO)
                {
                    if (valor.tipo == VALOR_DECIMAL)
                    {
                        valor.datos.entero = (long long)valor.datos.decimal;
                        valor.tipo = VALOR_ENTERO;
                    }
                    else if (valor.tipo == VALOR_DECIMAL_SIN_SIGNO)
                    {
                        valor.datos.entero = (long long)valor.datos.decimal_sin_signo;
                        valor.tipo = VALOR_ENTERO;
                    }
                }
                else if (tipo_elem == TIPO_DECIMAL || tipo_elem == TIPO_DECIMAL_SIN_SIGNO)
                {
                    if (valor.tipo == VALOR_ENTERO)
                    {
                        valor.datos.decimal = (double)valor.datos.entero;
                        valor.tipo = VALOR_DECIMAL;
                    }
                    else if (valor.tipo == VALOR_ENTERO_SIN_SIGNO)
                    {
                        valor.datos.decimal = (double)valor.datos.entero_sin_signo;
                        valor.tipo = VALOR_DECIMAL;
                    }
                }

                // Asignar el valor a la lista
                valor_destruir(&lista->datos.lista[idx]);
                lista->datos.lista[idx] = valor;

                valor_destruir(&indice);
            }

            else if (destino->tipo == AST_ACCESO_MATRIZ)
            {
                // Asignación a elemento de matriz
                const char *nombre_matriz = destino->datos.acceso_matriz.nombre_matriz;
                NodoAST *fila_nodo = destino->datos.acceso_matriz.fila;
                NodoAST *col_nodo = destino->datos.acceso_matriz.columna;

                // Evaluar índices
                Valor fila_v = evaluar_nodo(fila_nodo, ctx);
                if (ctx->hay_error)
                {
                    valor_destruir(&valor);
                    return valor_crear_vacio();
                }

                Valor col_v = evaluar_nodo(col_nodo, ctx);
                if (ctx->hay_error)
                {
                    valor_destruir(&fila_v);
                    valor_destruir(&valor);
                    return valor_crear_vacio();
                }

                // Convertir a int
                int fila = 0, col = 0;
                if (fila_v.tipo == VALOR_ENTERO)
                    fila = fila_v.datos.entero;
                else if (fila_v.tipo == VALOR_ENTERO_SIN_SIGNO)
                    fila = (int)fila_v.datos.entero_sin_signo;
                else if (fila_v.tipo == VALOR_DECIMAL)
                    fila = (int)fila_v.datos.decimal;
                else if (fila_v.tipo == VALOR_DECIMAL_SIN_SIGNO)
                    fila = (int)fila_v.datos.decimal_sin_signo;

                if (col_v.tipo == VALOR_ENTERO)
                    col = col_v.datos.entero;
                else if (col_v.tipo == VALOR_ENTERO_SIN_SIGNO)
                    col = (int)col_v.datos.entero_sin_signo;
                else if (col_v.tipo == VALOR_DECIMAL)
                    col = (int)col_v.datos.decimal;
                else if (col_v.tipo == VALOR_DECIMAL_SIN_SIGNO)
                    col = (int)col_v.datos.decimal_sin_signo;

                valor_destruir(&fila_v);
                valor_destruir(&col_v);

                // Buscar la matriz
                Valor *matriz = tabla_simbolos_buscar(ctx->tabla_actual, nombre_matriz);
                if (!matriz || matriz->tipo != VALOR_MATRIZ)
                {
                    contexto_set_error(ctx, "Matriz no encontrada");
                    valor_destruir(&valor);
                    return valor_crear_vacio();
                }

                // Verificar bounds
                if (fila < 0 || fila >= matriz->filas || col < 0 || col >= matriz->columnas)
                {
                    char msg[128];
                    snprintf(msg, sizeof(msg), "Índice [%d][%d] fuera de rango", fila, col);
                    contexto_set_error(ctx, msg);
                    valor_destruir(&valor);
                    return valor_crear_vacio();
                }

                // VALIDAR COMPATIBILIDAD DE TIPOS
                TipoDato tipo_elem = matriz->tipo_elemento;

                // Validar signo negativo para tipos SIN SIGNO
                if (tipo_elem == TIPO_ENTERO_SIN_SIGNO || tipo_elem == TIPO_DECIMAL_SIN_SIGNO)
                {
                    if (valor.tipo == VALOR_ENTERO && valor.datos.entero < 0)
                    {
                        char msg[256];
                        snprintf(msg, sizeof(msg),
                                 "La matriz '%s' es SIN SIGNO y no admite valores negativos en [%d][%d].",
                                 nombre_matriz, fila, col);
                        contexto_set_error(ctx, msg);
                        valor_destruir(&valor);
                        return valor_crear_vacio();
                    }
                    if (valor.tipo == VALOR_DECIMAL && valor.datos.decimal < 0.0)
                    {
                        char msg[256];
                        snprintf(msg, sizeof(msg),
                                 "La matriz '%s' es SIN SIGNO y no admite valores negativos en [%d][%d].",
                                 nombre_matriz, fila, col);
                        contexto_set_error(ctx, msg);
                        valor_destruir(&valor);
                        return valor_crear_vacio();
                    }
                }

                // CONVERSIÓN EXPLÍCITA DE TIPOS
                if (tipo_elem == TIPO_ENTERO || tipo_elem == TIPO_ENTERO_SIN_SIGNO)
                {
                    if (valor.tipo == VALOR_DECIMAL)
                    {
                        valor.datos.entero = (long long)valor.datos.decimal;
                        valor.tipo = VALOR_ENTERO;
                    }
                    else if (valor.tipo == VALOR_DECIMAL_SIN_SIGNO)
                    {
                        valor.datos.entero = (long long)valor.datos.decimal_sin_signo;
                        valor.tipo = VALOR_ENTERO;
                    }
                }
                else if (tipo_elem == TIPO_DECIMAL || tipo_elem == TIPO_DECIMAL_SIN_SIGNO)
                {
                    if (valor.tipo == VALOR_ENTERO)
                    {
                        valor.datos.decimal = (double)valor.datos.entero;
                        valor.tipo = VALOR_DECIMAL;
                    }
                    else if (valor.tipo == VALOR_ENTERO_SIN_SIGNO)
                    {
                        valor.datos.decimal = (double)valor.datos.entero_sin_signo;
                        valor.tipo = VALOR_DECIMAL;
                    }
                }

                // Asignar el valor
                valor_destruir(&matriz->datos.matriz[fila][col]);
                matriz->datos.matriz[fila][col] = valor;
            }

            else if (destino->tipo == AST_ACCESO_MATRIZ3D)
            {
                // Asignación a elemento de matriz 3D
                const char *nombre_matriz = destino->datos.acceso_matriz3d.nombre_matriz;
                NodoAST *indice1_nodo = destino->datos.acceso_matriz3d.indice1;
                NodoAST *indice2_nodo = destino->datos.acceso_matriz3d.indice2;
                NodoAST *indice3_nodo = destino->datos.acceso_matriz3d.indice3;

                // Evaluar índices
                Valor indice1_v = evaluar_nodo(indice1_nodo, ctx);
                if (ctx->hay_error)
                {
                    valor_destruir(&valor);
                    return valor_crear_vacio();
                }

                Valor indice2_v = evaluar_nodo(indice2_nodo, ctx);
                if (ctx->hay_error)
                {
                    valor_destruir(&indice1_v);
                    valor_destruir(&valor);
                    return valor_crear_vacio();
                }

                Valor indice3_v = evaluar_nodo(indice3_nodo, ctx);
                if (ctx->hay_error)
                {
                    valor_destruir(&indice1_v);
                    valor_destruir(&indice2_v);
                    valor_destruir(&valor);
                    return valor_crear_vacio();
                }

                // Convertir a int
                int idx1 = 0, idx2 = 0, idx3 = 0;

                if (indice1_v.tipo == VALOR_ENTERO)
                    idx1 = indice1_v.datos.entero;
                else if (indice1_v.tipo == VALOR_ENTERO_SIN_SIGNO)
                    idx1 = (int)indice1_v.datos.entero_sin_signo;
                else if (indice1_v.tipo == VALOR_DECIMAL)
                    idx1 = (int)indice1_v.datos.decimal;
                else if (indice1_v.tipo == VALOR_DECIMAL_SIN_SIGNO)
                    idx1 = (int)indice1_v.datos.decimal_sin_signo;

                if (indice2_v.tipo == VALOR_ENTERO)
                    idx2 = indice2_v.datos.entero;
                else if (indice2_v.tipo == VALOR_ENTERO_SIN_SIGNO)
                    idx2 = (int)indice2_v.datos.entero_sin_signo;
                else if (indice2_v.tipo == VALOR_DECIMAL)
                    idx2 = (int)indice2_v.datos.decimal;
                else if (indice2_v.tipo == VALOR_DECIMAL_SIN_SIGNO)
                    idx2 = (int)indice2_v.datos.decimal_sin_signo;

                if (indice3_v.tipo == VALOR_ENTERO)
                    idx3 = indice3_v.datos.entero;
                else if (indice3_v.tipo == VALOR_ENTERO_SIN_SIGNO)
                    idx3 = (int)indice3_v.datos.entero_sin_signo;
                else if (indice3_v.tipo == VALOR_DECIMAL)
                    idx3 = (int)indice3_v.datos.decimal;
                else if (indice3_v.tipo == VALOR_DECIMAL_SIN_SIGNO)
                    idx3 = (int)indice3_v.datos.decimal_sin_signo;

                valor_destruir(&indice1_v);
                valor_destruir(&indice2_v);
                valor_destruir(&indice3_v);

                // Buscar la matriz 3D
                Valor *matriz3d = tabla_simbolos_buscar(ctx->tabla_actual, nombre_matriz);
                if (!matriz3d || matriz3d->tipo != VALOR_MATRIZ3D)
                {
                    contexto_set_error(ctx, "Matriz 3D no encontrada");
                    valor_destruir(&valor);
                    return valor_crear_vacio();
                }

                // Verificar bounds
                if (idx1 < 0 || idx1 >= matriz3d->filas ||
                    idx2 < 0 || idx2 >= matriz3d->columnas ||
                    idx3 < 0 || idx3 >= matriz3d->profundidad)
                {
                    char msg[128];
                    snprintf(msg, sizeof(msg), "Índice [%d][%d][%d] fuera de rango", idx1, idx2, idx3);
                    contexto_set_error(ctx, msg);
                    valor_destruir(&valor);
                    return valor_crear_vacio();
                }

                // VALIDAR COMPATIBILIDAD DE TIPOS
                TipoDato tipo_elem = matriz3d->tipo_elemento;

                // Validar signo negativo para tipos SIN SIGNO
                if (tipo_elem == TIPO_ENTERO_SIN_SIGNO || tipo_elem == TIPO_DECIMAL_SIN_SIGNO)
                {
                    if (valor.tipo == VALOR_ENTERO && valor.datos.entero < 0)
                    {
                        char msg[256];
                        snprintf(msg, sizeof(msg),
                                 "La matriz 3D '%s' es SIN SIGNO y no admite valores negativos en [%d][%d][%d].",
                                 nombre_matriz, idx1, idx2, idx3);
                        contexto_set_error(ctx, msg);
                        valor_destruir(&valor);
                        return valor_crear_vacio();
                    }
                    if (valor.tipo == VALOR_DECIMAL && valor.datos.decimal < 0.0)
                    {
                        char msg[256];
                        snprintf(msg, sizeof(msg),
                                 "La matriz 3D '%s' es SIN SIGNO y no admite valores negativos en [%d][%d][%d].",
                                 nombre_matriz, idx1, idx2, idx3);
                        contexto_set_error(ctx, msg);
                        valor_destruir(&valor);
                        return valor_crear_vacio();
                    }
                }

                // CONVERSIÓN EXPLÍCITA DE TIPOS
                if (tipo_elem == TIPO_ENTERO || tipo_elem == TIPO_ENTERO_SIN_SIGNO)
                {
                    if (valor.tipo == VALOR_DECIMAL)
                    {
                        valor.datos.entero = (long long)valor.datos.decimal;
                        valor.tipo = VALOR_ENTERO;
                    }
                    else if (valor.tipo == VALOR_DECIMAL_SIN_SIGNO)
                    {
                        valor.datos.entero = (long long)valor.datos.decimal_sin_signo;
                        valor.tipo = VALOR_ENTERO;
                    }
                }
                else if (tipo_elem == TIPO_DECIMAL || tipo_elem == TIPO_DECIMAL_SIN_SIGNO)
                {
                    if (valor.tipo == VALOR_ENTERO)
                    {
                        valor.datos.decimal = (double)valor.datos.entero;
                        valor.tipo = VALOR_DECIMAL;
                    }
                    else if (valor.tipo == VALOR_ENTERO_SIN_SIGNO)
                    {
                        valor.datos.decimal = (double)valor.datos.entero_sin_signo;
                        valor.tipo = VALOR_DECIMAL;
                    }
                }

                // Asignar el valor
                valor_destruir(&matriz3d->datos.matriz3d[idx1][idx2][idx3]);
                matriz3d->datos.matriz3d[idx1][idx2][idx3] = valor;
            }

            return valor_crear_vacio();
        }
        case AST_RESULTADO: {
            Valor valor = evaluar_nodo(nodo->datos.resultado.expresion, ctx);
            if (ctx->hay_error) return valor_crear_vacio();
            
            valor_imprimir(valor);
            printf("\n");
            fflush(stdout);
            valor_destruir(&valor);
            return valor_crear_vacio();
        }
        
        // --------------------------------------------------------
        // COMANDOS DE TIEMPO
        // --------------------------------------------------------
        case AST_HORAACTUAL:
        case AST_FECHAACTUAL: {
            const char* nombre = (nodo->tipo == AST_HORAACTUAL) ? 
                         nodo->datos.horaactual.variable : 
                         nodo->datos.fechaactual.variable;
    
            time_t ahora = time(NULL);
            struct tm* tiempo_local = localtime(&ahora);
            char buffer[64];
    
            if (nodo->tipo == AST_HORAACTUAL) {
                strftime(buffer, sizeof(buffer), "%H:%M:%S", tiempo_local);
            } else {
                strftime(buffer, sizeof(buffer), "%d/%m/%Y", tiempo_local);
            }
    
            tabla_simbolos_definir(ctx->tabla_actual, nombre, 
                           valor_crear_texto(buffer), false);
            return valor_crear_vacio();
        }
                
        // --------------------------------------------------------
        // FUNCIONES Y SUBPROGRAMAS
        // --------------------------------------------------------
        case AST_FUNCION:
        {
            // Crear definición de función
            DefinicionFuncion *func = malloc(sizeof(DefinicionFuncion));
            if (!func)
            {
                contexto_set_error(ctx, "No se pudo asignar memoria para función");
                return valor_crear_vacio();
            }

            func->nombre = strdup(nodo->datos.funcion.nombre);
            func->tipo_retorno = nodo->datos.funcion.tipo_retorno;
            func->es_subprograma = false;
            func->num_parametros = nodo->datos.funcion.num_parametros;
            func->bloque = nodo->datos.funcion.bloque;
            func->siguiente = NULL;

            // Copiar parámetros
            if (func->num_parametros > 0)
            {
                func->parametros = malloc(sizeof(ParametroFuncion) * func->num_parametros);
                for (int i = 0; i < func->num_parametros; i++)
                {
                    func->parametros[i].nombre = strdup(nodo->datos.funcion.parametros[i]);
                    // El tipo se puede obtener del AST si está disponible, por ahora usamos TEXTO
                    func->parametros[i].tipo = TIPO_TEXTO;
                }
            }
            else
            {
                func->parametros = NULL;
            }

            // Registrar en el contexto
            contexto_registrar_funcion(ctx, func);
            return valor_crear_vacio();
        }

        case AST_SUBPROGRAMA:
        {
            // Crear definición de subprograma
            DefinicionFuncion *func = malloc(sizeof(DefinicionFuncion));
            if (!func)
            {
                contexto_set_error(ctx, "No se pudo asignar memoria para subprograma");
                return valor_crear_vacio();
            }

            func->nombre = strdup(nodo->datos.subprograma.nombre);
            func->tipo_retorno = TIPO_VACIO;
            func->es_subprograma = true;
            func->num_parametros = nodo->datos.subprograma.num_parametros;
            func->bloque = nodo->datos.subprograma.bloque;
            func->siguiente = NULL;

            // Copiar parámetros
            if (func->num_parametros > 0)
            {
                func->parametros = malloc(sizeof(ParametroFuncion) * func->num_parametros);
                for (int i = 0; i < func->num_parametros; i++)
                {
                    func->parametros[i].nombre = strdup(nodo->datos.subprograma.parametros[i]);
                    func->parametros[i].tipo = TIPO_TEXTO;
                }
            }
            else
            {
                func->parametros = NULL;
            }

            // Registrar en el contexto
            contexto_registrar_funcion(ctx, func);
            return valor_crear_vacio();
        }
        case AST_LLAMAR_A:
        {
            const char *nombre = nodo->datos.llamar_a.nombre;
            // Recolectar argumentos
            Valor args[20];
            int num_args = 0;
            NodoAST *arg = nodo->datos.llamar_a.argumentos ? nodo->datos.llamar_a.argumentos->datos.bloque.primera : NULL;

            while (arg && num_args < 20)
            {
                args[num_args++] = evaluar_nodo(arg, ctx);
                if (ctx->hay_error)
                    return valor_crear_vacio();
                arg = arg->siguiente;
            }

            // Buscar función o subprograma
            DefinicionFuncion *sub = contexto_buscar_funcion(ctx, nombre);

            if (!sub)
            {
                char msg[256];
                snprintf(msg, sizeof(msg), "Función o subprograma '%s' no encontrado", nombre);
                contexto_set_error(ctx, msg);
                return valor_crear_vacio();
            }

            if (!sub || (!sub->es_subprograma && sub->tipo_retorno != TIPO_VACIO))
            {
                char msg[256];
                snprintf(msg, sizeof(msg), "No se puede usar LLAMAR A con '%s' porque retorna un valor. Use ASIGNAR EN en su lugar.", nombre);
                contexto_set_error(ctx, msg);
                return valor_crear_vacio();
            }

            // Verificar número de parámetros
            if (sub->num_parametros != num_args)
            {
                char msg[256];
                snprintf(msg, sizeof(msg),
                         "Subprograma '%s' espera %d parámetros, se pasaron %d",
                         nombre, sub->num_parametros, num_args);
                contexto_set_error(ctx, msg);
                return valor_crear_vacio();
            }

            // Proteger contra recursión infinita
            if (ctx->profundidad_llamada > 100)
            {
                contexto_set_error(ctx, "Recursión demasiado profunda");
                return valor_crear_vacio();
            }

            // Guardar contexto
            TablaSimbolos *tabla_anterior = ctx->tabla_actual;
            EstadoFlujo estado_anterior = ctx->estado_flujo;
            Valor retorno_anterior = ctx->valor_retorno;

            // Crear nuevo scope
            TablaSimbolos *tabla_local = malloc(sizeof(TablaSimbolos));
            tabla_local->primera = NULL;
            tabla_local->padre = ctx->tabla_global;
            ctx->tabla_actual = tabla_local;
            ctx->estado_flujo = FLUJO_NORMAL;
            ctx->valor_retorno = valor_crear_vacio();
            ctx->profundidad_llamada++;

            // Asignar argumentos (con copia profunda)
            for (int i = 0; i < num_args; i++)
            {
                Valor copia;
                if (args[i].tipo == VALOR_TEXTO || args[i].tipo == VALOR_TEXTO_EXTENSO)
                {
                    copia = valor_crear_texto(args[i].datos.texto ? args[i].datos.texto : "");
                }
                else
                {
                    copia = args[i];
                }
                tabla_simbolos_definir(ctx->tabla_actual, sub->parametros[i].nombre, copia, false);
            }

            // Ejecutar cuerpo
            if (sub->bloque)
            {
                evaluar_bloque(sub->bloque, ctx);
            }

            // Validar que subprogramas no retornen valores
            if (sub->es_subprograma && ctx->valor_retorno.tipo != VALOR_VACIO)
            {
                char msg[256];
                snprintf(msg, sizeof(msg),
                         "El subprograma '%s' no debe retornar un valor pero se ejecutó RETORNAR con un valor.",
                         nombre);
                contexto_set_error(ctx, msg);
                ctx->profundidad_llamada--;
                tabla_simbolos_destruir(tabla_local);
                ctx->tabla_actual = tabla_anterior;
                ctx->estado_flujo = estado_anterior;
                ctx->valor_retorno = retorno_anterior;
                return valor_crear_vacio();
            }

            // Destruir argumentos
            for (int i = 0; i < num_args; i++)
            {
                valor_destruir(&args[i]);
            }

            return valor_crear_vacio();
        }
        case AST_RETORNAR: {
            if (nodo->datos.retornar.valor) {
                ctx->valor_retorno = evaluar_nodo(nodo->datos.retornar.valor, ctx);
            } else {
                ctx->valor_retorno = valor_crear_vacio();
            }
            ctx->estado_flujo = FLUJO_RETURN;
            return ctx->valor_retorno;
        }
        
        // --------------------------------------------------------
        // PROGRAMA Y BLOQUE PRINCIPAL
        // --------------------------------------------------------
        case AST_PROGRAMA:
        {
            // Guardar el nombre del programa
            if (nodo->datos.programa.nombre)
            {
                ctx->nombre_programa = strdup(nodo->datos.programa.nombre);
            }

            // 1. Primero registrar todas las funciones y subprogramas
            if (nodo->datos.programa.funciones)
            {
                evaluar_bloque(nodo->datos.programa.funciones, ctx);
                if (ctx->hay_error)
                    return valor_crear_vacio();
            }
            if (nodo->datos.programa.subprogramas)
            {
                evaluar_bloque(nodo->datos.programa.subprogramas, ctx);
                if (ctx->hay_error)
                    return valor_crear_vacio();
            }

            // 2. Evaluar declaraciones globales
            if (nodo->datos.programa.declaraciones)
            {
                evaluar_bloque(nodo->datos.programa.declaraciones, ctx);
                if (ctx->hay_error)
                    return valor_crear_vacio();
            }

            // 3. Evaluar bloque principal
            if (nodo->datos.programa.bloque_principal)
            {
                evaluar_bloque(nodo->datos.programa.bloque_principal, ctx);
            }

            return valor_crear_vacio();
        }
        case AST_BLOQUE_PRINCIPAL:
            return evaluar_bloque(nodo->datos.bloque_principal.sentencias, ctx);
        
        // --------------------------------------------------------
        // INCLUIR
        // --------------------------------------------------------
        case AST_INCLUIR:
        {
            const char *ruta = nodo->datos.incluir.ruta_archivo;

            // Verificar si ya se incluyó este archivo (evitar inclusión circular)
            static char *archivos_incluidos[100];
            static int num_incluidos = 0;

            for (int i = 0; i < num_incluidos; i++)
            {
                if (strcmp(archivos_incluidos[i], ruta) == 0)
                {
                    return valor_crear_vacio();
                }
            }

            // Registrar este archivo
            if (num_incluidos < 100)
            {
                archivos_incluidos[num_incluidos++] = strdup(ruta);
            }

            // Leer el archivo
            FILE *archivo = fopen(ruta, "r");
            if (!archivo)
            {
                contexto_set_error(ctx, "No se pudo abrir el archivo incluido");
                return valor_crear_vacio();
            }

            // Leer contenido completo
            fseek(archivo, 0, SEEK_END);
            long tamano = ftell(archivo);
            fseek(archivo, 0, SEEK_SET);

            char *contenido = malloc(tamano + 1);
            fread(contenido, 1, tamano, archivo);
            contenido[tamano] = '\0';
            fclose(archivo);

            // Parsear el archivo incluido
            Lexer *lexer_inc = lexer_crear(contenido);
            Parser *parser_inc = parser_crear(lexer_inc);
            NodoAST *ast_inc = parser_parsear(parser_inc);

            if (parser_tiene_error(parser_inc))
            {
                contexto_set_error(ctx, parser_obtener_error(parser_inc));
                free(contenido);
                lexer_destruir(lexer_inc);
                parser_destruir(parser_inc);
                return valor_crear_vacio();
            }

            // Procesar el AST incluido: solo funciones, constantes y variables globales
            if (ast_inc && ast_inc->tipo == AST_PROGRAMA)
            {
                // Procesar funciones (están dentro de un bloque)
                NodoAST *func_bloque = ast_inc->datos.programa.funciones;
                if (func_bloque && func_bloque->tipo == AST_BLOQUE)
                {
                    NodoAST *sentencia = func_bloque->datos.bloque.primera;
                    while (sentencia)
                    {
                        if (sentencia->tipo == AST_FUNCION || sentencia->tipo == AST_SUBPROGRAMA)
                        {
                            evaluar_nodo(sentencia, ctx);
                        }
                        sentencia = sentencia->siguiente;
                    }
                }

                // Procesar declaraciones globales (variables, constantes)
                NodoAST *decl_bloque = ast_inc->datos.programa.declaraciones;
                if (decl_bloque && decl_bloque->tipo == AST_BLOQUE)
                {
                    NodoAST *decl = decl_bloque->datos.bloque.primera;
                    while (decl)
                    {
                        if (decl->tipo == AST_DECLARACION_VARIABLE ||
                            decl->tipo == AST_DECLARACION_CONSTANTE ||
                            decl->tipo == AST_DECLARACION_LISTA ||
                            decl->tipo == AST_DECLARACION_MATRIZ ||
                            decl->tipo == AST_DECLARACION_TEXTO_EXTENSO)
                        {
                            evaluar_nodo(decl, ctx);
                        }
                        decl = decl->siguiente;
                    }
                }
            }

            // NO liberar ast_inc porque el contexto guarda punteros a sus nodos
            lexer_destruir(lexer_inc);
            parser_destruir(parser_inc);
            free(contenido);

            return valor_crear_vacio();
        }

        // --------------------------------------------------------
        // MANEJO DE ERRORES: ALERTA
        // --------------------------------------------------------
        case AST_ALERTA:
        {
            // Evaluar el mensaje (puede ser cualquier expresión)
            Valor mensaje = evaluar_nodo(nodo->datos.alerta.mensaje, ctx);
            if (ctx->hay_error)
                return valor_crear_vacio();
            // Convertir a string (reducido a 256 bytes para evitar warnings de truncamiento)
            char buffer[256];
            switch (mensaje.tipo)
            {
            case VALOR_TEXTO:
            case VALOR_TEXTO_EXTENSO:
                snprintf(buffer, sizeof(buffer), "%.240s", mensaje.datos.texto ? mensaje.datos.texto : "");
                break;
            case VALOR_ENTERO:
                snprintf(buffer, sizeof(buffer), "%lld", mensaje.datos.entero);
                break;
            case VALOR_DECIMAL:
                snprintf(buffer, sizeof(buffer), "%g", mensaje.datos.decimal);
                break;
            case VALOR_LOGICA:
                snprintf(buffer, sizeof(buffer), "%s", mensaje.datos.logica ? "VERDADERO" : "FALSO");
                break;
            case VALOR_CARACTER:
                snprintf(buffer, sizeof(buffer), "%.10s", mensaje.datos.texto ? mensaje.datos.texto : "");
                break;
            default:
                snprintf(buffer, sizeof(buffer), "(error)");
                break;
            }
            valor_destruir(&mensaje);
            // Si estamos dentro de un INTENTAR, capturar el error
            if (ctx->intentar_stack_top > 0 &&
                ctx->intentar_stack[ctx->intentar_stack_top - 1].activo)
            {
                // Guardar el error en el tope del stack (truncado a 240 caracteres)
                ctx->intentar_stack[ctx->intentar_stack_top - 1].error_capturado = true;
                snprintf(ctx->intentar_stack[ctx->intentar_stack_top - 1].mensaje_error,
                         sizeof(ctx->intentar_stack[ctx->intentar_stack_top - 1].mensaje_error),
                         "%.240s", buffer);
                ctx->intentar_stack[ctx->intentar_stack_top - 1].linea_error = nodo->linea;
                ctx->intentar_stack[ctx->intentar_stack_top - 1].codigo_error = 1;
                ctx->estado_flujo = FLUJO_BREAK;
                return valor_crear_vacio();
            }
            // Si no estamos en INTENTAR, es un error fatal
            contexto_set_error(ctx, buffer);
            return valor_crear_vacio();
        }

        // --------------------------------------------------------
        // MANEJO DE ERRORES: INTENTAR/ATRAPAR
        // --------------------------------------------------------
        case AST_INTENTAR_ATRAPAR:
        {
            // Verificar que haya espacio en el stack
            if (ctx->intentar_stack_top >= MAX_INTENTAR_STACK)
            {
                contexto_set_error(ctx, "Demasiados bloques INTENTAR anidados (máximo 50)");
                return valor_crear_vacio();
            }

            // Push al stack
            int top = ctx->intentar_stack_top;
            ctx->intentar_stack[top].activo = true;
            ctx->intentar_stack[top].error_capturado = false;
            ctx->intentar_stack[top].mensaje_error[0] = '\0';
            ctx->intentar_stack[top].linea_error = 0;
            ctx->intentar_stack[top].codigo_error = 0;
            ctx->intentar_stack_top++;

            // Guardar estado de flujo anterior
            EstadoFlujo estado_anterior = ctx->estado_flujo;
            ctx->estado_flujo = FLUJO_NORMAL;

            // Ejecutar bloque INTENTAR
            evaluar_bloque(nodo->datos.intentatrapar.bloque_intent, ctx);

            // Restaurar estado de flujo (por si quedó en BREAK por ALERTA)
            ctx->estado_flujo = estado_anterior;

            // Pop del stack
            ctx->intentar_stack_top--;
            bool hubo_error = ctx->intentar_stack[top].error_capturado || ctx->hay_error;
            char mensaje_error[512];
            int linea_error = 0;
            int codigo_error = 0;

            if (ctx->intentar_stack[top].error_capturado)
            {
                // Error capturado por ALERTA
                snprintf(mensaje_error, sizeof(mensaje_error), "%s",
                         ctx->intentar_stack[top].mensaje_error);
                linea_error = ctx->intentar_stack[top].linea_error;
                codigo_error = ctx->intentar_stack[top].codigo_error;
            }
            else if (ctx->hay_error)
            {
                // Error del sistema (división por cero, índice fuera de rango, etc.)
                snprintf(mensaje_error, sizeof(mensaje_error), "%s", ctx->mensaje_error);
                linea_error = ctx->linea_actual;
                codigo_error = 1; // Código genérico

                // Limpiar el error del contexto para que ATRAPAR pueda ejecutarse
                ctx->hay_error = false;
                ctx->mensaje_error[0] = '\0';
            }

            // Si hubo error, ejecutar bloque ATRAPAR
            if (hubo_error)
            {
                // Crear variables automáticas en el scope actual
                tabla_simbolos_definir(ctx->tabla_actual, "ERROR",
                                       valor_crear_texto(mensaje_error), false);
                tabla_simbolos_definir(ctx->tabla_actual, "LINEA_ERROR",
                                       valor_crear_entero(linea_error), false);
                tabla_simbolos_definir(ctx->tabla_actual, "CODIGO_ERROR",
                                       valor_crear_entero(codigo_error), false);

                // Ejecutar bloque ATRAPAR
                evaluar_bloque(nodo->datos.intentatrapar.bloque_atrapar, ctx);
            }

            return valor_crear_vacio();
        }

        case AST_CONFIGURARPIN:
        {
            Valor val_pin = evaluar_nodo(nodo->datos.configurarpin.pin, ctx);
            if (ctx->hay_error)
                return valor_crear_vacio();

            int pin = 0;
            if (val_pin.tipo == VALOR_ENTERO)
            {
                pin = val_pin.datos.entero;
            }
            else if (val_pin.tipo == VALOR_DECIMAL)
            {
                pin = (int)val_pin.datos.decimal;
            }
            else
            {
                contexto_set_error(ctx, "CONFIGURARPIN: el pin debe ser un número entero");
                return valor_crear_vacio();
            }

            procesar_gpio_configurar(pin, nodo->datos.configurarpin.direccion, nodo->datos.configurarpin.bias);
            return valor_crear_vacio();
        }

        // --------------------------------------------------------
        // GPIO: ESTADOPIN
        // --------------------------------------------------------
        case AST_ESTADOPIN:
        {
            Valor val_pin = evaluar_nodo(nodo->datos.estadopin.pin, ctx);
            if (ctx->hay_error)
                return valor_crear_vacio();

            int pin = 0;
            if (val_pin.tipo == VALOR_ENTERO)
            {
                pin = val_pin.datos.entero;
            }
            else if (val_pin.tipo == VALOR_DECIMAL)
            {
                pin = (int)val_pin.datos.decimal;
            }
            else
            {
                contexto_set_error(ctx, "ESTADOPIN: el pin debe ser un número entero");
                return valor_crear_vacio();
            }

            procesar_gpio_estado_pin(pin, nodo->datos.estadopin.valor);
            return valor_crear_vacio();
        }

        // --------------------------------------------------------
        // GPIO: LEERPIN
        // --------------------------------------------------------
        case AST_LEERPIN:
        {
            Valor val_pin = evaluar_nodo(nodo->datos.leerpin.pin, ctx);
            if (ctx->hay_error)
                return valor_crear_vacio();

            int pin = 0;
            if (val_pin.tipo == VALOR_ENTERO)
            {
                pin = val_pin.datos.entero;
            }
            else if (val_pin.tipo == VALOR_DECIMAL)
            {
                pin = (int)val_pin.datos.decimal;
            }
            else
            {
                contexto_set_error(ctx, "LEERPIN: el pin debe ser un número entero");
                return valor_crear_vacio();
            }

            int valor_leido = procesar_gpio_leer(pin);

            // Si hay variable destino, asignar el valor
            if (nodo->datos.leerpin.variable_destino)
            {
                Simbolo *sim = tabla_simbolos_buscar_simbolo(ctx->tabla_actual, nodo->datos.leerpin.variable_destino);
                if (!sim)
                {
                    sim = tabla_simbolos_buscar_simbolo(ctx->tabla_global, nodo->datos.leerpin.variable_destino);
                }

                if (sim)
                {
                    sim->valor = valor_crear_entero(valor_leido);
                }
                else
                {
                    char error_msg[256];
                    snprintf(error_msg, sizeof(error_msg), "Variable '$%s' no declarada", nodo->datos.leerpin.variable_destino);
                    contexto_set_error(ctx, error_msg);
                }
            }
            else
            {
                // Si no hay variable, imprimir el valor
                printf("%d", valor_leido);
                fflush(stdout);
            }

            return valor_crear_vacio();
        }

        // PWM
        case AST_PWM:
        {
            Valor val_pin = evaluar_nodo(nodo->datos.pwm.pin, ctx);
            if (ctx->hay_error) return valor_crear_vacio();
            
            Valor val_freq = evaluar_nodo(nodo->datos.pwm.frecuencia, ctx);
            if (ctx->hay_error) return valor_crear_vacio();
            
            Valor val_duty = evaluar_nodo(nodo->datos.pwm.duty_cycle, ctx);
            if (ctx->hay_error) return valor_crear_vacio();
            
            int pin = 0;
            if (val_pin.tipo == VALOR_ENTERO) {
                pin = val_pin.datos.entero;
            } else {
                contexto_set_error(ctx, "PWM: el pin debe ser un número entero");
                return valor_crear_vacio();
            }
            
            unsigned int frecuencia = 0;
            if (val_freq.tipo == VALOR_ENTERO) {
                frecuencia = (unsigned int)val_freq.datos.entero;
            } else if (val_freq.tipo == VALOR_DECIMAL) {
                frecuencia = (unsigned int)val_freq.datos.decimal;
            } else {
                contexto_set_error(ctx, "PWM: la frecuencia debe ser un número");
                return valor_crear_vacio();
            }
            
            int duty_cycle = 0;
            if (val_duty.tipo == VALOR_ENTERO) {
                duty_cycle = val_duty.datos.entero;
            } else if (val_duty.tipo == VALOR_DECIMAL) {
                duty_cycle = (int)val_duty.datos.decimal;
            } else {
                contexto_set_error(ctx, "PWM: el duty cycle debe ser un número");
                return valor_crear_vacio();
            }
            
            if (pwm_configurar(pin, frecuencia, duty_cycle) < 0) {
                contexto_set_error(ctx, "PWM: error al configurar");
            }
            
            return valor_crear_vacio();
        }
        
        // DETENERPWM
        case AST_DETENERPWM:
        {
            Valor val_pin = evaluar_nodo(nodo->datos.detenerpwm.pin, ctx);
            if (ctx->hay_error) return valor_crear_vacio();
            
            int pin = 0;
            if (val_pin.tipo == VALOR_ENTERO) {
                pin = val_pin.datos.entero;
            } else {
                contexto_set_error(ctx, "DETENERPWM: el pin debe ser un número entero");
                return valor_crear_vacio();
            }
            
            if (pwm_detener(pin) < 0) {
                contexto_set_error(ctx, "DETENERPWM: error al detener");
            }
            
            return valor_crear_vacio();
        }

        default:
            contexto_set_error(ctx, "Tipo de nodo no soportado");
            return valor_crear_vacio();
    }
}

// ============================================================
// EVALUACIÓN DE BLOQUES
// ============================================================
static Valor evaluar_bloque(NodoAST *bloque, Contexto *ctx)
{
    if (!bloque || bloque->tipo != AST_BLOQUE)
    {
        return valor_crear_vacio();
    }

    // Primer pase: registrar etiquetas de este bloque
    registrar_etiquetas_ast(bloque, ctx);

    NodoAST *actual = bloque->datos.bloque.primera;

    while (actual)
    {
        // Si hay un salto pendiente, buscar la etiqueta SOLO en este bloque
        if (ctx->estado_flujo == FLUJO_SALTAR)
        {
            NodoAST *buscador = bloque->datos.bloque.primera;
            bool encontrada = false;

            while (buscador)
            {
                if (buscador->tipo == AST_ETIQUETA &&
                    strcmp(buscador->datos.etiqueta.nombre, ctx->etiqueta_salto) == 0)
                {
                    actual = buscador;
                    ctx->estado_flujo = FLUJO_NORMAL;
                    free(ctx->etiqueta_salto);
                    ctx->etiqueta_salto = NULL;
                    encontrada = true;
                    break;
                }
                buscador = buscador->siguiente;
            }

            // Si no se encontró en este bloque, salir para que el padre la busque
            if (!encontrada)
            {
                return valor_crear_vacio();
            }

            // Si se encontró, continuar desde la etiqueta
            continue;
        }

        // Si el flujo no es normal, salir
        if (ctx->estado_flujo != FLUJO_NORMAL)
        {
            break;
        }

        Valor resultado = evaluar_nodo(actual, ctx);
        valor_destruir(&resultado);

        if (ctx->hay_error)
        {
            return valor_crear_vacio();
        }

        actual = actual->siguiente;
    }

    return valor_crear_vacio();
}

static void evaluar_sentencia(NodoAST* nodo, Contexto* ctx) __attribute__((unused));
static void evaluar_sentencia(NodoAST* nodo, Contexto* ctx) {
    Valor resultado = evaluar_nodo(nodo, ctx);
    valor_destruir(&resultado);
}

// ============================================================
// PUNTO DE ENTRADA PRINCIPAL
// ============================================================
void evaluar_ast(NodoAST* ast, Contexto* ctx) {
    if (!ast || !ctx) return;
    
    Valor resultado = evaluar_nodo(ast, ctx);
    valor_destruir(&resultado);
}