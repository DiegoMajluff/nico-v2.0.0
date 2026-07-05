/*
 * Nico v2.0.0 - Intérprete Educativo de Scripting en Español
 * @file:         parser.c
 * @author:       Diego Alejandro Majluff (Diseño, Arquitectura y Supervisión)
 * @ai_assist:    Qwen (Alibaba Cloud) - Implementación, Debugging y Optimización
 * @license:      MIT / Personal Use (ver LICENSE)
 * @description:  Implementación del analizador sintáctico (parser). Transforma
 *                la secuencia de tokens en AST utilizando descenso recursivo.
 *                Maneja gramática completa: declaraciones, expresiones, bloques,
 *                control de flujo (SI/MIENTRAS/PARA), funciones, subprogramas,
 *                hardware (GPIO/PWM), bases de datos y servidor web.
 */
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#define MAX_ITERACIONES_PARSER 10000
// Constante local
#define MAX_PARAMETROS 10

// Forward declarations
static NodoAST *parsear_bloque(Parser *parser);
static NodoAST *parsear_bloque_hasta(Parser *parser, TipoToken token_parada);
static NodoAST *parsear_sentencia(Parser *parser);
static NodoAST *parsear_factor(Parser *parser);
static NodoAST *parsear_termino(Parser *parser);
static NodoAST *parsear_expresion_simple(Parser *parser);
static NodoAST *parsear_funcion(Parser *parser);
static NodoAST *parsear_subprograma(Parser *parser);

// ============================================================
// FUNCIONES AUXILIARES INTERNAS
// ============================================================
static void parser_set_error(Parser* parser, const char* mensaje) {
    parser->hay_error = true;
    snprintf(parser->mensaje_error, sizeof(parser->mensaje_error),
             "Línea %d, columna %d: %s (token: %s)",
             parser->token_actual.linea, parser->token_actual.columna,
             mensaje, lexer_nombre_token(parser->token_actual.tipo));
}

static void parser_avanzar(Parser* parser) {
    token_destruir(&parser->token_actual);
    parser->token_actual = parser->token_siguiente;
    parser->token_siguiente = lexer_siguiente_token(parser->lexer);
}

static bool parser_coincidir(Parser* parser, TipoToken tipo) {
    return parser->token_actual.tipo == tipo;
}

static bool parser_esperar(Parser* parser, TipoToken tipo) {
    if (parser->token_actual.tipo == tipo) {
        parser_avanzar(parser);
        return true;
    }
    char msg[256];
    snprintf(msg, sizeof(msg), "Se esperaba %s", lexer_nombre_token(tipo));
    parser_set_error(parser, msg);
    return false;
}

static bool parser_verificar_siguiente(Parser* parser, TipoToken tipo) {
    return parser->token_siguiente.tipo == tipo;
}

static int parsear_modo_archivo(Parser* parser) {
    if (parser_coincidir(parser, TOK_ESCRITURA)) {
        parser_avanzar(parser);
        return 0;
    }
    else if (parser_coincidir(parser, TOK_AGREGAR)) {
        parser_avanzar(parser);
        return 1;
    }
    else if (parser_coincidir(parser, TOK_LECTURA)) {
        parser_avanzar(parser);
        return 2;
    }
    else if (parser_coincidir(parser, TOK_LECTOESCRITURA)) {
        parser_avanzar(parser);
        return 3;
    }
    
    parser_set_error(parser, "Modo de archivo no válido (ESCRITURA, AGREGAR, LECTURA, LECTOESCRITURA)");
    return -1;
}

// ============================================================
// FUNCIONES DE PARSEO - DECLARACIONES
// ============================================================
static TipoDato parsear_tipo_dato(Parser* parser) {
    if (parser_coincidir(parser, TOK_ENTERA)) {
        parser_avanzar(parser);
        if (parser_coincidir(parser, TOK_SIN)) {
            parser_avanzar(parser);
            if (parser_coincidir(parser, TOK_SIGNO)) {
                parser_avanzar(parser);
                return TIPO_ENTERO_SIN_SIGNO;
            }
            parser_set_error(parser, "Se esperaba SIGNO después de SIN");
            return TIPO_ENTERO;
        }
        return TIPO_ENTERO;
    }
    else if (parser_coincidir(parser, TOK_DECIMAL)) {
        parser_avanzar(parser);
        if (parser_coincidir(parser, TOK_SIN)) {
            parser_avanzar(parser);
            if (parser_coincidir(parser, TOK_SIGNO)) {
                parser_avanzar(parser);
                return TIPO_DECIMAL_SIN_SIGNO;
            }
            parser_set_error(parser, "Se esperaba SIGNO después de SIN");
            return TIPO_DECIMAL;
        }
        return TIPO_DECIMAL;
    }
    else if (parser_coincidir(parser, TOK_TEXTO_KW)) {
        parser_avanzar(parser);
        if (parser_coincidir(parser, TOK_EXTENSO)) {
            parser_avanzar(parser);
            return TIPO_TEXTO_EXTENSO;
        }
        return TIPO_TEXTO;
    }
    else if (parser_coincidir(parser, TOK_CARACTER_KW)) {
        parser_avanzar(parser);
        if (parser_coincidir(parser, TOK_SIN)) {
            parser_avanzar(parser);
            if (parser_coincidir(parser, TOK_SIGNO)) {
                parser_avanzar(parser);
                return TIPO_CARACTER_SIN_SIGNO;
            }
            parser_set_error(parser, "Se esperaba SIGNO después de SIN");
            return TIPO_CARACTER;
        }
        return TIPO_CARACTER;
    }
    else if (parser_coincidir(parser, TOK_ARCHIVO)) {
        parser_avanzar(parser);
        return TIPO_ARCHIVO;
    }
    else if (parser_coincidir(parser, TOK_LOGICA)) {
        parser_avanzar(parser);
        return TIPO_LOGICA;
    }
    
    parser_set_error(parser, "Tipo de dato no reconocido");
    return TIPO_ENTERO;
}

static NodoAST* parsear_literal(Parser* parser) {
    int linea = parser->token_actual.linea;
    
    if (parser_coincidir(parser, TOK_NUMERO_ENTERO)) {
        double valor = atof(parser->token_actual.valor);
        parser_avanzar(parser);
        return crear_nodo_literal_numero(valor, linea);
    }
    else if (parser_coincidir(parser, TOK_NUMERO_DECIMAL)) {
        double valor = atof(parser->token_actual.valor);
        parser_avanzar(parser);
        return crear_nodo_literal_numero(valor, linea);
    }
    else if (parser_coincidir(parser, TOK_NUMERO_HEX))
    {
        unsigned long long valor = strtoull(parser->token_actual.valor, NULL, 16);
        parser_avanzar(parser);
        return crear_nodo_literal_numero((double)valor, linea);
    }
    else if (parser_coincidir(parser, TOK_NUMERO_OCTAL)) {
        double valor = (double)strtol(parser->token_actual.valor, NULL, 8);
        parser_avanzar(parser);
        return crear_nodo_literal_numero(valor, linea);
    }
    else if (parser_coincidir(parser, TOK_TEXTO)) {
        NodoAST* nodo = crear_nodo_literal_texto(parser->token_actual.valor, linea);
        parser_avanzar(parser);
        return nodo;
    }
    else if (parser_coincidir(parser, TOK_CARACTER))
    {
        NodoAST *nodo = crear_nodo_literal_caracter(parser->token_actual.valor, linea);
        parser_avanzar(parser);
        return nodo;
    }
    else if (parser_coincidir(parser, TOK_VERDADERO)) {
        parser_avanzar(parser);
        return crear_nodo_literal_logico(true, linea);
    }
    else if (parser_coincidir(parser, TOK_FALSO)) {
        parser_avanzar(parser);
        return crear_nodo_literal_logico(false, linea);
    }
    
    parser_set_error(parser, "Se esperaba un literal");
    return NULL;
}

