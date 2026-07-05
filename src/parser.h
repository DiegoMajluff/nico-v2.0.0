/*
 * Nico v2.0.0 - Intérprete Educativo de Scripting en Español
 * @file:         parser.h
 * @author:       Diego Alejandro Majluff (Diseño, Arquitectura y Supervisión)
 * @ai_assist:    Qwen (Alibaba Cloud) - Implementación, Debugging y Optimización
 * @license:      MIT / Personal Use (ver LICENSE)
 * @description:  Definiciones del analizador sintáctico. Contiene la estructura
 *                del parser y declaración de funciones para convertir tokens
 *                en árbol de sintaxis abstracta (AST).
 */
#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"

// ============================================================
// ESTRUCTURA DEL PARSER
// ============================================================
typedef struct {
    Lexer* lexer;           // Lexer asociado
    Token token_actual;     // Token actual
    Token token_siguiente;  // Token siguiente (lookahead)
    bool hay_error;         // Flag de error
    char mensaje_error[512];// Mensaje de error
} Parser;

// ============================================================
// FUNCIONES PÚBLICAS
// ============================================================

// Crear el parser
Parser* parser_crear(Lexer* lexer);

// Liberar el parser
void parser_destruir(Parser* parser);

// Parsear el programa completo y devolver el AST
NodoAST* parser_parsear(Parser* parser);

// Función pública para parsear una expresión
NodoAST *parsear_comparacion(Parser *parser);
NodoAST *parsear_expresion(Parser *parser);

// Ver si hay error
bool parser_tiene_error(const Parser* parser);

// Obtener el mensaje de error
const char* parser_obtener_error(const Parser* parser);

#endif // PARSER_H
