/*
 * Nico v2.0.0 - Intérprete Educativo de Scripting en Español
 * @file:         lexer.h
 * @author:       Diego Alejandro Majluff (Diseño, Arquitectura y Supervisión)
 * @ai_assist:    Qwen (Alibaba Cloud) - Implementación, Debugging y Optimización
 * @license:      MIT / Personal Use (ver LICENSE)
 * @description:  Definiciones del analizador léxico. Contiene la enumeración
 *                de tokens (palabras clave, operadores, literales), estructura
 *                del lexer y funciones de tokenización.
 */
#ifndef LEXER_H
#define LEXER_H

#include <stdbool.h>
#include <stddef.h>

// ============================================================
// TIPOS DE TOKEN
// ============================================================
typedef enum
{
    // Fin de archivo
    TOK_EOF,
    TOK_ERROR,

    // Literales
    TOK_NUMERO_ENTERO,  // 42, -5
    TOK_NUMERO_DECIMAL, // 3.14, -0.5
    TOK_NUMERO_HEX,     // 0xFF, 0x1A
    TOK_NUMERO_OCTAL,   // 0777, 010
    TOK_TEXTO,          // "hola mundo"
    TOK_CARACTER,       // 'A', '\n'

    // Identificadores y variables
    TOK_IDENTIFICADOR, // nombreDeFuncion
    TOK_VARIABLE,      // $mi_variable

    // Símbolos
    TOK_PARENTESIS_ABRIR,  // (
    TOK_PARENTESIS_CERRAR, // )
    TOK_CORCHETE_ABRIR,    // [
    TOK_CORCHETE_CERRAR,   // ]
    TOK_LLAVE_ABRIR,       // {
    TOK_LLAVE_CERRAR,      // }
    TOK_COMA,              // ,
    TOK_IGUAL,             // =
    TOK_MAS,               // +
    TOK_MENOS,             // -
    TOK_ASTERISCO,         // *
    TOK_DIVISION,          // /
    TOK_MODULO,            // %
    TOK_POTENCIA,          // ^

    // --------------------------------------------------------
    // PALABRAS CLAVE DE ESTRUCTURA
    // --------------------------------------------------------
    TOK_PROGRAMA,
    TOK_FINAL,
    TOK_BLOQUE,
    TOK_PRINCIPAL,
    TOK_FIN,
    TOK_SI,
    TOK_SINO,
    TOK_SINOSI,
    TOK_ENTONCES,
    TOK_PARA,
    TOK_HACER,
    TOK_DESDE,
    TOK_HASTA,
    TOK_PASO,
    TOK_MIENTRAS,
    TOK_REALIZAR,
    TOK_SEGUN,
    TOK_CASO,
    TOK_DEFECTO,
    TOK_FUNCION,
    TOK_SUBPROGRAMA,
    TOK_LLAMAR,
    TOK_RETORNAR,
    TOK_SALTAR,
    TOK_ETIQUETA,
    TOK_CORTE,
    TOK_INCLUIR,
    TOK_A,
    TOK_SIN,
    TOK_POR,

    // --------------------------------------------------------
    // PALABRAS CLAVE DE TIPOS
    // --------------------------------------------------------
    TOK_DECLARAR,
    TOK_VARIABLE_KW, // VARIABLE (como palabra clave)
    TOK_CONSTANTE,
    TOK_LISTA,
    TOK_MATRIZ,
    TOK_ENTERA,
    TOK_DECIMAL,
    TOK_TEXTO_KW,    // TEXTO (como palabra clave)
    TOK_CARACTER_KW, // CARACTER (como palabra clave)
    TOK_ARCHIVO,
    TOK_LOGICA,
    TOK_EXTENSO,
    TOK_SIGNO,

    // --------------------------------------------------------
    // OPERADORES LÓGICOS Y DE COMPARACIÓN
    // --------------------------------------------------------
    TOK_MAYOR,
    TOK_MENOR,
    TOK_IGUAL_KW, // IGUAL (palabra clave)
    TOK_DIFERENTE,
    TOK_MAYOR_IGUAL, // >=
    TOK_MENOR_IGUAL, // <=
    TOK_Y,
    TOK_O,
    TOK_NO,
    TOK_MOD,
    TOK_VERDADERO,
    TOK_FALSO,

    // --------------------------------------------------------
    // OPERADORES BITWISE
    // --------------------------------------------------------
    TOK_BITY,
    TOK_BITO,
    TOK_BITXOR,
    TOK_BITNO,
    TOK_DESPLAZARIZQUIERDA,
    TOK_DESPLAZARDERECHA,
    TOK_ROTARIZQUIERDA,
    TOK_ROTARDERECHA,
    TOK_LEERBIT,
    TOK_ACTIVARBIT,
    TOK_DESACTIVARBIT,
    TOK_INVERTIRBYTES,
    TOK_CONTARBITS,

    // --------------------------------------------------------
    // FUNCIONES MATEMÁTICAS
    // --------------------------------------------------------
    TOK_SENO,
    TOK_COSENO,
    TOK_TANGENTE,
    TOK_RAIZ,
    TOK_POTENCIA_FN,
    TOK_MODULO_FN,
    TOK_MAXIMO,
    TOK_MINIMO,
    TOK_ABSOLUTO,
    TOK_ARCOSENO,
    TOK_ARCOCOSENO,
    TOK_ARCOTANGENTE,
    TOK_LOGNATURAL,
    TOK_LOGBASE10,
    TOK_LOGBASE2,
    TOK_LOGARITMO,
    TOK_EXPONENCIAL,
    TOK_DOSALAX,
    TOK_REDONDEAR,
    TOK_QUITARDECIMAL,
    TOK_SIGMOIDE,
    TOK_RAIZCUBICA,
    TOK_NUMEROPI,
    TOK_NUMEROEULER,
    TOK_RAIZDEUNMEDIO,
    TOK_LOGNATURALDE2,
    TOK_LOGNATURALDE10,
    TOK_PI,
    TOK_ALEATORIO,
    TOK_ALEATORIOSINSIGNO,
    TOK_LONGITUDLISTA,
    TOK_FINARCHIVO,

    // --------------------------------------------------------
    // FUNCIONES DE TEXTO
    // --------------------------------------------------------
    TOK_LONGITUDTEXTO,
    TOK_BUSCARTEXTO,
    TOK_BUSCARCARACTER,
    TOK_COMPARARTEXTO,
    TOK_TEXTOVACIO,
    TOK_TEXTOAENTERO,
    TOK_TEXTOADECIMAL,
    TOK_TEXTOACARACTER,
    TOK_COPIARTEXTO,
    TOK_CONCATENARTEXTO,
    TOK_MAYUSCULAS,
    TOK_MINUSCULAS,
    TOK_RECORTARTEXTO,
    TOK_REEMPLAZARTEXTO,
    TOK_REPETIRTEXTO,
    TOK_EXTRAERTEXTO,
    TOK_DIVIDIRTEXTO,
    TOK_ENTEROATEXTO,
    TOK_DECIMALATEXTO,
    TOK_CARACTERATEXTO,
    TOK_INVERTIRTEXTO,
    TOK_DECIMALES,
    TOK_SALTO,
    TOK_SIN_SALTO,

    // --------------------------------------------------------
    // COMANDOS DE E/S
    // --------------------------------------------------------
    TOK_ESCRIBIR,
    TOK_MOSTRAR,
    TOK_LEER,
    TOK_LEERCARACTER,
    TOK_LEERHASTA,
    TOK_LIMPIARPANTALLA,
    TOK_CLS,
    TOK_CLEAR,

    // --------------------------------------------------------
    // COMANDOS DE FORMATO
    // --------------------------------------------------------
    TOK_COLORTEXTO,
    TOK_COLORFONDO,
    TOK_TEXTONEGRITA,
    TOK_TEXTOCURSIVA,
    TOK_TEXTOSUBRAYADO,
    TOK_RESETTEXTO,
    TOK_RESETCOLOR,

    // --------------------------------------------------------
    // COMANDOS DE CURSOR Y TERMINAL
    // --------------------------------------------------------
    TOK_CURSOR,
    TOK_POSICIONAR,
    TOK_OCULTARCURSOR,
    TOK_MOSTRARCURSOR,
    TOK_ANCHOTERMINAL,
    TOK_ALTOTERMINAL,

    // --------------------------------------------------------
    // COMANDOS DE TIEMPO
    // --------------------------------------------------------
    TOK_ESPERAR,
    TOK_TIEMPOMS,
    TOK_HORAACTUAL,
    TOK_FECHAACTUAL,

    // --------------------------------------------------------
    // COMANDOS DE CÁLCULO
    // --------------------------------------------------------
    TOK_CALCULAR,
    TOK_RESULTADO,
    TOK_ASIGNAR,
    TOK_EN,

    // --------------------------------------------------------
    // COMANDOS DE ARCHIVOS
    // --------------------------------------------------------
    TOK_ABRIRARCHIVO,
    TOK_CERRARARCHIVO,
    TOK_ESCRIBIRARCHIVO,
    TOK_LEERARCHIVO,
    TOK_USARARCHIVO,
    TOK_ESCRITURA,
    TOK_AGREGAR,
    TOK_LECTURA,
    TOK_LECTOESCRITURA,

    // --------------------------------------------------------
    // COMANDO SISTEMA
    // --------------------------------------------------------
    TOK_SISTEMA,

    // --------------------------------------------------------
    // COMANDOS DE DIBUJO
    // --------------------------------------------------------
    TOK_DIBUJARLINEA,
    TOK_DIBUJARCIRCULO,
    TOK_RELLENARRECTANGULO,

    // --------------------------------------------------------
    // COMANDOS DE TECLADO
    // --------------------------------------------------------
    TOK_LEERTECLA,
    TOK_COLISIONRECTANGULOS,

    // --------------------------------------------------------
    // COMANDOS GPIO
    // --------------------------------------------------------
    TOK_CONFIGURARPIN,
    TOK_ESTADOPIN,
    TOK_LEERPIN,
    TOK_SALIDA,
    TOK_ENTRADA,
    TOK_PULLUP,
    TOK_PULLDOWN,
    // Comandos PWM
    TOK_GENERARPWM,
    TOK_DETENERPWM,

    // --------------------------------------------------------
    // COMANDOS BASE DE DATOS
    // --------------------------------------------------------
    TOK_CONECTARBD,
    TOK_CERRARBD,
    TOK_EJECUTARBD,
    TOK_CONSULTARBD,
    TOK_CERRARCONSULTABD,
    TOK_INICIARTRANSACCION,
    TOK_CONFIRMARTRANSACCION,
    TOK_DESHACERTRANSACCION,

    // --------------------------------------------------------
    // COMANDOS SERVIDOR WEB
    // --------------------------------------------------------
    TOK_INICIARSERVER,
    TOK_DETENERSERVER,

    // --------------------------------------------------------
    // MANEJO DE ERRORES
    // --------------------------------------------------------
    TOK_ALERTA,
    TOK_INTENTAR,
    TOK_ATRAPAR,

    // --------------------------------------------------------
    // MODOS DE REDONDEO
    // --------------------------------------------------------
    TOK_ARRIBA,
    TOK_ABAJO,
    TOK_ENTERO_KW,

    // --------------------------------------------------------
    // UNIDADES DE TIEMPO
    // --------------------------------------------------------
    TOK_MICROSEGUNDOS,
    TOK_MICROS,
    TOK_US,
    TOK_MILISEGUNDOS,
    TOK_MS,
    TOK_SEGUNDOS,
    TOK_S,
    TOK_MINUTOS,
    TOK_MIN,

    // --------------------------------------------------------
    // COLORES (para COLORTEXTO/COLORFONDO)
    // --------------------------------------------------------
    TOK_NEGRO,
    TOK_ROJO,
    TOK_VERDE,
    TOK_AMARILLO,
    TOK_AZUL,
    TOK_MAGENTA,
    TOK_CYAN,
    TOK_BLANCO,
    TOK_GRIS,
    TOK_ROJOCLARO,
    TOK_VERDECLARO,
    TOK_AMARILLOCLARO,
    TOK_AZULCLARO,
    TOK_MAGENTACLARO,
    TOK_CYANCLARO,
    TOK_BLANCOCLARO,

    // --------------------------------------------------------
    // TIPOS ALEATORIOS
    // --------------------------------------------------------
    TOK_ENTERO_RND,
    TOK_DECIMAL_RND

} TipoToken;