static NodoAST *parsear_factor(Parser *parser)
{
    int linea = parser->token_actual.linea;

    // Operador unario
    if (parser_coincidir(parser, TOK_MENOS))
    {
        parser_avanzar(parser);
        NodoAST *operando = parsear_factor(parser);
        return crear_nodo_operador_unario(OP_NEGACION, operando, linea);
    }
    else if (parser_coincidir(parser, TOK_NO))
    {
        parser_avanzar(parser);
        NodoAST *operando = parsear_factor(parser);
        return crear_nodo_operador_unario(OP_NO_LOGICO, operando, linea);
    }

    // Operadores bit a bit usados como funciones
    if (parser_coincidir(parser, TOK_BITY) ||
        parser_coincidir(parser, TOK_BITO) ||
        parser_coincidir(parser, TOK_BITXOR) ||
        parser_coincidir(parser, TOK_DESPLAZARIZQUIERDA) ||
        parser_coincidir(parser, TOK_DESPLAZARDERECHA) ||
        parser_coincidir(parser, TOK_ROTARIZQUIERDA) ||
        parser_coincidir(parser, TOK_ROTARDERECHA) ||
        parser_coincidir(parser, TOK_LEERBIT) ||
        parser_coincidir(parser, TOK_ACTIVARBIT) ||
        parser_coincidir(parser, TOK_DESACTIVARBIT) ||
        parser_coincidir(parser, TOK_BITNO))
    {
        int linea = parser->token_actual.linea;
        char *nombre = strdup(parser->token_actual.valor);
        parser_avanzar(parser);

        if (!parser_esperar(parser, TOK_PARENTESIS_ABRIR))
        {
            free(nombre);
            return NULL;
        }

        NodoAST *argumentos = crear_nodo_bloque(linea);

        if (!parser_coincidir(parser, TOK_PARENTESIS_CERRAR))
        {
            NodoAST *arg = parsear_expresion(parser);
            agregar_sentencia_a_bloque(argumentos, arg);

            while (parser_coincidir(parser, TOK_COMA))
            {
                parser_avanzar(parser);
                arg = parsear_expresion(parser);
                agregar_sentencia_a_bloque(argumentos, arg);
            }

            if (!parser_esperar(parser, TOK_PARENTESIS_CERRAR))
            {
                free(nombre);
                return NULL;
            }
        }
        else
        {
            parser_avanzar(parser); // Consumir )
        }

        return crear_nodo_llamada_funcion(nombre, argumentos, linea);
    }

    // Paréntesis
    if (parser_coincidir(parser, TOK_PARENTESIS_ABRIR))
    {
        parser_avanzar(parser);
        NodoAST *expr = parsear_expresion(parser);
        if (!parser_esperar(parser, TOK_PARENTESIS_CERRAR))
        {
            return NULL;
        }
        return expr;
    }

    // Caso especial para REDONDEAR con modo (ARRIBA, ABAJO, ENTERA)
    if (parser_coincidir(parser, TOK_REDONDEAR))
    {
        int linea = parser->token_actual.linea;
        parser_avanzar(parser); // Consumir REDONDEAR

        if (!parser_esperar(parser, TOK_PARENTESIS_ABRIR))
        {
            return NULL;
        }

        // Primer argumento: modo (ARRIBA, ABAJO, ENTERA)
        char *modo = NULL;
        if (parser_coincidir(parser, TOK_ARRIBA) ||
            parser_coincidir(parser, TOK_ABAJO) ||
            parser_coincidir(parser, TOK_ENTERO_KW))
        {
            modo = strdup(parser->token_actual.valor);
            parser_avanzar(parser);
        }
        else
        {
            parser_set_error(parser, "Se esperaba ARRIBA, ABAJO o ENTERA");
            return NULL;
        }

        if (!parser_esperar(parser, TOK_COMA))
        {
            free(modo);
            return NULL;
        }

        // Segundo argumento: número
        NodoAST *numero = parsear_expresion(parser);
        if (!numero)
        {
            free(modo);
            return NULL;
        }

        if (!parser_esperar(parser, TOK_PARENTESIS_CERRAR))
        {
            free(modo);
            liberar_nodo(numero);
            return NULL;
        }

        // Crear llamada a función con dos argumentos
        NodoAST *argumentos = crear_nodo_bloque(linea);
        NodoAST *modo_nodo = crear_nodo_literal_texto(modo, linea);
        agregar_sentencia_a_bloque(argumentos, modo_nodo);
        agregar_sentencia_a_bloque(argumentos, numero);

        return crear_nodo_llamada_funcion("REDONDEAR", argumentos, linea);
    }

    // Funciones matemáticas integradas (SENO, COSENO, TANGENTE, RAIZ, etc.)
    if (parser->token_actual.tipo >= TOK_SENO && parser->token_actual.tipo <= TOK_RAIZCUBICA)
    {
        int linea = parser->token_actual.linea;
        char *nombre = strdup(parser->token_actual.valor);
        parser_avanzar(parser); // Consumir nombre de la función

        if (!parser_esperar(parser, TOK_PARENTESIS_ABRIR))
        {
            free(nombre);
            return NULL;
        }

        NodoAST *argumentos = crear_nodo_bloque(linea);

        if (!parser_coincidir(parser, TOK_PARENTESIS_CERRAR))
        {
            NodoAST *arg = parsear_expresion(parser);
            agregar_sentencia_a_bloque(argumentos, arg);

            while (parser_coincidir(parser, TOK_COMA))
            {
                parser_avanzar(parser);
                arg = parsear_expresion(parser);
                agregar_sentencia_a_bloque(argumentos, arg);
            }

            if (!parser_esperar(parser, TOK_PARENTESIS_CERRAR))
            {
                free(nombre);
                return NULL;
            }
        }

        return crear_nodo_llamada_funcion(nombre, argumentos, linea);
    }

    // Función ALEATORIO(tipo, min, max)
    if (parser_coincidir(parser, TOK_ALEATORIO) || parser_coincidir(parser, TOK_ALEATORIOSINSIGNO))
    {
        int linea = parser->token_actual.linea;
        char *nombre = strdup(parser->token_actual.valor);
        parser_avanzar(parser); // Consumir ALEATORIO

        if (!parser_esperar(parser, TOK_PARENTESIS_ABRIR))
        {
            free(nombre);
            return NULL;
        }

        NodoAST *argumentos = crear_nodo_bloque(linea);

        // Primer argumento: tipo (ENTERO o DECIMAL)
        if (parser_coincidir(parser, TOK_ENTERO_KW) || parser_coincidir(parser, TOK_DECIMAL))
        {
            NodoAST *tipo = crear_nodo_literal_texto(strdup(parser->token_actual.valor), linea);
            agregar_sentencia_a_bloque(argumentos, tipo);
            parser_avanzar(parser);
        }
        else
        {
            parser_set_error(parser, "ALEATORIO requiere tipo (ENTERO o DECIMAL) como primer argumento");
            free(nombre);
            return NULL;
        }

        if (!parser_esperar(parser, TOK_COMA))
        {
            free(nombre);
            return NULL;
        }

        // Segundo argumento: mínimo
        NodoAST *min = parsear_expresion(parser);
        if (!min)
        {
            free(nombre);
            return NULL;
        }
        agregar_sentencia_a_bloque(argumentos, min);

        if (!parser_esperar(parser, TOK_COMA))
        {
            free(nombre);
            return NULL;
        }

        // Tercer argumento: máximo
        NodoAST *max = parsear_expresion(parser);
        if (!max)
        {
            free(nombre);
            return NULL;
        }
        agregar_sentencia_a_bloque(argumentos, max);

        if (!parser_esperar(parser, TOK_PARENTESIS_CERRAR))
        {
            free(nombre);
            return NULL;
        }

        return crear_nodo_llamada_funcion(nombre, argumentos, linea);
    }

    // Funciones de texto integradas (LONGITUDTEXTO, BUSCARTEXTO, etc.)
    if (parser->token_actual.tipo >= TOK_LONGITUDTEXTO && parser->token_actual.tipo <= TOK_CARACTERATEXTO)
    {
        int linea = parser->token_actual.linea;
        char *nombre = strdup(parser->token_actual.valor);
        parser_avanzar(parser); // Consumir nombre de la función

        if (!parser_esperar(parser, TOK_PARENTESIS_ABRIR))
        {
            free(nombre);
            return NULL;
        }

        NodoAST *argumentos = crear_nodo_bloque(linea);

        if (!parser_coincidir(parser, TOK_PARENTESIS_CERRAR))
        {
            NodoAST *arg = parsear_expresion(parser);
            agregar_sentencia_a_bloque(argumentos, arg);

            while (parser_coincidir(parser, TOK_COMA))
            {
                parser_avanzar(parser);
                arg = parsear_expresion(parser);
                agregar_sentencia_a_bloque(argumentos, arg);
            }

            if (!parser_esperar(parser, TOK_PARENTESIS_CERRAR))
            {
                free(nombre);
                return NULL;
            }
        }

        return crear_nodo_llamada_funcion(nombre, argumentos, linea);
    }

    // Función LONGITUDLISTA
    if (parser_coincidir(parser, TOK_LONGITUDLISTA))
    {
        int linea = parser->token_actual.linea;
        char *nombre = strdup(parser->token_actual.valor);
        parser_avanzar(parser); // Consumir LONGITUDLISTA

        if (!parser_esperar(parser, TOK_PARENTESIS_ABRIR))
        {
            free(nombre);
            return NULL;
        }

        NodoAST *argumentos = crear_nodo_bloque(linea);

        if (!parser_coincidir(parser, TOK_PARENTESIS_CERRAR))
        {
            NodoAST *arg = parsear_expresion(parser);
            agregar_sentencia_a_bloque(argumentos, arg);

            while (parser_coincidir(parser, TOK_COMA))
            {
                parser_avanzar(parser);
                arg = parsear_expresion(parser);
                agregar_sentencia_a_bloque(argumentos, arg);
            }

            if (!parser_esperar(parser, TOK_PARENTESIS_CERRAR))
            {
                free(nombre);
                return NULL;
            }
        }

        return crear_nodo_llamada_funcion(nombre, argumentos, linea);
    }

    // Constantes matemáticas (PI, E, etc.)
    if (parser->token_actual.tipo >= TOK_NUMEROPI && parser->token_actual.tipo <= TOK_LOGNATURALDE10)
    {
        int linea = parser->token_actual.linea;
        char *nombre = strdup(parser->token_actual.valor);
        parser_avanzar(parser);

        // Consumir paréntesis opcionales: ()
        if (parser_coincidir(parser, TOK_PARENTESIS_ABRIR))
        {
            parser_avanzar(parser);
            if (!parser_esperar(parser, TOK_PARENTESIS_CERRAR))
            {
                free(nombre);
                return NULL;
            }
        }

        return crear_nodo_llamada_funcion(nombre, NULL, linea);
    }
    
    // Modos de redondeo (ARRIBA, ABAJO, ENTERA) - se tratan como texto
    if (parser_coincidir(parser, TOK_ARRIBA) ||
        parser_coincidir(parser, TOK_ABAJO) ||
        parser_coincidir(parser, TOK_ENTERA))
    {
        int linea = parser->token_actual.linea;
        char *modo = strdup(parser->token_actual.valor);
        parser_avanzar(parser);
        return crear_nodo_literal_texto(modo, linea);
    }

    // Variable (con $) - con soporte para acceso a listas y matrices
    if (parser->token_actual.tipo == TOK_VARIABLE)
    {
        char *nombre = strdup(parser->token_actual.valor + 1); // Quitar el $
        parser_avanzar(parser);

        // Verificar si es acceso a elemento de lista/matriz
        if (parser_coincidir(parser, TOK_CORCHETE_ABRIR))
        {
            parser_avanzar(parser);
            NodoAST *indice1 = parsear_expresion(parser);
            if (!parser_esperar(parser, TOK_CORCHETE_CERRAR))
            {
                free(nombre);
                return NULL;
            }

            // Verificar si hay un segundo corchete (matriz)
            if (parser_coincidir(parser, TOK_CORCHETE_ABRIR))
            {
                parser_avanzar(parser);
                NodoAST *indice2 = parsear_expresion(parser);
                if (!parser_esperar(parser, TOK_CORCHETE_CERRAR))
                {
                    free(nombre);
                    return NULL;
                }
                // Es acceso a matriz [fila][columna]
                return crear_nodo_acceso_matriz(nombre, indice1, indice2, linea);
            }
            else
            {
                // Es acceso a lista [indice]
                return crear_nodo_acceso_lista(nombre, indice1, linea);
            }
        }

        return crear_nodo_variable(nombre, linea);
    }

    // Llamada a función (sin $) - cuando hay paréntesis
    if (parser_coincidir(parser, TOK_IDENTIFICADOR) &&
        parser_verificar_siguiente(parser, TOK_PARENTESIS_ABRIR))
    {
        int linea = parser->token_actual.linea;
        char *nombre = strdup(parser->token_actual.valor);
        parser_avanzar(parser); // Consumir identificador
        parser_avanzar(parser); // Consumir (

        NodoAST *argumentos = crear_nodo_bloque(linea);

        if (!parser_coincidir(parser, TOK_PARENTESIS_CERRAR))
        {
            NodoAST *arg = parsear_expresion(parser);
            agregar_sentencia_a_bloque(argumentos, arg);

            while (parser_coincidir(parser, TOK_COMA))
            {
                parser_avanzar(parser);
                arg = parsear_expresion(parser);
                agregar_sentencia_a_bloque(argumentos, arg);
            }

            if (!parser_esperar(parser, TOK_PARENTESIS_CERRAR))
            {
                free(nombre);
                return NULL;
            }
        }
        else
        {
            parser_avanzar(parser); // Consumir )
        }

        return crear_nodo_llamada_funcion(nombre, argumentos, linea);
    }

    // Literal
    if (parser_coincidir(parser, TOK_NUMERO_ENTERO) ||
        parser_coincidir(parser, TOK_NUMERO_DECIMAL) ||
        parser_coincidir(parser, TOK_NUMERO_HEX) ||
        parser_coincidir(parser, TOK_NUMERO_OCTAL) ||
        parser_coincidir(parser, TOK_TEXTO) ||
        parser_coincidir(parser, TOK_CARACTER) ||
        parser_coincidir(parser, TOK_VERDADERO) ||
        parser_coincidir(parser, TOK_FALSO))
    {
        return parsear_literal(parser);
    }

    // Llamada a función - cuando no hay paréntesis verificado antes
    if (parser_coincidir(parser, TOK_IDENTIFICADOR))
    {
        char *nombre = strdup(parser->token_actual.valor);
        parser_avanzar(parser);

        if (parser_coincidir(parser, TOK_PARENTESIS_ABRIR))
        {
            parser_avanzar(parser);
            NodoAST *args = crear_nodo_bloque(linea);

            if (!parser_coincidir(parser, TOK_PARENTESIS_CERRAR))
            {
                NodoAST *arg = parsear_expresion(parser);
                agregar_sentencia_a_bloque(args, arg);

                while (parser_coincidir(parser, TOK_COMA))
                {
                    parser_avanzar(parser);
                    arg = parsear_expresion(parser);
                    agregar_sentencia_a_bloque(args, arg);
                }
            }

            if (!parser_esperar(parser, TOK_PARENTESIS_CERRAR))
            {
                free(nombre);
                return NULL;
            }

            return crear_nodo_llamada_funcion(nombre, args, linea);
        }

        // No es llamada a función, es un identificador suelto
        free(nombre);
        parser_set_error(parser, "Identificador sin contexto válido");
        return NULL;
    }

    parser_set_error(parser, "Expresión inválida");
    return NULL;
}

static NodoAST *parsear_termino(Parser *parser)
{
    NodoAST *izq = parsear_factor(parser);
    int iteraciones = 0;

    while ((parser->token_actual.tipo == TOK_ASTERISCO ||
            parser->token_actual.tipo == TOK_DIVISION ||
            parser->token_actual.tipo == TOK_MODULO ||    // %
            parser->token_actual.tipo == TOK_MOD ||       // palabra MOD
            parser->token_actual.tipo == TOK_MODULO_FN || // palabra MODULO (función usada como op)
            parser->token_actual.tipo == TOK_POTENCIA) && // ^
           iteraciones < MAX_ITERACIONES_PARSER)
    {
        iteraciones++;
        int linea = parser->token_actual.linea;
        OperadorBinario op;

        if (parser_coincidir(parser, TOK_ASTERISCO))
        {
            op = OP_MULTIPLICACION;
        }
        else if (parser_coincidir(parser, TOK_DIVISION))
        {
            op = OP_DIVISION;
        }
        else if (parser_coincidir(parser, TOK_MODULO) ||
                 parser_coincidir(parser, TOK_MOD) ||
                 parser_coincidir(parser, TOK_MODULO_FN))
        {
            op = OP_MODULO;
        }
        else if (parser_coincidir(parser, TOK_POTENCIA))
        {
            op = OP_POTENCIA;
        }

        parser_avanzar(parser); // Consumir el operador

        NodoAST *der = parsear_factor(parser);
        izq = crear_nodo_operador_binario(op, izq, der, linea);
    }

    if (iteraciones >= MAX_ITERACIONES_PARSER)
    {
        parser_set_error(parser, "Bucle infinito detectado en parsear_termino");
    }

    return izq;
}

static NodoAST *parsear_expresion_simple(Parser *parser)
{
    NodoAST *izq = parsear_termino(parser);
    int iteraciones = 0;

    while (iteraciones < MAX_ITERACIONES_PARSER)
    {
        if (parser_coincidir(parser, TOK_MAS))
        {
            int linea = parser->token_actual.linea;
            parser_avanzar(parser);
            NodoAST *der = parsear_termino(parser);
            izq = crear_nodo_operador_binario(OP_SUMA, izq, der, linea);
        }
        else if (parser_coincidir(parser, TOK_MENOS))
        {
            int linea = parser->token_actual.linea;
            parser_avanzar(parser);
            NodoAST *der = parsear_termino(parser);
            izq = crear_nodo_operador_binario(OP_RESTA, izq, der, linea);
        }
        else if (parser_coincidir(parser, TOK_BITY))
        {
            int linea = parser->token_actual.linea;
            parser_avanzar(parser);
            NodoAST *der = parsear_termino(parser);
            izq = crear_nodo_operador_binario(OP_BIT_AND, izq, der, linea);
        }
        else if (parser_coincidir(parser, TOK_BITO))
        {
            int linea = parser->token_actual.linea;
            parser_avanzar(parser);
            NodoAST *der = parsear_termino(parser);
            izq = crear_nodo_operador_binario(OP_BIT_OR, izq, der, linea);
        }
        else if (parser_coincidir(parser, TOK_BITXOR))
        {
            int linea = parser->token_actual.linea;
            parser_avanzar(parser);
            NodoAST *der = parsear_termino(parser);
            izq = crear_nodo_operador_binario(OP_BIT_XOR, izq, der, linea);
        }
        else if (parser_coincidir(parser, TOK_DESPLAZARIZQUIERDA))
        {
            int linea = parser->token_actual.linea;
            parser_avanzar(parser);
            NodoAST *der = parsear_termino(parser);
            izq = crear_nodo_operador_binario(OP_DESPLAZAR_IZQ, izq, der, linea);
        }
        else if (parser_coincidir(parser, TOK_DESPLAZARDERECHA))
        {
            int linea = parser->token_actual.linea;
            parser_avanzar(parser);
            NodoAST *der = parsear_termino(parser);
            izq = crear_nodo_operador_binario(OP_DESPLAZAR_DER, izq, der, linea);
        }
        else
        {
            break;
        }
        iteraciones++;
    }

    if (iteraciones >= MAX_ITERACIONES_PARSER)
    {
        parser_set_error(parser, "Bucle infinito detectado en parsear_expresion_simple");
    }

    return izq;
}

NodoAST *parsear_comparacion(Parser *parser)
{
    NodoAST *izq = parsear_expresion_simple(parser);

    int iteraciones = 0;
    while (!parser->hay_error && iteraciones < MAX_ITERACIONES_PARSER)
    {
        int linea = parser->token_actual.linea;
        OperadorBinario op;
        bool encontrado = false;

        // Operadores de un solo token
        if (parser_coincidir(parser, TOK_MAYOR))
        {
            if (parser_verificar_siguiente(parser, TOK_IGUAL_KW))
            {
                parser_avanzar(parser);
                parser_avanzar(parser);
                op = OP_MAYOR_IGUAL;
                encontrado = true;
            }
            else
            {
                op = OP_MAYOR;
                parser_avanzar(parser);
                encontrado = true;
            }
        }
        else if (parser_coincidir(parser, TOK_MENOR))
        {
            if (parser_verificar_siguiente(parser, TOK_IGUAL_KW))
            {
                parser_avanzar(parser);
                parser_avanzar(parser);
                op = OP_MENOR_IGUAL;
                encontrado = true;
            }
            else
            {
                op = OP_MENOR;
                parser_avanzar(parser);
                encontrado = true;
            }
        }
        else if (parser_coincidir(parser, TOK_IGUAL_KW))
        {
            op = OP_IGUAL;
            parser_avanzar(parser);
            encontrado = true;
        }
        else if (parser_coincidir(parser, TOK_DIFERENTE))
        {
            op = OP_DIFERENTE;
            parser_avanzar(parser);
            encontrado = true;
        }
        else if (parser_coincidir(parser, TOK_MAYOR_IGUAL))
        {
            op = OP_MAYOR_IGUAL;
            parser_avanzar(parser);
            encontrado = true;
        }
        else if (parser_coincidir(parser, TOK_MENOR_IGUAL))
        {
            op = OP_MENOR_IGUAL;
            parser_avanzar(parser);
            encontrado = true;
        }

        if (!encontrado)
            break;

        NodoAST *der = parsear_expresion_simple(parser);
        izq = crear_nodo_operador_binario(op, izq, der, linea);
        iteraciones++;
    }

    if (iteraciones >= MAX_ITERACIONES_PARSER)
    {
        parser_set_error(parser, "Bucle infinito detectado en parsear_comparacion");
    }

    return izq;
}

