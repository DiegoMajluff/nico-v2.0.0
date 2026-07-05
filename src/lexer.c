/*
 * Nico v2.0.0 - Intérprete Educativo de Scripting en Español
 * @file:         lexer.c
 * @author:       Diego Alejandro Majluff (Diseño, Arquitectura y Supervisión)
 * @ai_assist:    Qwen (Alibaba Cloud) - Implementación, Debugging y Optimización
 * @license:      MIT / Personal Use (ver LICENSE)
 * @description:  Implementación del analizador léxico (tokenizer). Convierte
 *                el código fuente en secuencia de tokens, maneja palabras clave
 *                en español, identificadores, literales numéricos/texto,
 *                operadores y comentarios. Incluye manejo de errores léxicos.
 */
#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// ============================================================
// TABLA DE PALABRAS CLAVE
// ============================================================
typedef struct {
    const char* palabra;
    TipoToken tipo;
} PalabraClave;

// NOTA: Las palabras clave compuestas (BLOQUE PRINCIPAL, etc.)
// NO están aquí. El parser las maneja combinando tokens simples.
static const PalabraClave palabras_clave[] = {
    // Estructura
    {"PROGRAMA", TOK_PROGRAMA},
    {"FINAL", TOK_FINAL},
    {"BLOQUE", TOK_BLOQUE},
    {"PRINCIPAL", TOK_PRINCIPAL},
    {"FIN", TOK_FIN},
    {"SI", TOK_SI},
    {"SINOSI", TOK_SINOSI},
    {"SINO", TOK_SINO},
    {"ENTONCES", TOK_ENTONCES},
    {"PARA", TOK_PARA},
    {"HACER", TOK_HACER},
    {"DESDE", TOK_DESDE},
    {"HASTA", TOK_HASTA},
    {"PASO", TOK_PASO},
    {"MIENTRAS", TOK_MIENTRAS},
    {"REALIZAR", TOK_REALIZAR},
    {"SEGUN", TOK_SEGUN},
    {"CASO", TOK_CASO},
    {"DEFECTO", TOK_DEFECTO},
    {"FUNCION", TOK_FUNCION},
    {"SUBPROGRAMA", TOK_SUBPROGRAMA},
    {"LLAMAR", TOK_LLAMAR},
    {"RETORNAR", TOK_RETORNAR},
    {"SALTAR", TOK_SALTAR},
    {"ETIQUETA", TOK_ETIQUETA},
    {"CORTE", TOK_CORTE},
    {"INCLUIR", TOK_INCLUIR},
    {"A", TOK_A},
    {"SIN", TOK_SIN},
    {"POR", TOK_POR},

    // Tipos
    {"DECLARAR", TOK_DECLARAR},
    {"VARIABLE", TOK_VARIABLE_KW},
    {"CONSTANTE", TOK_CONSTANTE},
    {"LISTA", TOK_LISTA},
    {"MATRIZ", TOK_MATRIZ},
    {"ENTERA", TOK_ENTERA},
    {"ENTERO", TOK_ENTERO_KW},
    {"DECIMAL", TOK_DECIMAL},
    {"TEXTO", TOK_TEXTO_KW},
    {"CARACTER", TOK_CARACTER_KW},
    {"ARCHIVO", TOK_ARCHIVO},
    {"LOGICA", TOK_LOGICA},
    {"EXTENSO", TOK_EXTENSO},
    {"SIGNO", TOK_SIGNO},

    // Operadores lógicos
    {"MAYOR", TOK_MAYOR},
    {"MENOR", TOK_MENOR},
    {"IGUAL", TOK_IGUAL_KW},
    {"DIFERENTE", TOK_DIFERENTE},
    {"MAYORIGUAL", TOK_MAYOR_IGUAL},
    {"MENORIGUAL", TOK_MENOR_IGUAL},
    {"Y", TOK_Y},
    {"O", TOK_O},
    {"NO", TOK_NO},
    {"MOD", TOK_MOD},
    {"VERDADERO", TOK_VERDADERO},
    {"FALSO", TOK_FALSO},

    // Bitwise
    {"BITY", TOK_BITY},
    {"BITO", TOK_BITO},
    {"BITXOR", TOK_BITXOR},
    {"BITNO", TOK_BITNO},
    {"DESPLAZARIZQUIERDA", TOK_DESPLAZARIZQUIERDA},
    {"DESPLAZARDERECHA", TOK_DESPLAZARDERECHA},
    {"ROTARIZQUIERDA", TOK_ROTARIZQUIERDA},
    {"ROTARDERECHA", TOK_ROTARDERECHA},
    {"LEERBIT", TOK_LEERBIT},
    {"ACTIVARBIT", TOK_ACTIVARBIT},
    {"DESACTIVARBIT", TOK_DESACTIVARBIT},
    {"INVERTIRBYTES", TOK_INVERTIRBYTES},
    {"CONTARBITS", TOK_CONTARBITS},

    // Funciones matemáticas
    {"SENO", TOK_SENO},
    {"COSENO", TOK_COSENO},
    {"TANGENTE", TOK_TANGENTE},
    {"RAIZ", TOK_RAIZ},
    {"POTENCIA", TOK_POTENCIA_FN},
    {"MODULO", TOK_MODULO_FN},
    {"MAXIMO", TOK_MAXIMO},
    {"MINIMO", TOK_MINIMO},
    {"ABSOLUTO", TOK_ABSOLUTO},
    {"ARCOSENO", TOK_ARCOSENO},
    {"ARCOCOSENO", TOK_ARCOCOSENO},
    {"ARCOTANGENTE", TOK_ARCOTANGENTE},
    {"LOGNATURAL", TOK_LOGNATURAL},
    {"LOGBASE10", TOK_LOGBASE10},
    {"LOGBASE2", TOK_LOGBASE2},
    {"LOGARITMO", TOK_LOGARITMO},
    {"EXPONENCIAL", TOK_EXPONENCIAL},
    {"DOSALAX", TOK_DOSALAX},
    {"REDONDEAR", TOK_REDONDEAR},
    {"QUITARDECIMAL", TOK_QUITARDECIMAL},
    {"SIGMOIDE", TOK_SIGMOIDE},
    {"RAIZCUBICA", TOK_RAIZCUBICA},
    {"NUMEROPI", TOK_NUMEROPI},
    {"NUMEROEULER", TOK_NUMEROEULER},
    {"RAIZDEUNMEDIO", TOK_RAIZDEUNMEDIO},
    {"LOGNATURALDE2", TOK_LOGNATURALDE2},
    {"LOGNATURALDE10", TOK_LOGNATURALDE10},
    {"PI", TOK_PI},
    {"ALEATORIO", TOK_ALEATORIO},
    {"ALEATORIOSINSIGNO", TOK_ALEATORIOSINSIGNO},
    {"LONGITUDLISTA", TOK_LONGITUDLISTA},
    {"FINARCHIVO", TOK_FINARCHIVO},

    // Funciones de texto
    {"LONGITUDTEXTO", TOK_LONGITUDTEXTO},
    {"BUSCARTEXTO", TOK_BUSCARTEXTO},
    {"BUSCARCARACTER", TOK_BUSCARCARACTER},
    {"COMPARARTEXTO", TOK_COMPARARTEXTO},
    {"TEXTOVACIO", TOK_TEXTOVACIO},
    {"TEXTOAENTERO", TOK_TEXTOAENTERO},
    {"TEXTOADECIMAL", TOK_TEXTOADECIMAL},
    {"TEXTOACARACTER", TOK_TEXTOACARACTER},
    {"COPIARTEXTO", TOK_COPIARTEXTO},
    {"CONCATENARTEXTO", TOK_CONCATENARTEXTO},
    {"MAYUSCULAS", TOK_MAYUSCULAS},
    {"MINUSCULAS", TOK_MINUSCULAS},
    {"RECORTARTEXTO", TOK_RECORTARTEXTO},
    {"REEMPLAZARTEXTO", TOK_REEMPLAZARTEXTO},
    {"REPETIRTEXTO", TOK_REPETIRTEXTO},
    {"EXTRAERTEXTO", TOK_EXTRAERTEXTO},
    {"DIVIDIRTEXTO", TOK_DIVIDIRTEXTO},
    {"ENTEROATEXTO", TOK_ENTEROATEXTO},
    {"DECIMALATEXTO", TOK_DECIMALATEXTO},
    {"CARACTERATEXTO", TOK_CARACTERATEXTO},
    {"INVERTIRTEXTO", TOK_INVERTIRTEXTO},
    {"DECIMALES", TOK_DECIMALES},
    {"SALTO", TOK_SALTO},
    {"SIN_SALTO", TOK_SIN_SALTO},

    // E/S
    {"ESCRIBIR", TOK_ESCRIBIR},
    {"MOSTRAR", TOK_MOSTRAR},
    {"LEER", TOK_LEER},
    {"LEERCARACTER", TOK_LEERCARACTER},
    {"LEERHASTA", TOK_LEERHASTA},
    {"LIMPIARPANTALLA", TOK_LIMPIARPANTALLA},
    {"CLS", TOK_CLS},
    {"CLEAR", TOK_CLEAR},

    // Formato
    {"COLORTEXTO", TOK_COLORTEXTO},
    {"COLORFONDO", TOK_COLORFONDO},
    {"TEXTONEGRITA", TOK_TEXTONEGRITA},
    {"TEXTOCURSIVA", TOK_TEXTOCURSIVA},
    {"TEXTOSUBRAYADO", TOK_TEXTOSUBRAYADO},
    {"RESETTEXTO", TOK_RESETTEXTO},
    {"RESETCOLOR", TOK_RESETCOLOR},

    // Cursor y terminal
    {"CURSOR", TOK_CURSOR},
    {"POSICIONAR", TOK_POSICIONAR},
    {"OCULTARCURSOR", TOK_OCULTARCURSOR},
    {"MOSTRARCURSOR", TOK_MOSTRARCURSOR},
    {"ANCHOTERMINAL", TOK_ANCHOTERMINAL},
    {"ALTOTERMINAL", TOK_ALTOTERMINAL},

    // Tiempo
    {"ESPERAR", TOK_ESPERAR},
    {"TIEMPOMS", TOK_TIEMPOMS},
    {"HORAACTUAL", TOK_HORAACTUAL},
    {"FECHAACTUAL", TOK_FECHAACTUAL},

    // Cálculo
    {"CALCULAR", TOK_CALCULAR},
    {"RESULTADO", TOK_RESULTADO},
    {"ASIGNAR", TOK_ASIGNAR},
    {"EN", TOK_EN},

    // Archivos
    {"ABRIRARCHIVO", TOK_ABRIRARCHIVO},
    {"CERRARARCHIVO", TOK_CERRARARCHIVO},
    {"ESCRIBIRARCHIVO", TOK_ESCRIBIRARCHIVO},
    {"LEERARCHIVO", TOK_LEERARCHIVO},
    {"USARARCHIVO", TOK_USARARCHIVO},
    {"ESCRITURA", TOK_ESCRITURA},
    {"AGREGAR", TOK_AGREGAR},
    {"LECTURA", TOK_LECTURA},
    {"LECTOESCRITURA", TOK_LECTOESCRITURA},

    // Sistema
    {"SISTEMA", TOK_SISTEMA},

    // Dibujo
    {"DIBUJARLINEA", TOK_DIBUJARLINEA},
    {"DIBUJARCIRCULO", TOK_DIBUJARCIRCULO},
    {"RELLENARRECTANGULO", TOK_RELLENARRECTANGULO},

    // Teclado
    {"LEERTECLA", TOK_LEERTECLA},
    {"TECLAMANTENIDA", TOK_TECLAMANTENIDA},
    {"COLISIONRECTANGULOS", TOK_COLISIONRECTANGULOS},

    // GPIO
    {"CONFIGURARPIN", TOK_CONFIGURARPIN},
    {"ESTADOPIN", TOK_ESTADOPIN},
    {"LEERPIN", TOK_LEERPIN},
    {"SALIDA", TOK_SALIDA},
    {"ENTRADA", TOK_ENTRADA},
    {"PULLUP", TOK_PULLUP},
    {"PULLDOWN", TOK_PULLDOWN},
    {"GENERARPWM", TOK_GENERARPWM},
    {"DETENERPWM", TOK_DETENERPWM},
    
    // Base de datos
    {"CONECTARBD", TOK_CONECTARBD},
    {"CERRARBD", TOK_CERRARBD},
    {"EJECUTARBD", TOK_EJECUTARBD},
    {"CONSULTARBD", TOK_CONSULTARBD},
    {"CERRARCONSULTABD", TOK_CERRARCONSULTABD},
    {"INICIARTRANSACCION", TOK_INICIARTRANSACCION},
    {"CONFIRMARTRANSACCION", TOK_CONFIRMARTRANSACCION},
    {"DESHACERTRANSACCION", TOK_DESHACERTRANSACCION},

    // Servidor web
    {"INICIARSERVER", TOK_INICIARSERVER},
    {"DETENERSERVER", TOK_DETENERSERVER},

    // Redondeo
    {"ARRIBA", TOK_ARRIBA},
    {"ABAJO", TOK_ABAJO},

    // Unidades de tiempo
    {"MICROSEGUNDOS", TOK_MICROSEGUNDOS},
    {"MICROS", TOK_MICROS},
    {"US", TOK_US},
    {"MILISEGUNDOS", TOK_MILISEGUNDOS},
    {"MS", TOK_MS},
    {"SEGUNDOS", TOK_SEGUNDOS},
    {"S", TOK_S},
    {"MINUTOS", TOK_MINUTOS},
    {"MIN", TOK_MIN},

    // Colores
    {"NEGRO", TOK_NEGRO},
    {"ROJO", TOK_ROJO},
    {"VERDE", TOK_VERDE},
    {"AMARILLO", TOK_AMARILLO},
    {"AZUL", TOK_AZUL},
    {"MAGENTA", TOK_MAGENTA},
    {"CYAN", TOK_CYAN},
    {"BLANCO", TOK_BLANCO},
    {"GRIS", TOK_GRIS},
    {"ROJOCLARO", TOK_ROJOCLARO},
    {"VERDECLARO", TOK_VERDECLARO},
    {"AMARILLOCLARO", TOK_AMARILLOCLARO},
    {"AZULCLARO", TOK_AZULCLARO},
    {"MAGENTACLARO", TOK_MAGENTACLARO},
    {"CYANCLARO", TOK_CYANCLARO},
    {"BLANCOCLARO", TOK_BLANCOCLARO},

    {NULL, TOK_EOF} // Centinela
};

