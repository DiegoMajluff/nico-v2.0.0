/*
 * Nico v2.0.0 - Intérprete Educativo de Scripting en Español
 * @file:         evaluator.h
 * @author:       Diego Alejandro Majluff (Diseño, Arquitectura y Supervisión)
 * @ai_assist:    Qwen (Alibaba Cloud) - Implementación, Debugging y Optimización
 * @license:      MIT / Personal Use (ver LICENSE)
 * @description:  Definiciones del evaluador/intérprete. Contiene estructuras
 *                para contexto de ejecución, tabla de símbolos, valores,
 *                tipos de datos y estado de flujo del programa.
 */
#ifndef EVALUATOR_H
#define EVALUATOR_H

#include "ast.h"
#include <stdbool.h>
#include <stdio.h>

#ifndef _WIN32
#include <termios.h>
#endif

void teclado_iniciar_modo_raw(void);
void teclado_restaurar_modo(void);

#ifndef _WIN32
struct termios *obtener_terminal_original(void);
#else
void *obtener_terminal_original(void);
#endif

// ============================================================
// TIPOS DE VALOR
// ============================================================
typedef enum {
    VALOR_VACIO,
    VALOR_ENTERO,
    VALOR_ENTERO_SIN_SIGNO,
    VALOR_DECIMAL,
    VALOR_DECIMAL_SIN_SIGNO,
    VALOR_CARACTER,
    VALOR_CARACTER_SIN_SIGNO,
    VALOR_TEXTO,
    VALOR_TEXTO_EXTENSO,
    VALOR_LOGICA,
    VALOR_LISTA,
    VALOR_MATRIZ,
    VALOR_ARCHIVO
} TipoValor;

// ============================================================
// ESTRUCTURA DE VALOR
// ============================================================
typedef struct Valor {
    TipoValor tipo;
    union {
        long long entero;
        unsigned long long entero_sin_signo;
        double decimal;
        double decimal_sin_signo;
        char caracter;
        unsigned char caracter_sin_signo;
        char* texto;
        char* texto_extenso;
        bool logica;
        struct Valor* lista;  // Array de valores
        struct Valor** matriz; // Array 2D de valores
        FILE* archivo; 
    } datos;
    int tamano;  // Para listas/matrices
    int filas;   // Para matrices
    int columnas; // Para matrices
    int modo_archivo; 
} Valor;

// ============================================================
// TABLA DE SÍMBOLOS
// ============================================================
typedef struct Simbolo {
    char* nombre;
    Valor valor;
    bool es_constante;
    struct Simbolo* siguiente;
} Simbolo;

typedef struct TablaSimbolos {
    Simbolo* primera;
    struct TablaSimbolos* padre;  // Para scopes anidados
} TablaSimbolos;

// ============================================================
// DEFINICIÓN DE FUNCIÓN/SUBPROGRAMA
// ============================================================
    typedef struct ParametroFuncion
{
    char *nombre;
    TipoDato tipo;
} ParametroFuncion;

typedef struct DefinicionFuncion
{
    char *nombre;
    TipoDato tipo_retorno; // Para FUNCION (para SUBPROGRAMA es TIPO_VACIO)
    bool es_subprograma;   // true = SUBPROGRAMA, false = FUNCION
    int num_parametros;
    ParametroFuncion *parametros;
    NodoAST *bloque; // Cuerpo de la función
    struct DefinicionFuncion *siguiente;
} DefinicionFuncion;

// ============================================================
// CONTEXTO DE EJECUCIÓN
// ============================================================
typedef enum
{
    FLUJO_NORMAL,
    FLUJO_BREAK,
    FLUJO_RETURN,
    FLUJO_CONTINUE,
    FLUJO_SALTAR
} EstadoFlujo;

typedef struct Etiqueta
{
    char *nombre;
    NodoAST *nodo;
    struct Etiqueta *siguiente;
} Etiqueta;

typedef struct Contexto {
    char *nombre_programa; // Nombre del programa
    TablaSimbolos* tabla_global;
    TablaSimbolos* tabla_actual;
    EstadoFlujo estado_flujo;
    Valor valor_retorno;
    bool hay_error;
    char mensaje_error[512];
    int linea_actual;
    DefinicionFuncion *funciones; 
    int profundidad_llamada;
    // Pool de tablas reutilizables para funciones
    TablaSimbolos *pool_tablas[1000]; // Pool para hasta 1000 niveles de recursión
    int pool_index;                   // Índice actual del pool
    void *sqlite_db;               // sqlite3*
    void *sqlite_stmt;             // sqlite3_stmt* (resultado de consulta)
    int sqlite_columnas;           // Número de columnas en el resultado
    char **sqlite_columnas_nombre; // Nombres de las columnas
    Etiqueta *etiquetas;
    char *salto_pendiente;
    char *etiqueta_salto;
} Contexto;

// ============================================================
// FUNCIONES PÚBLICAS
// ============================================================

// Crear/destruir contexto
Contexto* contexto_crear(void);
void contexto_destruir(Contexto* ctx);

// Evaluar el AST completo
void evaluar_ast(NodoAST* ast, Contexto* ctx);

// Evaluar un nodo específico
Valor evaluar_nodo(NodoAST* nodo, Contexto* ctx);

// Funciones de valor
Valor valor_crear_entero(long long valor);
Valor valor_crear_decimal(double valor);
Valor valor_crear_texto(const char* valor);
Valor valor_crear_logica(bool valor);
Valor valor_crear_caracter(const char *valor);
Valor valor_crear_entero_sin_signo(unsigned int valor);
Valor valor_crear_decimal_sin_signo(double valor);
Valor valor_crear_caracter_sin_signo(unsigned char valor);
Valor valor_crear_vacio(void);
void valor_destruir(Valor* valor);
void valor_imprimir(Valor valor);

// Gestión de funciones definidas por usuario
void contexto_registrar_funcion(Contexto *ctx, DefinicionFuncion *func);
DefinicionFuncion *contexto_buscar_funcion(Contexto *ctx, const char *nombre);
void contexto_liberar_funciones(Contexto *ctx);

// Funciones de tabla de símbolos
void tabla_simbolos_definir(TablaSimbolos* tabla, const char* nombre, Valor valor, bool es_constante);
Valor* tabla_simbolos_buscar(TablaSimbolos* tabla, const char* nombre);
void tabla_simbolos_destruir(TablaSimbolos* tabla);

// Verificar si hay error
bool contexto_tiene_error(const Contexto* ctx);
const char* contexto_obtener_error(const Contexto* ctx);

#endif // EVALUATOR_H