NodoAST *parsear_expresion(Parser *parser)
{
    NodoAST *izq = parsear_comparacion(parser); // ← Cambiar de parsear_expresion_simple

    int iteraciones = 0;
    while (!parser->hay_error && iteraciones < MAX_ITERACIONES_PARSER)
    {
        int linea = parser->token_actual.linea;
        OperadorBinario op;
        bool encontrado = false;

        // Solo operadores lógicos
        if (parser_coincidir(parser, TOK_Y))
        {
            op = OP_Y_LOGICO;
            parser_avanzar(parser);
            encontrado = true;
        }
        else if (parser_coincidir(parser, TOK_O))
        {
            op = OP_O_LOGICO;
            parser_avanzar(parser);
            encontrado = true;
        }

        if (!encontrado)
            break;

        NodoAST *der = parsear_comparacion(parser); // ← Cambiar de parsear_expresion_simple
        izq = crear_nodo_operador_binario(op, izq, der, linea);
        iteraciones++;
    }

    if (iteraciones >= MAX_ITERACIONES_PARSER)
    {
        parser_set_error(parser, "Bucle infinito detectado en parsear_expresion");
    }

    return izq;
}

// ============================================================
// PARSEO DE DECLARACIONES
// ============================================================
static NodoAST *parsear_declaracion_variable(Parser *parser)
{
    int linea = parser->token_actual.linea;

    // Consumir VARIABLE o DECLARAR VARIABLE
    if (parser_coincidir(parser, TOK_DECLARAR))
    {
        parser_avanzar(parser);
    }
    if (!parser_esperar(parser, TOK_VARIABLE_KW))
        return NULL;

    TipoDato tipo = parsear_tipo_dato(parser);

    // Crear un bloque para contener múltiples declaraciones
    NodoAST *bloque = crear_nodo_bloque(linea);
    int num_vars = 0;

    // Parsear primera variable
    do
    {
        if (!parser_coincidir(parser, TOK_VARIABLE))
        {
            parser_set_error(parser, "Se esperaba nombre de variable después de '$'");
            return NULL;
        }

        char *nombre = strdup(parser->token_actual.valor + 1); // Saltar el '$'
        parser_avanzar(parser);

        NodoAST *valor_inicial = NULL;

        // Verificar si hay asignación inicial
        if (parser_coincidir(parser, TOK_IGUAL))
        {
            parser_avanzar(parser);
            valor_inicial = parsear_expresion(parser);
        }

        NodoAST *declaracion = crear_nodo_declaracion_variable(nombre, tipo, valor_inicial, linea);
        agregar_sentencia_a_bloque(bloque, declaracion);
        num_vars++;

    } while (parser_coincidir(parser, TOK_COMA) && (parser_avanzar(parser), true));

    // Si solo hay una variable, devolverla directamente
    if (num_vars == 1)
    {
        return bloque->datos.bloque.primera;
    }

    return bloque;
}

static NodoAST *parsear_declaracion_constante(Parser *parser)
{
    int linea = parser->token_actual.linea;

    // Consumir CONSTANTE o DECLARAR CONSTANTE
    if (parser_coincidir(parser, TOK_DECLARAR))
    {
        parser_avanzar(parser);
    }
    if (!parser_esperar(parser, TOK_CONSTANTE))
        return NULL;

    TipoDato tipo = parsear_tipo_dato(parser);

    // Crear un bloque para contener múltiples declaraciones
    NodoAST *bloque = crear_nodo_bloque(linea);
    int num_consts = 0;

    // Parsear primera constante
    do
    {
        if (!parser_coincidir(parser, TOK_VARIABLE))
        {
            parser_set_error(parser, "Se esperaba nombre de constante");
            return NULL;
        }

        char *nombre = strdup(parser->token_actual.valor + 1);
        parser_avanzar(parser);

        if (!parser_esperar(parser, TOK_IGUAL))
        {
            free(nombre);
            return NULL;
        }

        NodoAST *valor = parsear_expresion(parser);
        NodoAST *declaracion = crear_nodo_declaracion_constante(nombre, tipo, valor, linea);
        agregar_sentencia_a_bloque(bloque, declaracion);
        num_consts++;

    } while (parser_coincidir(parser, TOK_COMA) && (parser_avanzar(parser), true));

    // Si solo hay una constante, devolverla directamente
    if (num_consts == 1)
    {
        return bloque->datos.bloque.primera;
    }

    return bloque;
}

static NodoAST *parsear_declaracion_lista(Parser *parser)
{
    int linea = parser->token_actual.linea;

    if (parser_coincidir(parser, TOK_DECLARAR))
    {
        parser_avanzar(parser);
    }
    if (!parser_esperar(parser, TOK_LISTA))
        return NULL;

    TipoDato tipo = parsear_tipo_dato(parser);

    // Crear un bloque para contener múltiples declaraciones
    NodoAST *bloque = crear_nodo_bloque(linea);
    int num_listas = 0;

    do
    {
        if (!parser_coincidir(parser, TOK_VARIABLE))
        {
            parser_set_error(parser, "Se esperaba nombre de lista");
            return NULL;
        }

        char *nombre = strdup(parser->token_actual.valor + 1);
        parser_avanzar(parser);

        if (!parser_esperar(parser, TOK_CORCHETE_ABRIR))
        {
            free(nombre);
            return NULL;
        }

        // Parsear tamaño
        if (!parser_coincidir(parser, TOK_NUMERO_ENTERO))
        {
            parser_set_error(parser, "Se esperaba tamaño de lista");
            free(nombre);
            return NULL;
        }

        int tamano = atoi(parser->token_actual.valor);
        parser_avanzar(parser);

        if (!parser_esperar(parser, TOK_CORCHETE_CERRAR))
        {
            free(nombre);
            return NULL;
        }

        NodoAST *valores_iniciales = NULL;

        // Verificar si hay inicialización
        if (parser_coincidir(parser, TOK_IGUAL))
        {
            parser_avanzar(parser);

            // Verificar explícitamente si hay llave de apertura
            if (parser_coincidir(parser, TOK_LLAVE_ABRIR))
            {
                parser_avanzar(parser);
                valores_iniciales = crear_nodo_bloque(linea);

                // Cambiar: usar verificación sin consumo
                while (parser->token_actual.tipo != TOK_LLAVE_CERRAR && !parser->hay_error)
                {
                    NodoAST *valor = parsear_expresion(parser);
                    agregar_sentencia_a_bloque(valores_iniciales, valor);

                    if (parser_coincidir(parser, TOK_COMA))
                    {
                        parser_avanzar(parser);
                    }
                    else
                    {
                        break;
                    }
                }

                if (!parser_coincidir(parser, TOK_LLAVE_CERRAR))
                {
                    parser_set_error(parser, "Se esperaba }");
                    free(nombre);
                    return NULL;
                }
                parser_avanzar(parser);
            }
            else
            {
                parser_set_error(parser, "Se esperaba { para inicialización de lista");
                free(nombre);
                return NULL;
            }
        }

        NodoAST *declaracion = crear_nodo_declaracion_lista(nombre, tipo, tamano, valores_iniciales, linea);
        agregar_sentencia_a_bloque(bloque, declaracion);
        num_listas++;

    } while (parser_coincidir(parser, TOK_COMA) && (parser_avanzar(parser), true));

    // Si solo hay una lista, devolverla directamente
    if (num_listas == 1)
    {
        return bloque->datos.bloque.primera;
    }

    return bloque;
}

static NodoAST *parsear_declaracion_matriz(Parser *parser)
{
    int linea = parser->token_actual.linea;

    if (parser_coincidir(parser, TOK_DECLARAR))
    {
        parser_avanzar(parser);
    }
    if (!parser_esperar(parser, TOK_MATRIZ))
        return NULL;

    TipoDato tipo = parsear_tipo_dato(parser);

    // Crear un bloque para contener múltiples declaraciones
    NodoAST *bloque = crear_nodo_bloque(linea);
    int num_matrices = 0;

    do
    {
        if (!parser_coincidir(parser, TOK_VARIABLE))
        {
            parser_set_error(parser, "Se esperaba nombre de matriz");
            return NULL;
        }

        char *nombre = strdup(parser->token_actual.valor + 1);
        parser_avanzar(parser);

        // Parsear [filas]
        if (!parser_esperar(parser, TOK_CORCHETE_ABRIR))
        {
            free(nombre);
            return NULL;
        }

        if (!parser_coincidir(parser, TOK_NUMERO_ENTERO))
        {
            parser_set_error(parser, "Se esperaba número de filas");
            free(nombre);
            return NULL;
        }

        int filas = atoi(parser->token_actual.valor);
        parser_avanzar(parser);

        if (!parser_esperar(parser, TOK_CORCHETE_CERRAR))
        {
            free(nombre);
            return NULL;
        }

        // Parsear [columnas]
        if (!parser_esperar(parser, TOK_CORCHETE_ABRIR))
        {
            free(nombre);
            return NULL;
        }

        if (!parser_coincidir(parser, TOK_NUMERO_ENTERO))
        {
            parser_set_error(parser, "Se esperaba número de columnas");
            free(nombre);
            return NULL;
        }

        int columnas = atoi(parser->token_actual.valor);
        parser_avanzar(parser);

        if (!parser_esperar(parser, TOK_CORCHETE_CERRAR))
        {
            free(nombre);
            return NULL;
        }

        NodoAST *valores_iniciales = NULL;

        // Verificar si hay inicialización
        if (parser_coincidir(parser, TOK_IGUAL))
        {
            parser_avanzar(parser);
            if (parser_esperar(parser, TOK_LLAVE_ABRIR))
            {
                valores_iniciales = crear_nodo_bloque(linea);

                // Parsear filas
                while (!parser_coincidir(parser, TOK_LLAVE_CERRAR) && !parser->hay_error)
                {
                    if (parser_esperar(parser, TOK_LLAVE_ABRIR))
                    {
                        NodoAST *fila = crear_nodo_bloque(linea);

                        // Parsear columnas
                        while (!parser_coincidir(parser, TOK_LLAVE_CERRAR) && !parser->hay_error)
                        {
                            NodoAST *valor = parsear_expresion(parser);
                            agregar_sentencia_a_bloque(fila, valor);

                            if (parser_coincidir(parser, TOK_COMA))
                            {
                                parser_avanzar(parser);
                            }
                            else
                            {
                                break;
                            }
                        }

                        parser_esperar(parser, TOK_LLAVE_CERRAR);
                        agregar_sentencia_a_bloque(valores_iniciales, fila);
                    }

                    if (parser_coincidir(parser, TOK_COMA))
                    {
                        parser_avanzar(parser);
                    }
                    else
                    {
                        break;
                    }
                }

                parser_esperar(parser, TOK_LLAVE_CERRAR);
            }
        }

        NodoAST *declaracion = crear_nodo_declaracion_matriz(nombre, tipo, filas, columnas, valores_iniciales, linea);
        agregar_sentencia_a_bloque(bloque, declaracion);
        num_matrices++;

    } while (parser_coincidir(parser, TOK_COMA) && (parser_avanzar(parser), true));

    // Si solo hay una matriz, devolverla directamente
    if (num_matrices == 1)
    {
        return bloque->datos.bloque.primera;
    }

    return bloque;
}

// ============================================================
// PARSEO DE ESTRUCTURAS DE CONTROL
// ============================================================
// Parsear bloque dentro de SI, manejando correctamente SI anidados
static NodoAST *parsear_bloque_si(Parser *parser)
{
    int linea = parser->token_actual.linea;
    NodoAST *bloque = crear_nodo_bloque(linea);
    int nivel_si = 0;

    while (!parser->hay_error && parser->token_actual.tipo != TOK_EOF)
    {
        // Si encontramos FIN y estamos en nivel 0, terminamos
        if (parser->token_actual.tipo == TOK_FIN && nivel_si == 0)
        {
            break;
        }

        // Si encontramos SINO/SINOSI y estamos en nivel 0, terminamos
        if ((parser->token_actual.tipo == TOK_SINO || parser->token_actual.tipo == TOK_SINOSI) && nivel_si == 0)
        {
            break;
        }

        // Contar niveles de SI
        if (parser->token_actual.tipo == TOK_SI)
        {
            nivel_si++;
        }


        NodoAST *sentencia = parsear_sentencia(parser);
        if (sentencia)
        {
            agregar_sentencia_a_bloque(bloque, sentencia);

            // Si la sentencia es un SI completo, decrementar nivel
            if (sentencia->tipo == AST_SI)
            {
                nivel_si--;
            }
        }
        else
        {
            break;
        }
    }


    return bloque;
}

static NodoAST *parsear_si(Parser *parser)
{
    int linea = parser->token_actual.linea;
    parser_avanzar(parser); // Consumir SI

    if (!parser_esperar(parser, TOK_PARENTESIS_ABRIR))
        return NULL;

    NodoAST *condicion = parsear_expresion(parser);

    if (!parser_esperar(parser, TOK_PARENTESIS_CERRAR))
        return NULL;
    if (!parser_esperar(parser, TOK_ENTONCES))
        return NULL;

    // Parsear bloque_si
    NodoAST *bloque_si = parsear_bloque_si(parser);

    NodoAST *sino_si = NULL;
    NodoAST *bloque_sino = NULL;

    // Parsear SINO SI o SINOSI (ambas formas válidas)
    while (parser->token_actual.tipo == TOK_SINOSI ||
           (parser->token_actual.tipo == TOK_SINO &&
            parser->token_siguiente.tipo == TOK_SI &&
            parser->token_actual.linea == parser->token_siguiente.linea)) // ← Agregar esta condición
    {

        int linea_sino_si = parser->token_actual.linea;

        // Consumir los tokens apropiados
        if (parser->token_actual.tipo == TOK_SINOSI)
        {
            parser_avanzar(parser); // Consumir SINOSI
        }
        else
        {
            parser_avanzar(parser); // Consumir SINO
            parser_avanzar(parser); // Consumir SI
        }

        if (!parser_esperar(parser, TOK_PARENTESIS_ABRIR))
            return NULL;

        NodoAST *condicion_sino_si = parsear_expresion(parser);

        if (!parser_esperar(parser, TOK_PARENTESIS_CERRAR))
            return NULL;

        if (!parser_esperar(parser, TOK_ENTONCES))
            return NULL;

        NodoAST *bloque_sino_si = parsear_bloque(parser);

        NodoAST *nodo_sino_si = crear_nodo_sino_si(condicion_sino_si, bloque_sino_si, NULL, linea_sino_si);

        if (!sino_si)
        {
            sino_si = nodo_sino_si;
        }
        else
        {
            // Agregar al final de la cadena
            NodoAST *actual = sino_si;
            while (actual->datos.sino_si.siguiente_sino_si)
            {
                actual = actual->datos.sino_si.siguiente_sino_si;
            }
            actual->datos.sino_si.siguiente_sino_si = nodo_sino_si;
        }
    }


    // Parsear SINO
    if (parser_coincidir(parser, TOK_SINO))
    {
        parser_avanzar(parser);
        bloque_sino = parsear_bloque_si(parser);
    }


    if (!parser_esperar(parser, TOK_FIN))
        return NULL;
    if (!parser_esperar(parser, TOK_SI))
        return NULL;

    return crear_nodo_si(condicion, bloque_si, sino_si, bloque_sino, linea);
}
static NodoAST *parsear_para(Parser *parser)
{
    int linea = parser->token_actual.linea;
    parser_avanzar(parser); // Consumir PARA

    // Paréntesis opcionales
    bool tiene_parentesis = false;
    if (parser_coincidir(parser, TOK_PARENTESIS_ABRIR))
    {
        parser_avanzar(parser);
        tiene_parentesis = true;
    }

    if (!parser_coincidir(parser, TOK_VARIABLE))
    {
        parser_set_error(parser, "Se esperaba variable de control");
        return NULL;
    }

    char *variable = strdup(parser->token_actual.valor + 1);
    parser_avanzar(parser);

    // Parsear DESDE
    if (parser_coincidir(parser, TOK_DESDE))
    {
        parser_avanzar(parser);
    }
    else if (parser_coincidir(parser, TOK_IGUAL))
    {
        parser_avanzar(parser);
    }

    NodoAST *inicio = parsear_expresion(parser);

    // Parsear HASTA
    if (!parser_esperar(parser, TOK_HASTA))
    {
        free(variable);
        return NULL;
    }

    NodoAST *fin = parsear_expresion(parser);

    // Parsear PASO (opcional)
    NodoAST *paso = NULL;
    if (parser_coincidir(parser, TOK_PASO))
    {
        parser_avanzar(parser);
        paso = parsear_expresion(parser);
    }

    // Cerrar paréntesis si los abrió
    if (tiene_parentesis)
    {
        if (!parser_esperar(parser, TOK_PARENTESIS_CERRAR))
        {
            free(variable);
            return NULL;
        }
    }

    if (!parser_esperar(parser, TOK_HACER))
    {
        free(variable);
        return NULL;
    }

    NodoAST *bloque = parsear_bloque(parser);

    if (!parser_esperar(parser, TOK_FIN))
    {
        free(variable);
        return NULL;
    }
    if (!parser_esperar(parser, TOK_PARA))
    {
        free(variable);
        return NULL;
    }

    return crear_nodo_para(variable, inicio, fin, paso, bloque, linea);
}