// ============================================================
// FUNCIONES AUXILIARES INTERNAS
// ============================================================

static char lexer_peek(Lexer* lexer) {
    if (lexer->posicion >= lexer->longitud) return '\0';
    return lexer->codigo[lexer->posicion];
}

static char lexer_peek_siguiente(Lexer* lexer) {
    if (lexer->posicion + 1 >= lexer->longitud) return '\0';
    return lexer->codigo[lexer->posicion + 1];
}

static char lexer_avanzar(Lexer* lexer) {
    if (lexer->posicion >= lexer->longitud) return '\0';
    char c = lexer->codigo[lexer->posicion++];
    if (c == '\n') {
        lexer->linea++;
        lexer->columna = 1;
    } else {
        lexer->columna++;
    }
    return c;
}

static void lexer_set_error(Lexer* lexer, const char* mensaje) {
    lexer->hay_error = true;
    snprintf(lexer->mensaje_error, sizeof(lexer->mensaje_error),
             "Línea %d, columna %d: %s",
             lexer->linea, lexer->columna, mensaje);
}

static Token token_crear(TipoToken tipo, const char* valor, int linea, int columna) {
    Token t;
    t.tipo = tipo;
    t.valor = valor ? strdup(valor) : NULL;
    t.linea = linea;
    t.columna = columna;
    return t;
}