// ============================================================
// ESTRUCTURA DEL TOKEN
// ============================================================
typedef struct {
    TipoToken tipo;
    char* valor;          // Texto literal del token
    int linea;            // Número de línea (1-based)
    int columna;          // Número de columna (1-based)
} Token;

// ============================================================
// ESTRUCTURA DEL LEXER
// ============================================================
typedef struct {
    const char* codigo;       // Código fuente completo
    size_t longitud;          // Longitud del código
    size_t posicion;          // Posición actual
    int linea;                // Línea actual
    int columna;              // Columna actual
    bool hay_error;           // Flag de error
    char mensaje_error[256];  // Mensaje de error
} Lexer;

// ============================================================
// FUNCIONES PÚBLICAS
// ============================================================

// Inicializar el lexer con el código fuente
Lexer* lexer_crear(const char* codigo);

// Liberar el lexer
void lexer_destruir(Lexer* lexer);

// Obtener el siguiente token
Token lexer_siguiente_token(Lexer* lexer);

// Ver si llegamos al final
bool lexer_es_eof(const Token* token);

// Ver si hay error
bool lexer_tiene_error(const Lexer* lexer);

// Obtener el nombre de un tipo de token (para debugging)
const char* lexer_nombre_token(TipoToken tipo);

// Liberar un token (su valor es dinámico)
void token_destruir(Token* token);

// Duplicar un token
Token token_duplicar(const Token* token);

#endif // LEXER_H