static NodoAST* parsear_mientras(Parser* parser) {
    int linea = parser->token_actual.linea;
    parser_avanzar(parser); // Consumir MIENTRAS
    
    if (!parser_esperar(parser, TOK_PARENTESIS_ABRIR)) return NULL;
    
    NodoAST* condicion = parsear_expresion(parser);
    
    if (!parser_esperar(parser, TOK_PARENTESIS_CERRAR)) return NULL;
    if (!parser_esperar(parser, TOK_HACER)) return NULL;
    
    NodoAST* bloque = parsear_bloque(parser);
    
    if (!parser_esperar(parser, TOK_FIN)) return NULL;
    if (!parser_esperar(parser, TOK_MIENTRAS)) return NULL;
    
    return crear_nodo_mientras(condicion, bloque, linea);
}

static NodoAST *parsear_realizar(Parser *parser)
{
    int linea = parser->token_actual.linea;
    parser_avanzar(parser); // Consumir REALIZAR

    // Usar parsear_bloque_hasta para que se detenga en MIENTRAS
    NodoAST *bloque = parsear_bloque_hasta(parser, TOK_MIENTRAS);

    if (!parser_esperar(parser, TOK_MIENTRAS))
        return NULL;
    if (!parser_esperar(parser, TOK_PARENTESIS_ABRIR))
        return NULL;

    NodoAST *condicion = parsear_expresion(parser);

    if (!parser_esperar(parser, TOK_PARENTESIS_CERRAR))
        return NULL;

    return crear_nodo_realizar(bloque, condicion, linea);
}

static NodoAST *parsear_segun_caso(Parser *parser)
{
    int linea = parser->token_actual.linea;
    parser_avanzar(parser); // Consumir SEGUN

    if (!parser_esperar(parser, TOK_CASO))
        return NULL;

    if (!parser_esperar(parser, TOK_PARENTESIS_ABRIR))
        return NULL;

    NodoAST *expresion = parsear_expresion(parser);

    if (!parser_esperar(parser, TOK_PARENTESIS_CERRAR))
        return NULL;

    NodoAST *casos = crear_nodo_bloque(linea);

    // Parsear casos hasta encontrar FIN SEGUN
    while (!parser->hay_error && !parser_coincidir(parser, TOK_FIN))
    {
        if (parser_coincidir(parser, TOK_CASO))
        {
            int linea_caso = parser->token_actual.linea;
            parser_avanzar(parser);

            if (!parser_coincidir(parser, TOK_NUMERO_ENTERO))
            {
                parser_set_error(parser, "Se esperaba valor numérico en CASO");
                return NULL;
            }

            int valor = atoi(parser->token_actual.valor);
            parser_avanzar(parser);

            NodoAST *bloque_caso = parsear_bloque(parser);

            NodoAST *nodo_caso = crear_nodo_caso(valor, bloque_caso, linea_caso);
            agregar_sentencia_a_bloque(casos, nodo_caso);
        }
        else if (parser_coincidir(parser, TOK_POR))
        {
            int linea_defecto = parser->token_actual.linea;
            parser_avanzar(parser);

            if (!parser_esperar(parser, TOK_DEFECTO))
                return NULL;

            NodoAST *bloque_defecto = parsear_bloque(parser);
            
            NodoAST *nodo_defecto = crear_nodo_por_defecto(bloque_defecto, linea_defecto);
            agregar_sentencia_a_bloque(casos, nodo_defecto);
        }
        else
        {
            parser_set_error(parser, "Se esperaba CASO o POR DEFECTO");
            return NULL;
        }
    }

    if (!parser_esperar(parser, TOK_FIN))
        return NULL;
    if (!parser_esperar(parser, TOK_SEGUN))
        return NULL;

    return crear_nodo_segun_caso(expresion, casos, linea);
}

// ============================================================
// PARSEO DE COMANDOS
// ============================================================
static NodoAST *parsear_escribir(Parser *parser)
{
    int linea = parser->token_actual.linea;
    parser_avanzar(parser); // Consumir ESCRIBIR o MOSTRAR

    if (!parser_esperar(parser, TOK_PARENTESIS_ABRIR))
        return NULL;

    NodoAST *expresion = parsear_expresion(parser);

    // Verificar si hay segundo argumento: DECIMALES(n1, n2, ...)
    int *formatos = NULL;
    int num_formatos = 0;

    if (parser_coincidir(parser, TOK_COMA))
    {
        parser_avanzar(parser); // Consumir la coma

        if (parser_coincidir(parser, TOK_DECIMALES))
        {
            parser_avanzar(parser); // Consumir DECIMALES

            if (!parser_esperar(parser, TOK_PARENTESIS_ABRIR))
            {
                liberar_nodo(expresion);
                return NULL;
            }

            // Parsear lista de números
            int formatos_buffer[32];
            int count = 0;

            while (!parser_coincidir(parser, TOK_PARENTESIS_CERRAR) && !parser->hay_error && count < 32)
            {
                if (!parser_coincidir(parser, TOK_NUMERO_ENTERO))
                {
                    parser_set_error(parser, "Se esperaba número entero");
                    liberar_nodo(expresion);
                    return NULL;
                }
                formatos_buffer[count++] = atoi(parser->token_actual.valor);
                parser_avanzar(parser);

                if (parser_coincidir(parser, TOK_COMA))
                {
                    parser_avanzar(parser);
                }
                else
                {
                    break;
                }
            }

            if (!parser_esperar(parser, TOK_PARENTESIS_CERRAR))
            {
                liberar_nodo(expresion);
                return NULL;
            }

            num_formatos = count;
            if (count > 0)
            {
                formatos = malloc(sizeof(int) * count);
                for (int i = 0; i < count; i++)
                {
                    formatos[i] = formatos_buffer[i];
                }
            }
        }
        else
        {
            parser_set_error(parser, "Se esperaba DECIMALES(n1, n2, ...)");
            liberar_nodo(expresion);
            return NULL;
        }
    }

    if (!parser_esperar(parser, TOK_PARENTESIS_CERRAR))
    {
        liberar_nodo(expresion);
        if (formatos)
            free(formatos);
        return NULL;
    }

    bool salto_linea = false;
    if (parser_coincidir(parser, TOK_SALTO))
    {
        salto_linea = true;
        parser_avanzar(parser);
    }
    else if (parser_coincidir(parser, TOK_SIN_SALTO))
    {
        salto_linea = false;
        parser_avanzar(parser);
    }

    NodoAST *nodo = crear_nodo_escribir(expresion, salto_linea, linea);
    if (formatos && num_formatos > 0)
    {
        nodo_escribir_set_formatos(nodo, formatos, num_formatos);
        free(formatos); // El nodo ya hizo su copia
    }
    return nodo;
}

static NodoAST* parsear_leer(Parser* parser) {
    int linea = parser->token_actual.linea;
    parser_avanzar(parser); // Consumir LEER
    
    if (!parser_esperar(parser, TOK_PARENTESIS_ABRIR)) return NULL;
    
    if (!parser_coincidir(parser, TOK_VARIABLE)) {
        parser_set_error(parser, "Se esperaba variable");
        return NULL;
    }
    
    char* variable = strdup(parser->token_actual.valor + 1);
    parser_avanzar(parser);
    
    if (!parser_esperar(parser, TOK_PARENTESIS_CERRAR)) {
        free(variable);
        return NULL;
    }
    
    return crear_nodo_leer(variable, linea);
}

static NodoAST *parsear_incluir(Parser *parser)
{
    int linea = parser->token_actual.linea;
    parser_avanzar(parser); // Consumir INCLUIR

    if (!parser_coincidir(parser, TOK_TEXTO))
    {
        parser_set_error(parser, "Se esperaba ruta de archivo entre comillas");
        return NULL;
    }

    char *ruta = strdup(parser->token_actual.valor);
    parser_avanzar(parser);

    return crear_nodo_incluir(ruta, linea);
}

static NodoAST *parsear_calcular(Parser *parser)
{
    int linea = parser->token_actual.linea;
    parser_avanzar(parser); // Consumir CALCULAR

    if (!parser_esperar(parser, TOK_EN))
        return NULL;

    if (!parser_coincidir(parser, TOK_VARIABLE))
    {
        parser_set_error(parser, "Se esperaba variable destino");
        return NULL;
    }

    char *variable = strdup(parser->token_actual.valor + 1);
    parser_avanzar(parser);

    // Verificar si es acceso a elemento de lista/matriz
    NodoAST *destino = NULL;
    if (parser_coincidir(parser, TOK_CORCHETE_ABRIR))
    {
        parser_avanzar(parser);
        NodoAST *indice1 = parsear_expresion(parser);
        if (!parser_esperar(parser, TOK_CORCHETE_CERRAR))
        {
            free(variable);
            liberar_nodo(indice1);
            return NULL;
        }

        // Verificar si hay un segundo índice (matriz)
        if (parser_coincidir(parser, TOK_CORCHETE_ABRIR))
        {
            parser_avanzar(parser);
            NodoAST *indice2 = parsear_expresion(parser);
            if (!parser_esperar(parser, TOK_CORCHETE_CERRAR))
            {
                free(variable);
                liberar_nodo(indice1);
                liberar_nodo(indice2);
                return NULL;
            }
            destino = crear_nodo_acceso_matriz(variable, indice1, indice2, linea);
        }
        else
        {
            // Es lista (un solo índice)
            destino = crear_nodo_acceso_lista(variable, indice1, linea);
        }
    }
    else
    {
        destino = crear_nodo_variable(variable, linea);
    }

    if (!parser_esperar(parser, TOK_IGUAL))
    {
        return NULL;
    }

    NodoAST *expresion = parsear_expresion(parser);

    return crear_nodo_calcular_nodo(destino, expresion, linea);
}

static NodoAST *parsear_asignar(Parser *parser)
{
    int linea = parser->token_actual.linea;
    parser_avanzar(parser); // Consumir ASIGNAR

    if (!parser_esperar(parser, TOK_EN))
        return NULL;

    if (!parser_coincidir(parser, TOK_VARIABLE))
    {
        parser_set_error(parser, "Se esperaba variable destino");
        return NULL;
    }

    char *variable = strdup(parser->token_actual.valor + 1);
    parser_avanzar(parser);

    // Verificar si es acceso a elemento de lista/matriz
    NodoAST *destino = NULL;
    if (parser_coincidir(parser, TOK_CORCHETE_ABRIR))
    {
        parser_avanzar(parser);
        NodoAST *indice1 = parsear_expresion(parser);
        if (!parser_esperar(parser, TOK_CORCHETE_CERRAR))
        {
            free(variable);
            return NULL;
        }

        // Verificar si hay un segundo corchete (matriz)
        if (parser_coincidir(parser, TOK_CORCHETE_ABRIR))
        {
            parser_avanzar(parser);
            NodoAST *indice2 = parsear_expresion(parser);
            if (!parser_esperar(parser, TOK_CORCHETE_CERRAR))
            {
                free(variable);
                return NULL;
            }
            // Es acceso a matriz [fila][columna]
            destino = crear_nodo_acceso_matriz(variable, indice1, indice2, linea);
        }
        else
        {
            // Es acceso a lista [indice]
            destino = crear_nodo_acceso_lista(variable, indice1, linea);
        }
    }
    else
    {
        destino = crear_nodo_variable(variable, linea);
    }

    if (!parser_esperar(parser, TOK_IGUAL))
    {
        return NULL;
    }

    NodoAST *valor = parsear_expresion(parser);

    return crear_nodo_asignar_nodo(destino, valor, linea);
}

static NodoAST *parsear_retornar(Parser *parser)
{
    int linea = parser->token_actual.linea;
    parser_avanzar(parser); // Consumir RETORNAR

    // Parsear expresión completa (no solo variable)
    NodoAST *expresion = parsear_expresion(parser);

    return crear_nodo_retornar(expresion, linea);
}

static NodoAST *parsear_llamar_a(Parser *parser)
{
    int linea = parser->token_actual.linea;
    parser_avanzar(parser); // Consumir LLAMAR

    if (!parser_esperar(parser, TOK_A))
        return NULL;

    // Nombre del subprograma (SIN $, es un identificador)
    if (!parser_coincidir(parser, TOK_IDENTIFICADOR))
    {
        parser_set_error(parser, "Se esperaba nombre de subprograma");
        return NULL;
    }
    char *nombre = strdup(parser->token_actual.valor);
    parser_avanzar(parser);

    // Parámetros opcionales
    NodoAST *argumentos = NULL;

    if (parser_coincidir(parser, TOK_PARENTESIS_ABRIR))
    {
        parser_avanzar(parser);

        argumentos = crear_nodo_bloque(linea);

        if (!parser_coincidir(parser, TOK_PARENTESIS_CERRAR))
        {
            NodoAST *arg = parsear_expresion(parser);
            agregar_sentencia_a_bloque(argumentos, arg);

            while (parser_coincidir(parser, TOK_COMA))
            {
                parser_avanzar(parser);
                arg = parsear_expresion(parser);
                agregar_sentencia_a_bloque(argumentos, arg);
            }

            if (!parser_esperar(parser, TOK_PARENTESIS_CERRAR))
            {
                free(nombre);
                return NULL;
            }
        }
        else
        {
            parser_avanzar(parser); // Consumir )
        }
    }

    return crear_nodo_llamar_a(nombre, argumentos, linea);
}