static Token token_error(Lexer* lexer, const char* mensaje) {
    lexer_set_error(lexer, mensaje);
    return token_crear(TOK_ERROR, mensaje, lexer->linea, lexer->columna);
}

// Saltar espacios, tabs y comentarios
static void lexer_saltar_espacios(Lexer* lexer) {
    while (lexer->posicion < lexer->longitud) {
        char c = lexer_peek(lexer);
        
        // Espacios, tabs, saltos de línea
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
            lexer_avanzar(lexer);
            continue;
        }
        
        // Comentarios de línea: //
        if (c == '/' && lexer_peek_siguiente(lexer) == '/') {
            while (lexer->posicion < lexer->longitud && lexer_peek(lexer) != '\n') {
                lexer_avanzar(lexer);
            }
            continue;
        }
        
        // Comentarios de bloque NO permitidos
        if (c == '/' && lexer_peek_siguiente(lexer) == '*') {
            lexer_set_error(lexer, "Comentarios de bloque /* */ no permitidos. Use //");
            return;
        }
        
        break;
    }
}

// Buscar palabra clave en la tabla
static TipoToken buscar_palabra_clave(const char* palabra) {
    for (int i = 0; palabras_clave[i].palabra != NULL; i++) {
        if (strcmp(palabras_clave[i].palabra, palabra) == 0) {
            return palabras_clave[i].tipo;
        }
    }
    return TOK_IDENTIFICADOR;  // No es palabra clave, es identificador
}

// Leer un número (entero, decimal, hex, octal)
static Token lexer_leer_numero(Lexer* lexer) {
    int linea_inicio = lexer->linea;
    int col_inicio = lexer->columna;
    char buffer[256];
    int i = 0;
    bool es_decimal = false;
    
    // Detectar hexadecimal (0x...)
    if (lexer_peek(lexer) == '0' && 
        (lexer_peek_siguiente(lexer) == 'x' || lexer_peek_siguiente(lexer) == 'X')) {
        buffer[i++] = lexer_avanzar(lexer);  // '0'
        buffer[i++] = lexer_avanzar(lexer);  // 'x'
        
        while (i < 255 && isxdigit((unsigned char)lexer_peek(lexer))) {
            buffer[i++] = lexer_avanzar(lexer);
        }
        buffer[i] = '\0';
        return token_crear(TOK_NUMERO_HEX, buffer, linea_inicio, col_inicio);
    }
    
    // Detectar octal (0...)
    if (lexer_peek(lexer) == '0' && 
        lexer_peek_siguiente(lexer) >= '0' && lexer_peek_siguiente(lexer) <= '7') {
        buffer[i++] = lexer_avanzar(lexer);
        
        while (i < 255 && lexer_peek(lexer) >= '0' && lexer_peek(lexer) <= '7') {
            buffer[i++] = lexer_avanzar(lexer);
        }
        buffer[i] = '\0';
        return token_crear(TOK_NUMERO_OCTAL, buffer, linea_inicio, col_inicio);
    }
    
    // Número normal (entero o decimal)
    while (i < 255) {
        char c = lexer_peek(lexer);
        
        if (isdigit((unsigned char)c)) {
            buffer[i++] = lexer_avanzar(lexer);
        } else if (c == '.' && !es_decimal) {
            es_decimal = true;
            buffer[i++] = lexer_avanzar(lexer);
        } else {
            break;
        }
    }
    
    buffer[i] = '\0';
    
    TipoToken tipo = es_decimal ? TOK_NUMERO_DECIMAL : TOK_NUMERO_ENTERO;
    return token_crear(tipo, buffer, linea_inicio, col_inicio);
}

// Leer un string entre comillas dobles
static Token lexer_leer_texto(Lexer* lexer) {
    int linea_inicio = lexer->linea;
    int col_inicio = lexer->columna;
    char buffer[4096];
    int i = 0;
    
    lexer_avanzar(lexer);  // Consumir la comilla de apertura
    
    while (lexer->posicion < lexer->longitud) {
        char c = lexer_peek(lexer);
        
        if (c == '"') {
            lexer_avanzar(lexer);  // Consumir la comilla de cierre
            buffer[i] = '\0';
            return token_crear(TOK_TEXTO, buffer, linea_inicio, col_inicio);
        }
        
        if (c == '\n') {
            return token_error(lexer, "String sin cerrar (fin de línea inesperado)");
        }
        
        if (c == '\\' && i < 4094) {
            lexer_avanzar(lexer);  // Consumir la barra
            char escape = lexer_avanzar(lexer);
            switch (escape) {
                case 'n':  buffer[i++] = '\n'; break;
                case 't':  buffer[i++] = '\t'; break;
                case 'r':  buffer[i++] = '\r'; break;
                case '\\': buffer[i++] = '\\'; break;
                case '"':  buffer[i++] = '"';  break;
                case '\'': buffer[i++] = '\''; break;
                case '$':
                    buffer[i++] = '\x03';
                    break; // Marcador para $ literal
                case '[':
                    buffer[i++] = '\x01';
                    break; // Marcador para [ literal
                case ']':
                    buffer[i++] = '\x02';
                    break; // Marcador para ] literal
                case '0':  buffer[i++] = '\0'; break;
                default:
                    buffer[i++] = escape;
                    break;
            }
        } else if (i < 4095) {
            buffer[i++] = lexer_avanzar(lexer);
        } else {
            return token_error(lexer, "String demasiado largo");
        }
    }
    
    return token_error(lexer, "String sin cerrar (fin de archivo)");
}

// Leer un caracter entre comillas simples
static Token lexer_leer_caracter(Lexer *lexer)
{
    int linea_inicio = lexer->linea;
    int col_inicio = lexer->columna;
    char buffer[8];
    int i = 0;

    lexer_avanzar(lexer); // Consumir la comilla de apertura

    if (lexer->posicion >= lexer->longitud)
    {
        return token_error(lexer, "Caracter sin cerrar");
    }

    char c = lexer_peek(lexer);

    if (c == '\'')
    {
        // Caracter vacío '' se trata como \0 (nulo)
        lexer_avanzar(lexer); // Consumir la comilla de cierre
        buffer[i++] = '\0';
        buffer[i] = '\0';
        return token_crear(TOK_CARACTER, buffer, linea_inicio, col_inicio);
    }

    if (c == '\\')
    {
        lexer_avanzar(lexer);
        char escape = lexer_avanzar(lexer);
        switch (escape)
        {
        case 'n':
            buffer[i++] = '\n';
            break;
        case 't':
            buffer[i++] = '\t';
            break;
        case 'r':
            buffer[i++] = '\r';
            break;
        case '\\':
            buffer[i++] = '\\';
            break;
        case '\'':
            buffer[i++] = '\'';
            break;
        case '0':
            buffer[i++] = '\0';
            break;
        case 'x':
        case 'X':
        {
            // Hexadecimal: \xNN
            char hex[3] = {0};
            for (int k = 0; k < 2 && isxdigit((unsigned char)lexer_peek(lexer)); k++)
            {
                hex[k] = lexer_avanzar(lexer);
            }
            buffer[i++] = (char)strtol(hex, NULL, 16);
            break;
        }
        default:
            buffer[i++] = escape;
            break;
        }
    }
    else
    {
        // Detectar y leer secuencias UTF-8 multi-byte
        unsigned char primer_byte = (unsigned char)lexer_peek(lexer);
        int num_bytes = 1;

        // Determinar cuántos bytes tiene el carácter UTF-8
        if ((primer_byte & 0x80) == 0)
        {
            num_bytes = 1; // ASCII: 0xxxxxxx
        }
        else if ((primer_byte & 0xE0) == 0xC0)
        {
            num_bytes = 2; // 2 bytes: 110xxxxx
        }
        else if ((primer_byte & 0xF0) == 0xE0)
        {
            num_bytes = 3; // 3 bytes: 1110xxxx
        }
        else if ((primer_byte & 0xF8) == 0xF0)
        {
            num_bytes = 4; // 4 bytes: 11110xxx
        }

        // Leer todos los bytes del carácter
        for (int k = 0; k < num_bytes && lexer->posicion < lexer->longitud; k++)
        {
            buffer[i++] = lexer_avanzar(lexer);
        }
    }

    if (lexer_peek(lexer) != '\'')
    {
        return token_error(lexer, "Falta comilla simple de cierre");
    }
    lexer_avanzar(lexer); // Consumir la comilla de cierre

    buffer[i] = '\0';
    return token_crear(TOK_CARACTER, buffer, linea_inicio, col_inicio);
}