// ============================================================
// PARSEO DE COMANDOS ADICIONALES
// ============================================================

static NodoAST *parsear_comando_simple(Parser *parser, TipoToken tipo_comando)
{
    int linea = parser->token_actual.linea;
    parser_avanzar(parser); // Consumir el comando

    // Comandos que no llevan paréntesis
    if (tipo_comando == TOK_TEXTONEGRITA)
    {
        return crear_nodo_textonegrita(linea);
    }
    if (tipo_comando == TOK_TEXTOCURSIVA)
    {
        return crear_nodo_textocursiva(linea);
    }
    if (tipo_comando == TOK_TEXTOSUBRAYADO)
    {
        return crear_nodo_textosubrayado(linea);
    }
    if (tipo_comando == TOK_RESETTEXTO)
    {
        return crear_nodo_resettexto(linea);
    }
    if (tipo_comando == TOK_RESETCOLOR)
    {
        return crear_nodo_resetcolor(linea);
    }
    if (tipo_comando == TOK_LIMPIARPANTALLA)
    {
        return crear_nodo_limpiarpantalla(linea);
    }
    if (tipo_comando == TOK_OCULTARCURSOR)
    {
        return crear_nodo_ocultarcursor(linea);
    }
    if (tipo_comando == TOK_MOSTRARCURSOR)
    {
        return crear_nodo_mostrarcursor(linea);
    }

    // Comandos que sí pueden llevar paréntesis (HORAACTUAL, FECHAACTUAL)
    if (parser_coincidir(parser, TOK_PARENTESIS_ABRIR))
    {
        parser_avanzar(parser);

        NodoAST *args = crear_nodo_bloque(linea);

        if (!parser_coincidir(parser, TOK_PARENTESIS_CERRAR))
        {
            NodoAST *arg = parsear_expresion(parser);
            agregar_sentencia_a_bloque(args, arg);

            while (parser_coincidir(parser, TOK_COMA))
            {
                parser_avanzar(parser);
                arg = parsear_expresion(parser);
                agregar_sentencia_a_bloque(args, arg);
            }

            if (!parser_esperar(parser, TOK_PARENTESIS_CERRAR))
            {
                return NULL;
            }
        }

        switch (tipo_comando)
        {
        case TOK_HORAACTUAL:
        {
            if (args->datos.bloque.primera &&
                args->datos.bloque.primera->tipo == AST_VARIABLE)
            {
                char *var = strdup(args->datos.bloque.primera->datos.variable.nombre);
                return crear_nodo_horaactual(var, linea);
            }
            return crear_nodo_horaactual("", linea);
        }
        case TOK_FECHAACTUAL:
        {
            if (args->datos.bloque.primera &&
                args->datos.bloque.primera->tipo == AST_VARIABLE)
            {
                char *var = strdup(args->datos.bloque.primera->datos.variable.nombre);
                return crear_nodo_fechaactual(var, linea);
            }
            return crear_nodo_fechaactual("", linea);
        }
        default:
            return args;
        }
    }

    parser_set_error(parser, "Comando no implementado");
    return NULL;
}

static NodoAST *parsear_comando_color(Parser *parser, bool es_fondo)
{
    int linea = parser->token_actual.linea;
    parser_avanzar(parser); // Consumir COLORTEXTO o COLORFONDO

    if (!parser_esperar(parser, TOK_PARENTESIS_ABRIR))
        return NULL;

    // El color puede ser:
    // 1. Una palabra clave de color (TOK_VERDE, TOK_ROJO, etc.)
    // 2. Un identificador
    // 3. Un string
    char *color = NULL;

    if (parser_coincidir(parser, TOK_NEGRO) || parser_coincidir(parser, TOK_ROJO) ||
        parser_coincidir(parser, TOK_VERDE) || parser_coincidir(parser, TOK_AMARILLO) ||
        parser_coincidir(parser, TOK_AZUL) || parser_coincidir(parser, TOK_MAGENTA) ||
        parser_coincidir(parser, TOK_CYAN) || parser_coincidir(parser, TOK_BLANCO) ||
        parser_coincidir(parser, TOK_GRIS) || parser_coincidir(parser, TOK_ROJOCLARO) ||
        parser_coincidir(parser, TOK_VERDECLARO) || parser_coincidir(parser, TOK_AMARILLOCLARO) ||
        parser_coincidir(parser, TOK_AZULCLARO) || parser_coincidir(parser, TOK_MAGENTACLARO) ||
        parser_coincidir(parser, TOK_CYANCLARO) || parser_coincidir(parser, TOK_BLANCOCLARO))
    {

        // Es una palabra clave de color, convertir a string
        color = strdup(parser->token_actual.valor);
        parser_avanzar(parser);
    }
    else if (parser_coincidir(parser, TOK_IDENTIFICADOR))
    {
        color = strdup(parser->token_actual.valor);
        parser_avanzar(parser);
    }
    else if (parser_coincidir(parser, TOK_TEXTO))
    {
        color = strdup(parser->token_actual.valor);
        parser_avanzar(parser);
    }
    else
    {
        parser_set_error(parser, "Se esperaba nombre de color");
        return NULL;
    }

    if (!parser_esperar(parser, TOK_PARENTESIS_CERRAR))
    {
        free(color);
        return NULL;
    }

    if (es_fondo)
    {
        return crear_nodo_colorfondo(color, linea);
    }
    else
    {
        return crear_nodo_colortexto(color, linea);
    }
}

static NodoAST *parsear_comando_esperar(Parser *parser)
{
    int linea = parser->token_actual.linea;
    parser_avanzar(parser); // Consumir ESPERAR

    if (!parser_esperar(parser, TOK_PARENTESIS_ABRIR))
        return NULL;

    NodoAST *cantidad = parsear_expresion(parser);

    if (!parser_esperar(parser, TOK_COMA))
        return NULL;

    // Parsear unidad de tiempo (puede ser palabra clave o identificador)
    char *unidad = NULL;

    if (parser_coincidir(parser, TOK_MICROSEGUNDOS) || parser_coincidir(parser, TOK_MICROS) ||
        parser_coincidir(parser, TOK_US) || parser_coincidir(parser, TOK_MILISEGUNDOS) ||
        parser_coincidir(parser, TOK_MS) || parser_coincidir(parser, TOK_SEGUNDOS) ||
        parser_coincidir(parser, TOK_S) || parser_coincidir(parser, TOK_MINUTOS) ||
        parser_coincidir(parser, TOK_MIN))
    {

        unidad = strdup(parser->token_actual.valor);
        parser_avanzar(parser);
    }
    else if (parser_coincidir(parser, TOK_IDENTIFICADOR))
    {
        unidad = strdup(parser->token_actual.valor);
        parser_avanzar(parser);
    }
    else
    {
        parser_set_error(parser, "Se esperaba unidad de tiempo");
        return NULL;
    }

    if (!parser_esperar(parser, TOK_PARENTESIS_CERRAR))
    {
        free(unidad);
        return NULL;
    }

    return crear_nodo_esperar(cantidad, unidad, linea);
}

static NodoAST* parsear_comando_archivo(Parser* parser, TipoToken tipo_comando) {
    int linea = parser->token_actual.linea;
    parser_avanzar(parser); // Consumir el comando
    
    if (!parser_esperar(parser, TOK_PARENTESIS_ABRIR)) return NULL;
    
    if (!parser_coincidir(parser, TOK_VARIABLE)) {
        parser_set_error(parser, "Se esperaba variable de archivo");
        return NULL;
    }
    
    char* variable = strdup(parser->token_actual.valor + 1);
    parser_avanzar(parser);
    
    if (!parser_esperar(parser, TOK_PARENTESIS_CERRAR)) {
        free(variable);
        return NULL;
    }
    
    switch (tipo_comando) {
        case TOK_CERRARARCHIVO:
            return crear_nodo_cerrararchivo(variable, linea);
        default:
            free(variable);
            parser_set_error(parser, "Comando de archivo no implementado");
            return NULL;
    }
}