// Leer un identificador o palabra clave
static Token lexer_leer_identificador(Lexer *lexer)
{
    int linea_inicio = lexer->linea;
    int col_inicio = lexer->columna;
    char buffer[256];
    char buffer_mayusculas[256]; // Para buscar palabras clave
    int i = 0;

    // Leer el identificador manteniendo el case original
    while (i < 255)
    {
        char c = lexer_peek(lexer);
        if (isalnum((unsigned char)c) || c == '_')
        {
            char original = lexer_avanzar(lexer);
            buffer[i] = original;
            buffer_mayusculas[i] = toupper((unsigned char)original);
            i++;
        }
        else
        {
            break;
        }
    }

    buffer[i] = '\0';
    buffer_mayusculas[i] = '\0';

    // Solo buscar palabras clave si el identificador original está completamente en mayúsculas
    TipoToken tipo = TOK_IDENTIFICADOR;

    // Verificar si el buffer original es igual al buffer en mayúsculas
    bool es_mayusculas = (strcmp(buffer, buffer_mayusculas) == 0);

    if (es_mayusculas)
    {
        // Solo buscar palabras clave si todo está en mayúsculas
        tipo = buscar_palabra_clave(buffer_mayusculas);
    }

    // Retornar el token
    if (tipo != TOK_IDENTIFICADOR)
    {
        // Es palabra clave (todo en mayúsculas)
        return token_crear(tipo, buffer, linea_inicio, col_inicio);
    }
    else
    {
        // Es identificador (puede tener minúsculas)
        return token_crear(TOK_IDENTIFICADOR, buffer, linea_inicio, col_inicio);
    }
}

// Leer una variable ($nombre)
static Token lexer_leer_variable(Lexer* lexer) {
    int linea_inicio = lexer->linea;
    int col_inicio = lexer->columna;
    char buffer[256];
    int i = 0;
    
    lexer_avanzar(lexer);  // Consumir el '$'
    buffer[i++] = '$';
    
    while (i < 255) {
        char c = lexer_peek(lexer);
        if (isalnum((unsigned char)c) || c == '_') {
            buffer[i++] = lexer_avanzar(lexer);
        } else {
            break;
        }
    }
    buffer[i] = '\0';
    
    if (i == 1) {
        return token_error(lexer, "Variable sin nombre después de '$'");
    }
    
    return token_crear(TOK_VARIABLE, buffer, linea_inicio, col_inicio);
}

// ============================================================
// FUNCIONES PÚBLICAS
// ============================================================

Lexer* lexer_crear(const char* codigo) {
    if (!codigo) return NULL;
    
    Lexer* lexer = malloc(sizeof(Lexer));
    if (!lexer) return NULL;
    
    lexer->codigo = codigo;
    lexer->longitud = strlen(codigo);
    lexer->posicion = 0;
    lexer->linea = 1;
    lexer->columna = 1;
    lexer->hay_error = false;
    lexer->mensaje_error[0] = '\0';
    
    return lexer;
}

void lexer_destruir(Lexer* lexer) {
    if (lexer) free(lexer);
}

Token lexer_siguiente_token(Lexer* lexer) {
    if (!lexer)
        return token_crear(TOK_EOF, "", 0, 0);

    // Saltar espacios en blanco y comentarios
    while (lexer->posicion < lexer->longitud)
    {
        char c = lexer_peek(lexer);

        // Saltar espacios, tabs, saltos de línea
        if (isspace((unsigned char)c))
        {
            lexer_avanzar(lexer);
            continue;
        }

        // Comentario de línea: //
        if (c == '/' && lexer_peek_siguiente(lexer) == '/')
        {
            // Saltar hasta el final de la línea
            while (lexer->posicion < lexer->longitud && lexer_peek(lexer) != '\n')
            {
                lexer_avanzar(lexer);
            }
            continue;
        }

        // Comentario de bloque: /* */
        if (c == '/' && lexer_peek_siguiente(lexer) == '*')
        {
            lexer_avanzar(lexer); // Consumir /
            lexer_avanzar(lexer); // Consumir *

            // Saltar hasta encontrar */
            while (lexer->posicion < lexer->longitud)
            {
                if (lexer_peek(lexer) == '*' && lexer_peek_siguiente(lexer) == '/')
                {
                    lexer_avanzar(lexer); // Consumir *
                    lexer_avanzar(lexer); // Consumir /
                    break;
                }
                lexer_avanzar(lexer);
            }
            continue;
        }

        // Si no es espacio ni comentario, salir del bucle
        break;
    }
    if (!lexer)
    {
        Token t = {TOK_ERROR, strdup("Lexer nulo"), 0, 0};
        return t;
    }

    if (lexer->hay_error)
    {
        return token_crear(TOK_ERROR, lexer->mensaje_error, lexer->linea, lexer->columna);
    }

    lexer_saltar_espacios(lexer);

    if (lexer->hay_error)
    {
        return token_crear(TOK_ERROR, lexer->mensaje_error, lexer->linea, lexer->columna);
    }

    if (lexer->posicion >= lexer->longitud)
    {
        return token_crear(TOK_EOF, "", lexer->linea, lexer->columna);
    }

    int linea_inicio = lexer->linea;
    int col_inicio = lexer->columna;
    char c = lexer_peek(lexer);

    // Número
    if (isdigit((unsigned char)c))
    {
        return lexer_leer_numero(lexer);
    }

    // String
    if (c == '"')
    {
        return lexer_leer_texto(lexer);
    }

    // Caracter
    if (c == '\'')
    {
        return lexer_leer_caracter(lexer);
    }

    // Variable
    if (c == '$')
    {
        return lexer_leer_variable(lexer);
    }

    // Identificador o palabra clave
    if (isalpha((unsigned char)c) || c == '_')
    {
        return lexer_leer_identificador(lexer);
    }

    // Símbolos
    lexer_avanzar(lexer);
    switch (c)
    {
    case '(':
        return token_crear(TOK_PARENTESIS_ABRIR, "(", linea_inicio, col_inicio);
    case ')':
        return token_crear(TOK_PARENTESIS_CERRAR, ")", linea_inicio, col_inicio);
    case '[':
        return token_crear(TOK_CORCHETE_ABRIR, "[", linea_inicio, col_inicio);
    case ']':
        return token_crear(TOK_CORCHETE_CERRAR, "]", linea_inicio, col_inicio);
    case '{':
        return token_crear(TOK_LLAVE_ABRIR, "{", linea_inicio, col_inicio);
    case '}':
        return token_crear(TOK_LLAVE_CERRAR, "}", linea_inicio, col_inicio);
    case ',':
        return token_crear(TOK_COMA, ",", linea_inicio, col_inicio);
    case '=':
        return token_crear(TOK_IGUAL, "=", linea_inicio, col_inicio);
    case '+':
        return token_crear(TOK_MAS, "+", linea_inicio, col_inicio);
    case '-':
        return token_crear(TOK_MENOS, "-", linea_inicio, col_inicio);
    case '*':
        return token_crear(TOK_ASTERISCO, "*", linea_inicio, col_inicio);
    case '/':
        return token_crear(TOK_DIVISION, "/", linea_inicio, col_inicio);
    case '%':
        return token_crear(TOK_MODULO, "%", linea_inicio, col_inicio);
    case '^':
        return token_crear(TOK_POTENCIA, "^", linea_inicio, col_inicio);
    default:
    {
        char msg[64];
        snprintf(msg, sizeof(msg), "Caracter inesperado: '%c'", c);
        return token_error(lexer, msg);
    }
    }
}