// ============================================================
// PARSEO DE BLOQUES Y SENTENCIAS
// ============================================================
static NodoAST* parsear_sentencia(Parser* parser) {
    int linea = parser->token_actual.linea;

    // Si encontramos un token de terminación de bloque, devolver NULL
    if (parser->token_actual.tipo == TOK_SINO ||
        parser->token_actual.tipo == TOK_SINOSI ||
        parser->token_actual.tipo == TOK_CASO ||
        parser->token_actual.tipo == TOK_DEFECTO ||
        parser->token_actual.tipo == TOK_FIN ||
        parser->token_actual.tipo == TOK_EOF)
    {
        return NULL;
    }

    // Declaraciones
    if (parser->token_actual.tipo == TOK_VARIABLE_KW ||
        (parser->token_actual.tipo == TOK_DECLARAR && parser_verificar_siguiente(parser, TOK_VARIABLE_KW)))
    {
        return parsear_declaracion_variable(parser);
    }

    if (parser->token_actual.tipo == TOK_CONSTANTE ||
        (parser->token_actual.tipo == TOK_DECLARAR && parser_verificar_siguiente(parser, TOK_CONSTANTE)))
    {
        return parsear_declaracion_constante(parser);
    }

    if (parser->token_actual.tipo == TOK_LISTA ||
        (parser->token_actual.tipo == TOK_DECLARAR && parser_verificar_siguiente(parser, TOK_LISTA)))
    {
        return parsear_declaracion_lista(parser);
    }

    if (parser->token_actual.tipo == TOK_MATRIZ ||
        (parser->token_actual.tipo == TOK_DECLARAR && parser_verificar_siguiente(parser, TOK_MATRIZ)))
    {
        return parsear_declaracion_matriz(parser);
    }

    // Estructuras de control
    if (parser_coincidir(parser, TOK_SI)) {
        return parsear_si(parser);
    }
    
    if (parser_coincidir(parser, TOK_PARA)) {
        return parsear_para(parser);
    }
    
    if (parser_coincidir(parser, TOK_MIENTRAS)) {
        return parsear_mientras(parser);
    }
    
    if (parser_coincidir(parser, TOK_REALIZAR)) {
        return parsear_realizar(parser);
    }
    
    if (parser_coincidir(parser, TOK_SEGUN)) {
        return parsear_segun_caso(parser);
    }
    
    // Funciones y subprogramas
    if (parser_coincidir(parser, TOK_FUNCION)) {
        return parsear_funcion(parser);
    }
    
    if (parser_coincidir(parser, TOK_SUBPROGRAMA)) {
        return parsear_subprograma(parser);
    }
    
    // Comandos de E/S
    if (parser_coincidir(parser, TOK_ESCRIBIR) || parser_coincidir(parser, TOK_MOSTRAR)) {
        return parsear_escribir(parser);
    }

    if (parser_coincidir(parser, TOK_INCLUIR))
    {
        return parsear_incluir(parser);
    }

    if (parser_coincidir(parser, TOK_LEER)) {
        return parsear_leer(parser);
    }
    
    if (parser_coincidir(parser, TOK_CALCULAR)) {
        return parsear_calcular(parser);
    }
    
    if (parser_coincidir(parser, TOK_ASIGNAR)) {
        return parsear_asignar(parser);
    }

    if (parser_coincidir(parser, TOK_RESULTADO))
    {
        return parsear_calcular(parser); // RESULTADO es alias de CALCULAR
    }

    // Funciones de texto que modifican variables (COPIARTEXTO, CONCATENARTEXTO, etc.)
    if (parser->token_actual.tipo >= TOK_COPIARTEXTO && parser->token_actual.tipo <= TOK_INVERTIRTEXTO)
    {
        int linea = parser->token_actual.linea;
        char *nombre = strdup(parser->token_actual.valor);
        parser_avanzar(parser); // Consumir nombre de la función

        if (!parser_esperar(parser, TOK_PARENTESIS_ABRIR))
        {
            free(nombre);
            return NULL;
        }

        NodoAST *argumentos = crear_nodo_bloque(linea);

        if (!parser_coincidir(parser, TOK_PARENTESIS_CERRAR))
        {
            NodoAST *arg = parsear_expresion(parser);
            agregar_sentencia_a_bloque(argumentos, arg);

            while (parser_coincidir(parser, TOK_COMA))
            {
                parser_avanzar(parser);
                arg = parsear_expresion(parser);
                agregar_sentencia_a_bloque(argumentos, arg);
            }

            if (!parser_esperar(parser, TOK_PARENTESIS_CERRAR))
            {
                free(nombre);
                return NULL;
            }
        }

        return crear_nodo_llamada_funcion_modificadora(nombre, argumentos, linea);
    }

    if (parser_coincidir(parser, TOK_RETORNAR)) {
        return parsear_retornar(parser);
    }
    
    if (parser_coincidir(parser, TOK_LLAMAR)) {
        return parsear_llamar_a(parser);
    }
    
    if (parser_coincidir(parser, TOK_CORTE)) {
        parser_avanzar(parser);
        return crear_nodo_corte(linea);
    }
    
    // Comandos de tiempo
    if (parser_coincidir(parser, TOK_HORAACTUAL)) {
        return parsear_comando_simple(parser, TOK_HORAACTUAL);
    }
    
    if (parser_coincidir(parser, TOK_FECHAACTUAL)) {
        return parsear_comando_simple(parser, TOK_FECHAACTUAL);
    }
    
    if (parser_coincidir(parser, TOK_ESPERAR)) {
        return parsear_comando_esperar(parser);
    }
    
    // Comandos de formato
    if (parser_coincidir(parser, TOK_COLORTEXTO)) {
        return parsear_comando_color(parser, false);
    }
    
    if (parser_coincidir(parser, TOK_COLORFONDO)) {
        return parsear_comando_color(parser, true);
    }

    // Comandos simples sin argumentos
    if (parser_coincidir(parser, TOK_LIMPIARPANTALLA) ||
        parser_coincidir(parser, TOK_TEXTONEGRITA) ||
        parser_coincidir(parser, TOK_TEXTOCURSIVA) ||
        parser_coincidir(parser, TOK_TEXTOSUBRAYADO) ||
        parser_coincidir(parser, TOK_RESETTEXTO) ||
        parser_coincidir(parser, TOK_RESETCOLOR) ||
        parser_coincidir(parser, TOK_OCULTARCURSOR) ||
        parser_coincidir(parser, TOK_MOSTRARCURSOR))
    {
        return parsear_comando_simple(parser, parser->token_actual.tipo);
    }

    // Comandos de archivos
    if (parser_coincidir(parser, TOK_CERRARARCHIVO)) {
        return parsear_comando_archivo(parser, TOK_CERRARARCHIVO);
    }
    // ESCRIBIRARCHIVO
    if (parser_coincidir(parser, TOK_ESCRIBIRARCHIVO))
    {
        int linea = parser->token_actual.linea;
        parser_avanzar(parser);
        if (!parser_esperar(parser, TOK_PARENTESIS_ABRIR))
            return NULL;

        // Variable de archivo
        if (!parser_coincidir(parser, TOK_VARIABLE))
        {
            parser_set_error(parser, "Se esperaba variable de archivo");
            return NULL;
        }
        char *var = strdup(parser->token_actual.valor + 1);
        parser_avanzar(parser);

        if (!parser_esperar(parser, TOK_COMA))
        {
            free(var);
            return NULL;
        }

        // Contenido a escribir
        NodoAST *contenido = parsear_expresion(parser);
        if (!contenido)
        {
            free(var);
            return NULL;
        }

        if (!parser_esperar(parser, TOK_PARENTESIS_CERRAR))
        {
            free(var);
            liberar_nodo(contenido);
            return NULL;
        }

        return crear_nodo_escribirarchivo(var, contenido, linea);
    }

    // LEERARCHIVO
    if (parser_coincidir(parser, TOK_LEERARCHIVO))
    {
        int linea = parser->token_actual.linea;
        parser_avanzar(parser);
        if (!parser_esperar(parser, TOK_PARENTESIS_ABRIR))
            return NULL;

        // Variable de archivo
        if (!parser_coincidir(parser, TOK_VARIABLE))
        {
            parser_set_error(parser, "Se esperaba variable de archivo");
            return NULL;
        }
        char *var_arch = strdup(parser->token_actual.valor + 1);
        parser_avanzar(parser);

        if (!parser_esperar(parser, TOK_COMA))
        {
            free(var_arch);
            return NULL;
        }

        // Variable destino
        if (!parser_coincidir(parser, TOK_VARIABLE))
        {
            parser_set_error(parser, "Se esperaba variable destino");
            free(var_arch);
            return NULL;
        }
        char *var_dest = strdup(parser->token_actual.valor + 1);
        parser_avanzar(parser);

        if (!parser_esperar(parser, TOK_PARENTESIS_CERRAR))
        {
            free(var_arch);
            free(var_dest);
            return NULL;
        }

        return crear_nodo_leerarchivo(var_arch, var_dest, linea);
    }

    if (parser_coincidir(parser, TOK_VARIABLE))
    {
        char *variable = strdup(parser->token_actual.valor + 1);
        parser_avanzar(parser);

        // Verificar si es acceso a lista/matriz: $variable[indice] o $variable[fila][columna]
        if (parser_coincidir(parser, TOK_CORCHETE_ABRIR))
        {
            parser_avanzar(parser);
            NodoAST *indice1 = parsear_expresion(parser);

            if (!parser_esperar(parser, TOK_CORCHETE_CERRAR))
            {
                free(variable);
                liberar_nodo(indice1);
                return NULL;
            }

            // Verificar si hay un segundo índice (matriz)
            if (parser_coincidir(parser, TOK_CORCHETE_ABRIR))
            {
                parser_avanzar(parser);
                NodoAST *indice2 = parsear_expresion(parser);

                if (!parser_esperar(parser, TOK_CORCHETE_CERRAR))
                {
                    free(variable);
                    liberar_nodo(indice1);
                    liberar_nodo(indice2);
                    return NULL;
                }

                if (!parser_esperar(parser, TOK_IGUAL))
                {
                    free(variable);
                    liberar_nodo(indice1);
                    liberar_nodo(indice2);
                    return NULL;
                }

                NodoAST *valor = parsear_expresion(parser);
                NodoAST *destino = crear_nodo_acceso_matriz(variable, indice1, indice2, linea);
                return crear_nodo_asignar_nodo(destino, valor, linea);
            }
            else
            {
                // Es lista (un solo índice)
                if (!parser_esperar(parser, TOK_IGUAL))
                {
                    free(variable);
                    liberar_nodo(indice1);
                    return NULL;
                }

                NodoAST *valor = parsear_expresion(parser);
                NodoAST *destino = crear_nodo_acceso_lista(variable, indice1, linea);
                return crear_nodo_asignar_nodo(destino, valor, linea);
            }
        }

        // Asignación simple: $variable = valor
        if (parser_coincidir(parser, TOK_IGUAL))
        {
            parser_avanzar(parser);
            NodoAST *valor = parsear_expresion(parser);
            NodoAST *destino = crear_nodo_variable(variable, linea);
            return crear_nodo_asignar_nodo(destino, valor, linea);
        }

        free(variable);
        parser_set_error(parser, "Se esperaba '=' después de variable");
        return NULL;
    }

    // Comandos de entrada
    if (parser_coincidir(parser, TOK_LEERCARACTER)) {
        int linea = parser->token_actual.linea;
        parser_avanzar(parser);
        if (!parser_esperar(parser, TOK_PARENTESIS_ABRIR)) return NULL;
        if (!parser_coincidir(parser, TOK_VARIABLE)) {
            parser_set_error(parser, "Se esperaba variable");
            return NULL;
        }
        char* var = strdup(parser->token_actual.valor + 1);
        parser_avanzar(parser);
        if (!parser_esperar(parser, TOK_PARENTESIS_CERRAR)) {
            free(var);
            return NULL;
        }
        return crear_nodo_leercaracter(var, linea);
    }

    if (parser_coincidir(parser, TOK_LEERHASTA))
    {
        int linea = parser->token_actual.linea;
        parser_avanzar(parser);
        if (!parser_esperar(parser, TOK_PARENTESIS_ABRIR))
            return NULL;

        // Variable destino
        if (!parser_coincidir(parser, TOK_VARIABLE))
        {
            parser_set_error(parser, "Se esperaba variable");
            return NULL;
        }
        char *var = strdup(parser->token_actual.valor + 1);
        parser_avanzar(parser);

        if (!parser_esperar(parser, TOK_COMA))
        {
            free(var);
            return NULL;
        }

        // Delimitador (texto)
        NodoAST *delimitador = parsear_expresion(parser);
        if (!delimitador)
        {
            free(var);
            return NULL;
        }

        if (!parser_esperar(parser, TOK_PARENTESIS_CERRAR))
        {
            free(var);
            liberar_nodo(delimitador);
            return NULL;
        }

        return crear_nodo_leerhasta(var, delimitador, linea);
    }

    // DIBUJARLINEA(x1, y1, x2, y2, patron)
    if (parser_coincidir(parser, TOK_DIBUJARLINEA))
    {
        int linea = parser->token_actual.linea;
        parser_avanzar(parser);
        if (!parser_esperar(parser, TOK_PARENTESIS_ABRIR))
            return NULL;

        NodoAST *x1 = parsear_expresion(parser);
        if (!parser_esperar(parser, TOK_COMA))
            return NULL;

        NodoAST *y1 = parsear_expresion(parser);
        if (!parser_esperar(parser, TOK_COMA))
            return NULL;

        NodoAST *x2 = parsear_expresion(parser);
        if (!parser_esperar(parser, TOK_COMA))
            return NULL;

        NodoAST *y2 = parsear_expresion(parser);
        if (!parser_esperar(parser, TOK_COMA))
            return NULL;

        // Patron (puede ser texto literal o variable)
        NodoAST *patron = parsear_expresion(parser);

        if (!parser_esperar(parser, TOK_PARENTESIS_CERRAR))
            return NULL;

        return crear_nodo_dibujarlinea(x1, y1, x2, y2, patron, linea);
    }

    // DIBUJARCIRCULO(cx, cy, radio, patron)
    if (parser_coincidir(parser, TOK_DIBUJARCIRCULO))
    {
        int linea = parser->token_actual.linea;
        parser_avanzar(parser);
        if (!parser_esperar(parser, TOK_PARENTESIS_ABRIR))
            return NULL;

        NodoAST *centro_x = parsear_expresion(parser);
        if (!parser_esperar(parser, TOK_COMA))
            return NULL;

        NodoAST *centro_y = parsear_expresion(parser);
        if (!parser_esperar(parser, TOK_COMA))
            return NULL;

        NodoAST *radio = parsear_expresion(parser);
        if (!parser_esperar(parser, TOK_COMA))
            return NULL;

        // Patron (puede ser texto literal o variable)
        NodoAST *patron = parsear_expresion(parser);

        if (!parser_esperar(parser, TOK_PARENTESIS_CERRAR))
            return NULL;

        return crear_nodo_dibujarcirculo(centro_x, centro_y, radio, patron, linea);
    }

    // COLISIONRECTANGULOS(x1, y1, ancho1, alto1, x2, y2, ancho2, alto2, variable_resultado)
    if (parser_coincidir(parser, TOK_COLISIONRECTANGULOS))
    {
        int linea = parser->token_actual.linea;
        parser_avanzar(parser);
        if (!parser_esperar(parser, TOK_PARENTESIS_ABRIR))
            return NULL;

        NodoAST *x1 = parsear_expresion(parser);
        if (!parser_esperar(parser, TOK_COMA))
            return NULL;

        NodoAST *y1 = parsear_expresion(parser);
        if (!parser_esperar(parser, TOK_COMA))
            return NULL;

        NodoAST *ancho1 = parsear_expresion(parser);
        if (!parser_esperar(parser, TOK_COMA))
            return NULL;

        NodoAST *alto1 = parsear_expresion(parser);
        if (!parser_esperar(parser, TOK_COMA))
            return NULL;

        NodoAST *x2 = parsear_expresion(parser);
        if (!parser_esperar(parser, TOK_COMA))
            return NULL;

        NodoAST *y2 = parsear_expresion(parser);
        if (!parser_esperar(parser, TOK_COMA))
            return NULL;

        NodoAST *ancho2 = parsear_expresion(parser);
        if (!parser_esperar(parser, TOK_COMA))
            return NULL;

        NodoAST *alto2 = parsear_expresion(parser);
        if (!parser_esperar(parser, TOK_COMA))
            return NULL;

        // Variable de resultado - guardar el valor ANTES de avanzar
        if (!parser_coincidir(parser, TOK_VARIABLE))
        {
            parser_set_error(parser, "Se esperaba variable de resultado");
            return NULL;
        }
        char *var_resultado = strdup(parser->token_actual.valor + 1);
        parser_avanzar(parser);

        if (!parser_esperar(parser, TOK_PARENTESIS_CERRAR))
        {
            free(var_resultado);
            return NULL;
        }

        NodoAST *resultado = crear_nodo_colisionrectangulos(x1, y1, ancho1, alto1,
                                                            x2, y2, ancho2, alto2,
                                                            var_resultado, linea);
        free(var_resultado);
        return resultado;
    }

    // RELLENARRECTANGULO(x1, y1, x2, y2, patron)
    if (parser_coincidir(parser, TOK_RELLENARRECTANGULO))
    {
        int linea = parser->token_actual.linea;
        parser_avanzar(parser);
        if (!parser_esperar(parser, TOK_PARENTESIS_ABRIR))
            return NULL;

        NodoAST *x1 = parsear_expresion(parser);
        if (!parser_esperar(parser, TOK_COMA))
            return NULL;

        NodoAST *y1 = parsear_expresion(parser);
        if (!parser_esperar(parser, TOK_COMA))
            return NULL;

        NodoAST *x2 = parsear_expresion(parser);
        if (!parser_esperar(parser, TOK_COMA))
            return NULL;

        NodoAST *y2 = parsear_expresion(parser);
        if (!parser_esperar(parser, TOK_COMA))
            return NULL;

        // Patron (puede ser texto literal o variable)
        NodoAST *patron = parsear_expresion(parser);

        if (!parser_esperar(parser, TOK_PARENTESIS_CERRAR))
            return NULL;

        return crear_nodo_rellenarrectangulo(x1, y1, x2, y2, patron, linea);
    }

    // Comandos de cursor
    if (parser_coincidir(parser, TOK_CURSOR) || parser_coincidir(parser, TOK_POSICIONAR)) {
        int linea = parser->token_actual.linea;
        TipoToken tipo = parser->token_actual.tipo;
        parser_avanzar(parser);
        if (!parser_esperar(parser, TOK_PARENTESIS_ABRIR)) return NULL;
        NodoAST* fila = parsear_expresion(parser);
        if (!parser_esperar(parser, TOK_COMA)) return NULL;
        NodoAST* col = parsear_expresion(parser);
        if (!parser_esperar(parser, TOK_PARENTESIS_CERRAR)) return NULL;
        return (tipo == TOK_CURSOR) ? 
        crear_nodo_cursor(fila, col, linea) : 
        crear_nodo_posicionar(fila, col, linea);
    }

    // Comandos de terminal
    if (parser_coincidir(parser, TOK_ANCHOTERMINAL)) {
        int linea = parser->token_actual.linea;
        parser_avanzar(parser);
        if (!parser_esperar(parser, TOK_PARENTESIS_ABRIR)) return NULL;
        if (!parser_coincidir(parser, TOK_VARIABLE)) {
            parser_set_error(parser, "Se esperaba variable");
            return NULL;
        }
        char* var = strdup(parser->token_actual.valor + 1);
        parser_avanzar(parser);
        if (!parser_esperar(parser, TOK_PARENTESIS_CERRAR)) {
            free(var);
            return NULL;
        }
        return crear_nodo_anchoterminal(var, linea);
    }

    if (parser_coincidir(parser, TOK_ALTOTERMINAL)) {
        int linea = parser->token_actual.linea;
        parser_avanzar(parser);
        if (!parser_esperar(parser, TOK_PARENTESIS_ABRIR)) return NULL;
        if (!parser_coincidir(parser, TOK_VARIABLE)) {
            parser_set_error(parser, "Se esperaba variable");
            return NULL;
        }
        char* var = strdup(parser->token_actual.valor + 1);
        parser_avanzar(parser);
        if (!parser_esperar(parser, TOK_PARENTESIS_CERRAR)) {
            free(var);
            return NULL;
        }
        return crear_nodo_altoterminal(var, linea);
    }

    // Comandos de sistema
    if (parser_coincidir(parser, TOK_SISTEMA)) {
        int linea = parser->token_actual.linea;
        parser_avanzar(parser);
        if (!parser_esperar(parser, TOK_PARENTESIS_ABRIR)) return NULL;
        if (!parser_coincidir(parser, TOK_TEXTO)) {
            parser_set_error(parser, "Se esperaba comando entre comillas");
            return NULL;
        }
        char* cmd = strdup(parser->token_actual.valor);
        parser_avanzar(parser);
        if (!parser_esperar(parser, TOK_PARENTESIS_CERRAR)) {
            free(cmd);
            return NULL;
        }
        return crear_nodo_sistema(cmd, linea);
    }

    // ABRIRARCHIVO
    if (parser_coincidir(parser, TOK_ABRIRARCHIVO)) {
        int linea = parser->token_actual.linea;
        parser_avanzar(parser);
        if (!parser_esperar(parser, TOK_PARENTESIS_ABRIR)) return NULL;
    
        // Variable de archivo
        if (!parser_coincidir(parser, TOK_VARIABLE)) {
            parser_set_error(parser, "Se esperaba variable de archivo");
            return NULL;
        }
        char* var = strdup(parser->token_actual.valor + 1);
        parser_avanzar(parser);
    
        if (!parser_esperar(parser, TOK_COMA)) {
            free(var);
            return NULL;
        }
    
        // Ruta del archivo
        if (!parser_coincidir(parser, TOK_TEXTO)) {
            parser_set_error(parser, "Se esperaba ruta entre comillas");
            free(var);
            return NULL;
        }
        char* ruta = strdup(parser->token_actual.valor);
        parser_avanzar(parser);
    
        if (!parser_esperar(parser, TOK_COMA)) {
            free(var);
            free(ruta);
            return NULL;
        }
    
        // Modo
        int modo = parsear_modo_archivo(parser);
        if (modo < 0) {
            free(var);
            free(ruta);
            return NULL;
        }
    
        if (!parser_esperar(parser, TOK_PARENTESIS_CERRAR)) {
            free(var);
            free(ruta);
            return NULL;
        }
    
        return crear_nodo_abrirarchivo(var, ruta, modo, linea);
    }

    // USARARCHIVO
    if (parser_coincidir(parser, TOK_USARARCHIVO)) {
        int linea = parser->token_actual.linea;
        parser_avanzar(parser);
        if (!parser_esperar(parser, TOK_PARENTESIS_ABRIR)) return NULL;
    
        // Ruta del archivo
        if (!parser_coincidir(parser, TOK_TEXTO)) {
            parser_set_error(parser, "Se esperaba ruta entre comillas");
            return NULL;
        }
        char* ruta = strdup(parser->token_actual.valor);
        parser_avanzar(parser);
    
        if (!parser_esperar(parser, TOK_COMA)) {
            free(ruta);
            return NULL;
        }
    
        // Modo
        int modo = parsear_modo_archivo(parser);
        if (modo < 0) {
            free(ruta);
            return NULL;
        }
    
        if (!parser_esperar(parser, TOK_PARENTESIS_CERRAR)) {
            free(ruta);
            return NULL;
        }
    
        // USARARCHIVO usa la ruta como nombre de variable
        return crear_nodo_usararchivo(ruta, ruta, modo, linea);
    }

    // TIEMPOMS
    if (parser_coincidir(parser, TOK_TIEMPOMS)) {
        int linea = parser->token_actual.linea;
        parser_avanzar(parser);
        if (!parser_esperar(parser, TOK_PARENTESIS_ABRIR)) return NULL;
        if (!parser_coincidir(parser, TOK_VARIABLE)) {
            parser_set_error(parser, "Se esperaba variable");
            return NULL;
        }
        char* var = strdup(parser->token_actual.valor + 1);
        parser_avanzar(parser);
        if (!parser_esperar(parser, TOK_PARENTESIS_CERRAR)) {
            free(var);
            return NULL;
        }
        return crear_nodo_tiempoms(var, linea);
    }

    // Comandos de base de datos
    if (parser_coincidir(parser, TOK_CONECTARBD) ||
        parser_coincidir(parser, TOK_EJECUTARBD))
    {
        int linea = parser->token_actual.linea;
        char *nombre = strdup(parser->token_actual.valor);
        parser_avanzar(parser);

        if (!parser_esperar(parser, TOK_PARENTESIS_ABRIR))
        {
            free(nombre);
            return NULL;
        }

        NodoAST *argumentos = crear_nodo_bloque(linea);

        if (!parser_coincidir(parser, TOK_PARENTESIS_CERRAR))
        {
            NodoAST *arg = parsear_expresion(parser);
            agregar_sentencia_a_bloque(argumentos, arg);

            while (parser_coincidir(parser, TOK_COMA))
            {
                parser_avanzar(parser);
                arg = parsear_expresion(parser);
                agregar_sentencia_a_bloque(argumentos, arg);
            }

            if (!parser_esperar(parser, TOK_PARENTESIS_CERRAR))
            {
                free(nombre);
                return NULL;
            }
        }

        return crear_nodo_llamada_funcion(nombre, argumentos, linea);
    }

    if (parser_coincidir(parser, TOK_CONSULTARBD))
    {
        int linea = parser->token_actual.linea;
        parser_avanzar(parser);

        if (!parser_esperar(parser, TOK_PARENTESIS_ABRIR))
            return NULL;

        NodoAST *argumentos = crear_nodo_bloque(linea);

        if (!parser_coincidir(parser, TOK_PARENTESIS_CERRAR))
        {
            NodoAST *arg = parsear_expresion(parser);
            agregar_sentencia_a_bloque(argumentos, arg);

            while (parser_coincidir(parser, TOK_COMA))
            {
                parser_avanzar(parser);
                arg = parsear_expresion(parser);
                agregar_sentencia_a_bloque(argumentos, arg);
            }

            if (!parser_esperar(parser, TOK_PARENTESIS_CERRAR))
                return NULL;
        }

        return crear_nodo_llamada_funcion("CONSULTARBD", argumentos, linea);
    }

    if (parser_coincidir(parser, TOK_CERRARBD) ||
        parser_coincidir(parser, TOK_CERRARCONSULTABD))
    {
        int linea = parser->token_actual.linea;
        char *nombre = strdup(parser->token_actual.valor);
        parser_avanzar(parser);
        return crear_nodo_llamada_funcion(nombre, NULL, linea);
    }

    if (parser_coincidir(parser, TOK_INICIARTRANSACCION) ||
        parser_coincidir(parser, TOK_CONFIRMARTRANSACCION) ||
        parser_coincidir(parser, TOK_DESHACERTRANSACCION))
    {
        int linea = parser->token_actual.linea;
        char *nombre = strdup(parser->token_actual.valor);
        parser_avanzar(parser);
        return crear_nodo_llamada_funcion(nombre, NULL, linea);
    }

    // Servidor web
    if (parser_coincidir(parser, TOK_INICIARSERVER))
    {
        int linea = parser->token_actual.linea;
        parser_avanzar(parser);

        if (!parser_esperar(parser, TOK_PARENTESIS_ABRIR))
            return NULL;

        NodoAST *argumentos = crear_nodo_bloque(linea);

        if (!parser_coincidir(parser, TOK_PARENTESIS_CERRAR))
        {
            NodoAST *arg = parsear_expresion(parser);
            agregar_sentencia_a_bloque(argumentos, arg);

            if (!parser_esperar(parser, TOK_PARENTESIS_CERRAR))
                return NULL;
        }

        return crear_nodo_llamada_funcion("INICIARSERVER", argumentos, linea);
    }

    if (parser_coincidir(parser, TOK_DETENERSERVER))
    {
        int linea = parser->token_actual.linea;
        parser_avanzar(parser);

        // Puede tener paréntesis opcionales
        if (parser_coincidir(parser, TOK_PARENTESIS_ABRIR))
        {
            parser_avanzar(parser);
            if (!parser_esperar(parser, TOK_PARENTESIS_CERRAR))
                return NULL;
        }

        return crear_nodo_llamada_funcion("DETENERSERVER", NULL, linea);
    }

    // ETIQUETA nombre
    if (parser_coincidir(parser, TOK_ETIQUETA))
    {
        int linea = parser->token_actual.linea;
        parser_avanzar(parser); // Consumir ETIQUETA

        if (!parser_coincidir(parser, TOK_IDENTIFICADOR))
        {
            parser_set_error(parser, "Se esperaba nombre de etiqueta");
            return NULL;
        }

        char *nombre = strdup(parser->token_actual.valor);
        parser_avanzar(parser); // Consumir nombre

        return crear_nodo_etiqueta(nombre, linea);
    }

    // SALTAR A nombre
    if (parser_coincidir(parser, TOK_SALTAR))
    {
        int linea = parser->token_actual.linea;
        parser_avanzar(parser); // Consumir SALTAR

        if (!parser_coincidir(parser, TOK_A))
        {
            parser_set_error(parser, "Se esperaba 'A' después de SALTAR");
            return NULL;
        }
        parser_avanzar(parser); // Consumir A

        if (!parser_coincidir(parser, TOK_IDENTIFICADOR))
        {
            parser_set_error(parser, "Se esperaba nombre de etiqueta");
            return NULL;
        }

        char *nombre = strdup(parser->token_actual.valor);
        parser_avanzar(parser); // Consumir nombre

        return crear_nodo_saltar_a(nombre, linea);
    }

    // TECLAMANTENIDA(codigo, variable)
    if (parser_coincidir(parser, TOK_TECLAMANTENIDA))
    {
        int linea = parser->token_actual.linea;
        parser_avanzar(parser); // Consumir TECLAMANTENIDA

        if (!parser_esperar(parser, TOK_PARENTESIS_ABRIR))
            return NULL;

        NodoAST *codigo = parsear_expresion(parser);

        if (!parser_esperar(parser, TOK_COMA))
            return NULL;

        if (!parser_coincidir(parser, TOK_VARIABLE))
        {
            parser_set_error(parser, "Se esperaba variable de destino");
            return NULL;
        }

        char *variable = strdup(parser->token_actual.valor + 1); // Sin el $
        parser_avanzar(parser);

        if (!parser_esperar(parser, TOK_PARENTESIS_CERRAR))
        {
            free(variable);
            return NULL;
        }

        return crear_nodo_teclamantenida(codigo, variable, linea);
    }

    // LEERTECLA(variable)
    if (parser_coincidir(parser, TOK_LEERTECLA))
    {
        int linea = parser->token_actual.linea;
        parser_avanzar(parser); // Consumir LEERTECLA

        if (!parser_esperar(parser, TOK_PARENTESIS_ABRIR))
            return NULL;

        if (!parser_coincidir(parser, TOK_VARIABLE))
        {
            parser_set_error(parser, "Se esperaba variable de destino");
            return NULL;
        }

        char *variable = strdup(parser->token_actual.valor + 1); // Sin el $
        parser_avanzar(parser);

        if (!parser_esperar(parser, TOK_PARENTESIS_CERRAR))
        {
            free(variable);
            return NULL;
        }

        return crear_nodo_leertecla(variable, linea);
    }
    // CONFIGURARPIN
    if (parser_coincidir(parser, TOK_CONFIGURARPIN))
    {
        int linea = parser->token_actual.linea;
        parser_avanzar(parser);

        if (!parser_esperar(parser, TOK_PARENTESIS_ABRIR))
            return NULL;

        NodoAST *pin = parsear_expresion(parser);

        if (!parser_esperar(parser, TOK_COMA))
        {
            return NULL;
        }

        int direccion = -1;
        int bias = 0;

        if (parser_coincidir(parser, TOK_SALIDA))
        {
            direccion = 1;
            parser_avanzar(parser);
        }
        else if (parser_coincidir(parser, TOK_ENTRADA))
        {
            direccion = 0;
            parser_avanzar(parser);

            if (parser_coincidir(parser, TOK_COMA))
            {
                parser_avanzar(parser);
                if (parser_coincidir(parser, TOK_PULLUP))
                {
                    bias = 1;
                    parser_avanzar(parser);
                }
                else if (parser_coincidir(parser, TOK_PULLDOWN))
                {
                    bias = 2;
                    parser_avanzar(parser);
                }
            }
        }
        else
        {
            parser_set_error(parser, "Se esperaba SALIDA o ENTRADA");
            return NULL;
        }

        if (!parser_esperar(parser, TOK_PARENTESIS_CERRAR))
            return NULL;

        return crear_nodo_configurarpin(pin, direccion, bias, linea);
    }

    // ESTADOPIN
    if (parser_coincidir(parser, TOK_ESTADOPIN))
    {
        int linea = parser->token_actual.linea;
        parser_avanzar(parser);

        if (!parser_esperar(parser, TOK_PARENTESIS_ABRIR))
            return NULL;

        NodoAST *pin = parsear_expresion(parser);

        if (!parser_esperar(parser, TOK_COMA))
            return NULL;

        int valor = -1;
        if (parser_coincidir(parser, TOK_SI))
        {
            valor = 1;
            parser_avanzar(parser);
        }
        else if (parser_coincidir(parser, TOK_NO))
        {
            valor = 0;
            parser_avanzar(parser);
        }
        else
        {
            parser_set_error(parser, "Se esperaba SI o NO");
            return NULL;
        }

        if (!parser_esperar(parser, TOK_PARENTESIS_CERRAR))
            return NULL;

        return crear_nodo_estadopin(pin, valor, linea);
    }

    // LEERPIN
    if (parser_coincidir(parser, TOK_LEERPIN))
    {
        int linea = parser->token_actual.linea;
        parser_avanzar(parser);

        if (!parser_esperar(parser, TOK_PARENTESIS_ABRIR))
            return NULL;

        NodoAST *pin = parsear_expresion(parser);

        char *variable_destino = NULL;

        if (parser_coincidir(parser, TOK_COMA))
        {
            parser_avanzar(parser);

            // Esperar "EN"
            if (parser->token_actual.tipo != TOK_EN)
            {
                parser_set_error(parser, "Se esperaba EN");
                return NULL;
            }
            parser_avanzar(parser);

            // Esperar variable
            if (parser->token_actual.tipo != TOK_VARIABLE)
            {
                parser_set_error(parser, "Se esperaba variable");
                return NULL;
            }
            variable_destino = strdup(parser->token_actual.valor + 1); // Sin el $
            parser_avanzar(parser);
        }

        if (!parser_esperar(parser, TOK_PARENTESIS_CERRAR))
        {
            if (variable_destino)
                free(variable_destino);
            return NULL;
        }

        return crear_nodo_leerpin(pin, variable_destino, linea);
    }

    // GENERARPWM
    if (parser_coincidir(parser, TOK_GENERARPWM)) {
        int linea = parser->token_actual.linea;
        parser_avanzar(parser);
        if (!parser_esperar(parser, TOK_PARENTESIS_ABRIR)) return NULL;
        NodoAST *pin = parsear_expresion(parser);
        if (!parser_esperar(parser, TOK_COMA)) return NULL;
        NodoAST *frecuencia = parsear_expresion(parser);
        if (!parser_esperar(parser, TOK_COMA)) return NULL;
        NodoAST *duty_cycle = parsear_expresion(parser);
        if (!parser_esperar(parser, TOK_PARENTESIS_CERRAR)) return NULL;
        return crear_nodo_pwm(pin, frecuencia, duty_cycle, linea);
    }
    
    // DETENERPWM
    if (parser_coincidir(parser, TOK_DETENERPWM)) {
        int linea = parser->token_actual.linea;
        parser_avanzar(parser);
        
        if (!parser_esperar(parser, TOK_PARENTESIS_ABRIR)) return NULL;
        
        NodoAST *pin = parsear_expresion(parser);
        
        if (!parser_esperar(parser, TOK_PARENTESIS_CERRAR)) return NULL;
        
        return crear_nodo_detenerpwm(pin, linea);
    }

    return NULL;
}