bool lexer_es_eof(const Token *token)
{
    return token && token->tipo == TOK_EOF;
}

bool lexer_tiene_error(const Lexer* lexer) {
    return lexer && lexer->hay_error;
}

void token_destruir(Token* token) {
    if (token && token->valor) {
        free(token->valor);
        token->valor = NULL;
    }
}

Token token_duplicar(const Token* token) {
    if (!token) {
        Token t = { TOK_ERROR, NULL, 0, 0 };
        return t;
    }
    return token_crear(token->tipo, token->valor, token->linea, token->columna);
}

const char* lexer_nombre_token(TipoToken tipo) {
    switch (tipo) {
        case TOK_EOF: return "EOF";
        case TOK_ERROR: return "ERROR";
        case TOK_NUMERO_ENTERO: return "NUMERO_ENTERO";
        case TOK_NUMERO_DECIMAL: return "NUMERO_DECIMAL";
        case TOK_NUMERO_HEX: return "NUMERO_HEX";
        case TOK_NUMERO_OCTAL: return "NUMERO_OCTAL";
        case TOK_TEXTO: return "TEXTO";
        case TOK_CARACTER: return "CARACTER";
        case TOK_IDENTIFICADOR: return "IDENTIFICADOR";
        case TOK_VARIABLE: return "VARIABLE";
        case TOK_PARENTESIS_ABRIR: return "(";
        case TOK_PARENTESIS_CERRAR: return ")";
        case TOK_CORCHETE_ABRIR: return "[";
        case TOK_CORCHETE_CERRAR: return "]";
        case TOK_LLAVE_ABRIR: return "{";
        case TOK_LLAVE_CERRAR: return "}";
        case TOK_COMA: return ",";
        case TOK_IGUAL: return "=";
        case TOK_MAS: return "+";
        case TOK_MENOS: return "-";
        case TOK_ASTERISCO: return "*";
        case TOK_DIVISION: return "/";
        case TOK_MODULO: return "%";
        case TOK_POTENCIA: return "^";
        
        // Estructura
        case TOK_PROGRAMA: return "PROGRAMA";
        case TOK_FINAL: return "FINAL";
        case TOK_BLOQUE: return "BLOQUE";
        case TOK_PRINCIPAL: return "PRINCIPAL";
        case TOK_FIN: return "FIN";
        case TOK_SI: return "SI";
        case TOK_SINOSI: return "SINOSI";
        case TOK_SINO: return "SINO";
        case TOK_ENTONCES: return "ENTONCES";
        case TOK_PARA: return "PARA";
        case TOK_HACER: return "HACER";
        case TOK_DESDE: return "DESDE";
        case TOK_HASTA: return "HASTA";
        case TOK_PASO: return "PASO";
        case TOK_MIENTRAS: return "MIENTRAS";
        case TOK_REALIZAR: return "REALIZAR";
        case TOK_SEGUN: return "SEGUN";
        case TOK_CASO: return "CASO";
        case TOK_DEFECTO: return "DEFECTO";
        case TOK_FUNCION: return "FUNCION";
        case TOK_SUBPROGRAMA: return "SUBPROGRAMA";
        case TOK_LLAMAR: return "LLAMAR";
        case TOK_RETORNAR: return "RETORNAR";
        case TOK_SALTAR: return "SALTAR";
        case TOK_ETIQUETA: return "ETIQUETA";
        case TOK_CORTE: return "CORTE";
        case TOK_INCLUIR: return "INCLUIR";
        case TOK_A: return "A";
        case TOK_SIN: return "SIN";
        case TOK_POR: return "POR";
        case TOK_SALTO: return "SALTO";
        case TOK_SIN_SALTO: return "SIN_SALTO";

        // Tipos
        case TOK_DECLARAR: return "DECLARAR";
        case TOK_VARIABLE_KW: return "VARIABLE";
        case TOK_CONSTANTE: return "CONSTANTE";
        case TOK_LISTA: return "LISTA";
        case TOK_MATRIZ: return "MATRIZ";
        case TOK_ENTERA: return "ENTERA";
        case TOK_DECIMAL: return "DECIMAL";
        case TOK_TEXTO_KW: return "TEXTO";
        case TOK_CARACTER_KW: return "CARACTER";
        case TOK_ARCHIVO: return "ARCHIVO";
        case TOK_LOGICA: return "LOGICA";
        case TOK_EXTENSO: return "EXTENSO";
        case TOK_SIGNO: return "SIGNO";
        
        // Operadores lógicos y comparación
        case TOK_MAYOR: return "MAYOR";
        case TOK_MENOR: return "MENOR";
        case TOK_IGUAL_KW: return "IGUAL";
        case TOK_DIFERENTE: return "DIFERENTE";
        case TOK_MAYOR_IGUAL: return "MAYORIGUAL";
        case TOK_MENOR_IGUAL: return "MENORIGUAL";
        case TOK_Y: return "Y";
        case TOK_O: return "O";
        case TOK_NO: return "NO";
        case TOK_MOD: return "MOD";
        case TOK_VERDADERO: return "VERDADERO";
        case TOK_FALSO: return "FALSO";
        
        // Bitwise
        case TOK_BITY: return "BITY";
        case TOK_BITO: return "BITO";
        case TOK_BITXOR: return "BITXOR";
        case TOK_BITNO: return "BITNO";
        case TOK_DESPLAZARIZQUIERDA: return "DESPLAZARIZQUIERDA";
        case TOK_DESPLAZARDERECHA: return "DESPLAZARDERECHA";
        case TOK_ROTARIZQUIERDA: return "ROTARIZQUIERDA";
        case TOK_ROTARDERECHA: return "ROTARDERECHA";
        case TOK_LEERBIT: return "LEERBIT";
        case TOK_ACTIVARBIT: return "ACTIVARBIT";
        case TOK_DESACTIVARBIT: return "DESACTIVARBIT";
        case TOK_INVERTIRBYTES: return "INVERTIRBYTES";
        case TOK_CONTARBITS: return "CONTARBITS";
        
        // Funciones matemáticas
        case TOK_SENO: return "SENO";
        case TOK_COSENO: return "COSENO";
        case TOK_TANGENTE: return "TANGENTE";
        case TOK_RAIZ: return "RAIZ";
        case TOK_POTENCIA_FN: return "POTENCIA";
        case TOK_MODULO_FN: return "MODULO";
        case TOK_MAXIMO: return "MAXIMO";
        case TOK_MINIMO: return "MINIMO";
        case TOK_ABSOLUTO: return "ABSOLUTO";
        case TOK_ARCOSENO: return "ARCOSENO";
        case TOK_ARCOCOSENO: return "ARCOCOSENO";
        case TOK_ARCOTANGENTE: return "ARCOTANGENTE";
        case TOK_LOGNATURAL: return "LOGNATURAL";
        case TOK_LOGBASE10: return "LOGBASE10";
        case TOK_LOGBASE2: return "LOGBASE2";
        case TOK_LOGARITMO: return "LOGARITMO";
        case TOK_EXPONENCIAL: return "EXPONENCIAL";
        case TOK_DOSALAX: return "DOSALAX";
        case TOK_REDONDEAR: return "REDONDEAR";
        case TOK_QUITARDECIMAL: return "QUITARDECIMAL";
        case TOK_SIGMOIDE: return "SIGMOIDE";
        case TOK_RAIZCUBICA: return "RAIZCUBICA";
        case TOK_NUMEROPI: return "NUMEROPI";
        case TOK_NUMEROEULER: return "NUMEROEULER";
        case TOK_RAIZDEUNMEDIO: return "RAIZDEUNMEDIO";
        case TOK_LOGNATURALDE2: return "LOGNATURALDE2";
        case TOK_LOGNATURALDE10: return "LOGNATURALDE10";
        case TOK_PI: return "PI";
        case TOK_ALEATORIO: return "ALEATORIO";
        case TOK_ALEATORIOSINSIGNO: return "ALEATORIOSINSIGNO";
        case TOK_LONGITUDLISTA: return "LONGITUDLISTA";
        case TOK_FINARCHIVO: return "FINARCHIVO";
        
        // Funciones de texto
        case TOK_LONGITUDTEXTO: return "LONGITUDTEXTO";
        case TOK_BUSCARTEXTO: return "BUSCARTEXTO";
        case TOK_BUSCARCARACTER: return "BUSCARCARACTER";
        case TOK_COMPARARTEXTO: return "COMPARARTEXTO";
        case TOK_TEXTOVACIO: return "TEXTOVACIO";
        case TOK_TEXTOAENTERO: return "TEXTOAENTERO";
        case TOK_TEXTOADECIMAL: return "TEXTOADECIMAL";
        case TOK_TEXTOACARACTER: return "TEXTOACARACTER";
        case TOK_COPIARTEXTO: return "COPIARTEXTO";
        case TOK_CONCATENARTEXTO: return "CONCATENARTEXTO";
        case TOK_MAYUSCULAS: return "MAYUSCULAS";
        case TOK_MINUSCULAS: return "MINUSCULAS";
        case TOK_RECORTARTEXTO: return "RECORTARTEXTO";
        case TOK_REEMPLAZARTEXTO: return "REEMPLAZARTEXTO";
        case TOK_REPETIRTEXTO: return "REPETIRTEXTO";
        case TOK_EXTRAERTEXTO: return "EXTRAERTEXTO";
        case TOK_DIVIDIRTEXTO: return "DIVIDIRTEXTO";
        case TOK_ENTEROATEXTO: return "ENTEROATEXTO";
        case TOK_DECIMALATEXTO: return "DECIMALATEXTO";
        case TOK_CARACTERATEXTO: return "CARACTERATEXTO";
        case TOK_INVERTIRTEXTO: return "INVERTIRTEXTO";
        case TOK_DECIMALES: return "DECIMALES";

        // E/S
        case TOK_ESCRIBIR: return "ESCRIBIR";
        case TOK_MOSTRAR: return "MOSTRAR";
        case TOK_LEER: return "LEER";
        case TOK_LEERCARACTER: return "LEERCARACTER";
        case TOK_LEERHASTA: return "LEERHASTA";
        case TOK_LIMPIARPANTALLA: return "LIMPIARPANTALLA";
        case TOK_CLS: return "CLS";
        case TOK_CLEAR: return "CLEAR";
        
        // Formato
        case TOK_COLORTEXTO: return "COLORTEXTO";
        case TOK_COLORFONDO: return "COLORFONDO";
        case TOK_TEXTONEGRITA: return "TEXTONEGRITA";
        case TOK_TEXTOCURSIVA: return "TEXTOCURSIVA";
        case TOK_TEXTOSUBRAYADO: return "TEXTOSUBRAYADO";
        case TOK_RESETTEXTO: return "RESETTEXTO";
        case TOK_RESETCOLOR: return "RESETCOLOR";
        
        // Cursor y terminal
        case TOK_CURSOR: return "CURSOR";
        case TOK_POSICIONAR: return "POSICIONAR";
        case TOK_OCULTARCURSOR: return "OCULTARCURSOR";
        case TOK_MOSTRARCURSOR: return "MOSTRARCURSOR";
        case TOK_ANCHOTERMINAL: return "ANCHOTERMINAL";
        case TOK_ALTOTERMINAL: return "ALTOTERMINAL";
        
        // Tiempo
        case TOK_ESPERAR: return "ESPERAR";
        case TOK_TIEMPOMS: return "TIEMPOMS";
        case TOK_HORAACTUAL: return "HORAACTUAL";
        case TOK_FECHAACTUAL: return "FECHAACTUAL";
        
        // Cálculo
        case TOK_CALCULAR: return "CALCULAR";
        case TOK_RESULTADO: return "RESULTADO";
        case TOK_ASIGNAR: return "ASIGNAR";
        case TOK_EN: return "EN";
        
        // Archivos
        case TOK_ABRIRARCHIVO: return "ABRIRARCHIVO";
        case TOK_CERRARARCHIVO: return "CERRARARCHIVO";
        case TOK_ESCRIBIRARCHIVO: return "ESCRIBIRARCHIVO";
        case TOK_LEERARCHIVO: return "LEERARCHIVO";
        case TOK_USARARCHIVO: return "USARARCHIVO";
        case TOK_ESCRITURA: return "ESCRITURA";
        case TOK_AGREGAR: return "AGREGAR";
        case TOK_LECTURA: return "LECTURA";
        case TOK_LECTOESCRITURA: return "LECTOESCRITURA";
        
        // Sistema
        case TOK_SISTEMA: return "SISTEMA";
        
        // Dibujo
        case TOK_DIBUJARLINEA: return "DIBUJARLINEA";
        case TOK_DIBUJARCIRCULO: return "DIBUJARCIRCULO";
        case TOK_RELLENARRECTANGULO: return "RELLENARRECTANGULO";
        
        // Teclado
        case TOK_LEERTECLA: return "LEERTECLA";
        case TOK_TECLAMANTENIDA: return "TECLAMANTENIDA";
        case TOK_COLISIONRECTANGULOS: return "COLISIONRECTANGULOS";
        
        // GPIO
        case TOK_CONFIGURARPIN:
            return "CONFIGURARPIN";
        case TOK_ESTADOPIN:
            return "ESTADOPIN";
        case TOK_LEERPIN:
            return "LEERPIN";
        case TOK_SALIDA:
            return "SALIDA";
        case TOK_ENTRADA:
            return "ENTRADA";
        case TOK_PULLUP:
            return "PULLUP";
        case TOK_PULLDOWN:
            return "PULLDOWN";
        case TOK_GENERARPWM: 
            return "GENERARPWM";
        case TOK_DETENERPWM: 
            return "DETENERPWM";
        
        // Base de datos
        case TOK_CONECTARBD: return "CONECTARBD";
        case TOK_CERRARBD: return "CERRARBD";
        case TOK_EJECUTARBD: return "EJECUTARBD";
        case TOK_CONSULTARBD: return "CONSULTARBD";
        case TOK_CERRARCONSULTABD: return "CERRARCONSULTABD";
        case TOK_INICIARTRANSACCION: return "INICIARTRANSACCION";
        case TOK_CONFIRMARTRANSACCION: return "CONFIRMARTRANSACCION";
        case TOK_DESHACERTRANSACCION: return "DESHACERTRANSACCION";
        
        // Servidor web
        case TOK_INICIARSERVER: return "INICIARSERVER";
        case TOK_DETENERSERVER: return "DETENERSERVER";
        
        // Redondeo
        case TOK_ARRIBA: return "ARRIBA";
        case TOK_ABAJO: return "ABAJO";
        case TOK_ENTERO_KW: return "ENTERO";
        
        // Unidades de tiempo
        case TOK_MICROSEGUNDOS: return "MICROSEGUNDOS";
        case TOK_MICROS: return "MICROS";
        case TOK_US: return "US";
        case TOK_MILISEGUNDOS: return "MILISEGUNDOS";
        case TOK_MS: return "MS";
        case TOK_SEGUNDOS: return "SEGUNDOS";
        case TOK_S: return "S";
        case TOK_MINUTOS: return "MINUTOS";
        case TOK_MIN: return "MIN";
        
        // Colores
        case TOK_NEGRO: return "NEGRO";
        case TOK_ROJO: return "ROJO";
        case TOK_VERDE: return "VERDE";
        case TOK_AMARILLO: return "AMARILLO";
        case TOK_AZUL: return "AZUL";
        case TOK_MAGENTA: return "MAGENTA";
        case TOK_CYAN: return "CYAN";
        case TOK_BLANCO: return "BLANCO";
        case TOK_GRIS: return "GRIS";
        case TOK_ROJOCLARO: return "ROJOCLARO";
        case TOK_VERDECLARO: return "VERDECLARO";
        case TOK_AMARILLOCLARO: return "AMARILLOCLARO";
        case TOK_AZULCLARO: return "AZULCLARO";
        case TOK_MAGENTACLARO: return "MAGENTACLARO";
        case TOK_CYANCLARO: return "CYANCLARO";
        case TOK_BLANCOCLARO: return "BLANCOCLARO";
        
        // Tipos aleatorios
        case TOK_ENTERO_RND: return "ENTERO";
        case TOK_DECIMAL_RND: return "DECIMAL";
        
        default: return "TOKEN_DESCONOCIDO";
    }
}