static NodoAST *parsear_bloque(Parser *parser)
{
    int linea = parser->token_actual.linea;
    NodoAST *bloque = crear_nodo_bloque(linea);

    while (!parser->hay_error &&
           parser->token_actual.tipo != TOK_EOF &&
           parser->token_actual.tipo != TOK_FIN &&
           parser->token_actual.tipo != TOK_SINO &&
           parser->token_actual.tipo != TOK_SINOSI &&
           parser->token_actual.tipo != TOK_CASO &&
           parser->token_actual.tipo != TOK_POR &&
           parser->token_actual.tipo != TOK_DEFECTO)
    {

        NodoAST *sentencia = parsear_sentencia(parser);
        if (sentencia)
        {
            agregar_sentencia_a_bloque(bloque, sentencia);
        }
        else
        {
            break;
        }
    }

    return bloque;
}

static NodoAST *parsear_bloque_hasta(Parser *parser, TipoToken token_parada)
{
    int linea = parser->token_actual.linea;
    NodoAST *bloque = crear_nodo_bloque(linea);

    while (!parser->hay_error &&
           !parser_coincidir(parser, TOK_EOF) &&
           !parser_coincidir(parser, TOK_FIN) &&
           !parser_coincidir(parser, TOK_SINO) &&
           !parser_coincidir(parser, TOK_CASO) &&
           !parser_coincidir(parser, TOK_POR) &&
           !parser_coincidir(parser, TOK_DEFECTO) &&
           !parser_coincidir(parser, token_parada))
    { 

        NodoAST *sentencia = parsear_sentencia(parser);
        if (sentencia)
        {
            agregar_sentencia_a_bloque(bloque, sentencia);
        }
        else
        {
            break;
        }
    }

    return bloque;
}

// ============================================================
// PARSEO DEL PROGRAMA PRINCIPAL
// ============================================================
static NodoAST *parsear_programa(Parser *parser)
{
    int linea = parser->token_actual.linea;

    // PROGRAMA nombre
    if (!parser_esperar(parser, TOK_PROGRAMA))
        return NULL;

    char *nombre = NULL;

    // Si el token actual es TEXTO, usar su valor directamente (nombre entre comillas)
    if (parser->token_actual.tipo == TOK_TEXTO)
    {
        nombre = strdup(parser->token_actual.valor);
        parser_avanzar(parser);
    }
    else if (parser->token_actual.tipo == TOK_IDENTIFICADOR)
    {
        // Si es un identificador válido, usar su valor
        nombre = strdup(parser->token_actual.valor);
        parser_avanzar(parser);
    }
    else
    {
        // Para casos especiales (palabras reservadas, caracteres especiales),
        // leer directamente del código fuente
        char buffer_nombre[1024] = {0};
        size_t pos = 0;

        const char *codigo = parser->lexer->codigo;
        size_t posicion = parser->lexer->posicion;

        // Saltar espacios iniciales
        while (codigo[posicion] == ' ' || codigo[posicion] == '\t')
        {
            posicion++;
        }

        // Leer hasta el fin de línea
        while (codigo[posicion] != '\0' &&
               codigo[posicion] != '\n' &&
               codigo[posicion] != '\r' &&
               pos < sizeof(buffer_nombre) - 1)
        {
            buffer_nombre[pos++] = codigo[posicion++];
        }

        buffer_nombre[pos] = '\0';

        // Avanzar al inicio de la siguiente línea
        while (codigo[posicion] != '\0' &&
               codigo[posicion] != '\n' &&
               codigo[posicion] != '\r')
        {
            posicion++;
        }

        if (codigo[posicion] == '\r')
            posicion++;
        if (codigo[posicion] == '\n')
            posicion++;

        // Actualizar posición del lexer
        parser->lexer->posicion = posicion;
        parser->lexer->linea++;
        parser->lexer->columna = 1;
        parser->lexer->hay_error = false;
        parser->lexer->mensaje_error[0] = '\0';

        // Eliminar espacios al inicio y final
        char *nombre_inicio = buffer_nombre;
        while (*nombre_inicio == ' ')
            nombre_inicio++;

        if (strlen(nombre_inicio) > 0)
        {
            char *nombre_fin = nombre_inicio + strlen(nombre_inicio) - 1;
            while (nombre_fin > nombre_inicio && *nombre_fin == ' ')
            {
                *nombre_fin = '\0';
                nombre_fin--;
            }
        }

        if (strlen(nombre_inicio) == 0)
        {
            parser_set_error(parser, "Se esperaba nombre del programa");
            return NULL;
        }

        nombre = strdup(nombre_inicio);

        // Invalidar tokens pre-cargados y generar nuevos
        token_destruir(&parser->token_actual);
        token_destruir(&parser->token_siguiente);
        parser->token_actual = lexer_siguiente_token(parser->lexer);
        parser->token_siguiente = lexer_siguiente_token(parser->lexer);
    }


    NodoAST *programa = crear_nodo_programa(nombre, linea);

    // Declaraciones globales
    NodoAST *declaraciones = crear_nodo_bloque(linea);
    NodoAST *funciones = crear_nodo_bloque(linea);
    NodoAST *subprogramas = crear_nodo_bloque(linea);

    while (parser->token_actual.tipo != TOK_BLOQUE && !parser->hay_error)
    {
        if (parser->token_actual.tipo == TOK_FUNCION)
        {
            NodoAST *funcion = parsear_funcion(parser);
            agregar_sentencia_a_bloque(funciones, funcion);
        }
        else if (parser->token_actual.tipo == TOK_SUBPROGRAMA)
        {
            NodoAST *subprograma = parsear_subprograma(parser);
            agregar_sentencia_a_bloque(subprogramas, subprograma);
        }

        else if (parser->token_actual.tipo == TOK_VARIABLE_KW ||
                 parser->token_actual.tipo == TOK_CONSTANTE ||
                 parser->token_actual.tipo == TOK_LISTA ||
                 parser->token_actual.tipo == TOK_MATRIZ ||
                 parser->token_actual.tipo == TOK_DECLARAR)
        {
            NodoAST *declaracion = parsear_sentencia(parser);
            agregar_sentencia_a_bloque(declaraciones, declaracion);
        }
        else if (parser->token_actual.tipo == TOK_INCLUIR)
        {
            NodoAST *incluir = parsear_incluir(parser);
            agregar_sentencia_a_bloque(declaraciones, incluir);
        }
        else
        {
            parser_set_error(parser, "Se esperaba declaración, función o BLOQUE PRINCIPAL");
            return NULL;
        }
    }

    programa->datos.programa.declaraciones = declaraciones;
    programa->datos.programa.funciones = funciones;
    programa->datos.programa.subprogramas = subprogramas;

    // BLOQUE PRINCIPAL
    if (!parser_esperar(parser, TOK_BLOQUE))
        return NULL;
    if (!parser_esperar(parser, TOK_PRINCIPAL))
        return NULL;

    NodoAST *bloque_principal = parsear_bloque(parser);
    programa->datos.programa.bloque_principal = bloque_principal;

    // FIN PRINCIPAL
    if (!parser_esperar(parser, TOK_FIN))
        return NULL;
    if (!parser_esperar(parser, TOK_PRINCIPAL))
        return NULL;

    // FINAL
    if (!parser_esperar(parser, TOK_FINAL))
        return NULL;

    return programa;
}

// ============================================================
// FUNCIONES PÚBLICAS
// ============================================================

Parser* parser_crear(Lexer* lexer) {
    if (!lexer) return NULL;
    
    Parser* parser = malloc(sizeof(Parser));
    if (!parser) return NULL;
    
    parser->lexer = lexer;
    parser->hay_error = false;
    parser->mensaje_error[0] = '\0';
    
    // Inicializar tokens
    parser->token_actual = lexer_siguiente_token(lexer);
    parser->token_siguiente = lexer_siguiente_token(lexer);
    
    return parser;
}

void parser_destruir(Parser* parser) {
    if (parser) {
        token_destruir(&parser->token_actual);
        token_destruir(&parser->token_siguiente);
        free(parser);
    }
}

NodoAST* parser_parsear(Parser* parser) {
    if (!parser) return NULL;
    
    NodoAST* ast = parsear_programa(parser);
    
    if (parser->hay_error) {
        if (ast) liberar_ast(ast);
        return NULL;
    }
    
    return ast;
}

bool parser_tiene_error(const Parser* parser) {
    return parser && parser->hay_error;
}

const char* parser_obtener_error(const Parser* parser) {
    if (!parser) return "Parser nulo";
    return parser->mensaje_error;
}

// ============================================================
// PARSER DE FUNCIONES Y SUBPROGRAMAS
// ============================================================
static NodoAST *parsear_funcion(Parser *parser)
{
    int linea = parser->token_actual.linea;
    parser_avanzar(parser); // Consumir FUNCION

    TipoDato tipo_retorno = TIPO_VACIO;
    if (parser_coincidir(parser, TOK_ENTERA))
    {
        tipo_retorno = TIPO_ENTERO;
        parser_avanzar(parser);
    }
    else if (parser_coincidir(parser, TOK_DECIMAL))
    {
        tipo_retorno = TIPO_DECIMAL;
        parser_avanzar(parser);
    }
    else if (parser_coincidir(parser, TOK_TEXTO))
    {
        tipo_retorno = TIPO_TEXTO;
        parser_avanzar(parser);
    }
    else if (parser_coincidir(parser, TOK_CARACTER))
    {
        tipo_retorno = TIPO_CARACTER;
        parser_avanzar(parser);
    }
    else if (parser_coincidir(parser, TOK_LOGICA))
    {
        tipo_retorno = TIPO_LOGICA;
        parser_avanzar(parser);
    }

    if (!parser_coincidir(parser, TOK_IDENTIFICADOR))
    {
        parser_set_error(parser, "Se esperaba nombre de función");
        return NULL;
    }
    char *nombre = strdup(parser->token_actual.valor);
    parser_avanzar(parser);

    if (!parser_esperar(parser, TOK_PARENTESIS_ABRIR))
    {
        free(nombre);
        return NULL;
    }

    char **parametros = NULL;
    int num_parametros = 0;

    if (!parser_coincidir(parser, TOK_PARENTESIS_CERRAR))
    {
        char params_buffer[256][64];
        int count = 0;
        while (!parser->hay_error && parser->token_actual.tipo != TOK_PARENTESIS_CERRAR)
        {
            if (!parser_coincidir(parser, TOK_VARIABLE))
            {
                parser_set_error(parser, "Se esperaba nombre de parámetro");
                free(nombre);
                return NULL;
            }
            strncpy(params_buffer[count], parser->token_actual.valor + 1, 63);
            params_buffer[count][63] = '\0';
            count++;
            parser_avanzar(parser);
            if (parser_coincidir(parser, TOK_COMA))
                parser_avanzar(parser);
        }
        if (!parser_esperar(parser, TOK_PARENTESIS_CERRAR))
        {
            free(nombre);
            return NULL;
        }
        num_parametros = count;
        parametros = malloc(sizeof(char *) * num_parametros);
        for (int i = 0; i < num_parametros; i++)
            parametros[i] = strdup(params_buffer[i]);
    }
    else
    {
        parser_avanzar(parser);
    }

    // ✅ Parsear cuerpo de la función con loop propio
    // Se detiene SOLO en FIN FUNCION, no en FIN SI, FIN MIENTRAS, etc.
    NodoAST *bloque = crear_nodo_bloque(linea);
    while (!parser->hay_error && parser->token_actual.tipo != TOK_EOF)
    {
        // Verificar si es FIN FUNCION (no cualquier FIN)
        if (parser->token_actual.tipo == TOK_FIN &&
            parser_verificar_siguiente(parser, TOK_FUNCION))
        {
            break;
        }
        NodoAST *s = parsear_sentencia(parser);
        if (s)
            agregar_sentencia_a_bloque(bloque, s);
        else
            break;
    }

    if (!parser_esperar(parser, TOK_FIN))
    {
        free(nombre);
        if (parametros)
        {
            for (int i = 0; i < num_parametros; i++)
                free(parametros[i]);
            free(parametros);
        }
        return NULL;
    }

    if (!parser_esperar(parser, TOK_FUNCION))
    {
        free(nombre);
        if (parametros)
        {
            for (int i = 0; i < num_parametros; i++)
                free(parametros[i]);
            free(parametros);
        }
        return NULL;
    }

    return crear_nodo_funcion(nombre, tipo_retorno, parametros, num_parametros, bloque, linea);
}

static NodoAST *parsear_subprograma(Parser *parser)
{
    int linea = parser->token_actual.linea;
    parser_avanzar(parser); // Consumir SUBPROGRAMA

    // Aceptar cualquier token que pueda ser un nombre (identificador o palabra clave)
    char *nombre = NULL;
    if (parser->token_actual.tipo == TOK_IDENTIFICADOR ||
        parser->token_actual.tipo >= TOK_PROGRAMA) // Cualquier palabra clave
    {
        nombre = strdup(parser->token_actual.valor);
        parser_avanzar(parser);
    }
    else
    {
        parser_set_error(parser, "Se esperaba nombre de subprograma");
        return NULL;
    }


    char **parametros = NULL;
    int num_parametros = 0;
    if (parser_coincidir(parser, TOK_PARENTESIS_ABRIR))
    {
        parser_avanzar(parser);
        if (!parser_coincidir(parser, TOK_PARENTESIS_CERRAR))
        {
            char params_buffer[256][64];
            int count = 0;
            while (!parser->hay_error && !parser_coincidir(parser, TOK_PARENTESIS_CERRAR))
            {
                if (!parser_coincidir(parser, TOK_VARIABLE))
                {
                    parser_set_error(parser, "Se esperaba nombre de parámetro");
                    free(nombre);
                    return NULL;
                }
                strncpy(params_buffer[count], parser->token_actual.valor + 1, 63);
                params_buffer[count][63] = '\0';
                count++;
                parser_avanzar(parser);
                if (parser_coincidir(parser, TOK_COMA))
                    parser_avanzar(parser);
            }
            if (!parser_esperar(parser, TOK_PARENTESIS_CERRAR))
            {
                free(nombre);
                return NULL;
            }
            num_parametros = count;
            parametros = malloc(sizeof(char *) * num_parametros);
            for (int i = 0; i < num_parametros; i++)
                parametros[i] = strdup(params_buffer[i]);
        }
        else
        {
            parser_avanzar(parser);
        }
    }

    NodoAST *bloque = crear_nodo_bloque(linea);
    while (!parser->hay_error && !parser_coincidir(parser, TOK_FIN) && !parser_coincidir(parser, TOK_EOF))
    {
        NodoAST *s = parsear_sentencia(parser);
        if (s)
            agregar_sentencia_a_bloque(bloque, s);
        else
            break;
    }

    if (!parser_esperar(parser, TOK_FIN))
    {
        free(nombre);
        if (parametros)
        {
            for (int i = 0; i < num_parametros; i++)
                free(parametros[i]);
            free(parametros);
        }
        return NULL;
    }
    if (!parser_esperar(parser, TOK_SUBPROGRAMA))
    {
        free(nombre);
        if (parametros)
        {
            for (int i = 0; i < num_parametros; i++)
                free(parametros[i]);
            free(parametros);
        }
        return NULL;
    }
    return crear_nodo_subprograma(nombre, parametros, num_parametros, bloque, linea);
}
