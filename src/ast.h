/*
 * Nico v2.0.0 - Intérprete Educativo de Scripting en Español
 * @file:         ast.h
 * @author:       Diego Alejandro Majluff (Diseño, Arquitectura y Supervisión)
 * @ai_assist:    Qwen (Alibaba Cloud) - Implementación, Debugging y Optimización
 * @license:      MIT / Personal Use (ver LICENSE)
 * @description:  Definiciones del Árbol de Sintaxis Abstracta (AST). Contiene
 *                los tipos de nodos, estructuras de datos para representar
 *                expresiones, sentencias, bloques y programas completos.
 */
#ifndef AST_H
#define AST_H

#include <stdbool.h>

// Tipos de nodos del AST
typedef enum
{
    // Programa
    AST_PROGRAMA,
    AST_BLOQUE_PRINCIPAL,

    // Declaraciones
    AST_DECLARACION_VARIABLE,
    AST_DECLARACION_CONSTANTE,
    AST_DECLARACION_LISTA,
    AST_DECLARACION_MATRIZ,
    AST_DECLARACION_MATRIZ3D,
    AST_DECLARACION_TEXTO_EXTENSO,
    AST_DECLARACION_ARCHIVO,

    // Estructuras de control
    AST_SI,
    AST_SINO_SI,
    AST_SINO,
    AST_PARA,
    AST_MIENTRAS,
    AST_REALIZAR,
    AST_SEGUN_CASO,
    AST_CASO,
    AST_POR_DEFECTO,
    AST_CORTE,

    // Funciones y subprogramas
    AST_FUNCION,
    AST_SUBPROGRAMA,
    AST_LLAMAR_A,
    AST_RETORNAR,

    // Expresiones
    AST_LITERAL_NUMERO,
    AST_LITERAL_TEXTO,
    AST_LITERAL_CARACTER,
    AST_LITERAL_LOGICO,
    AST_VARIABLE,
    AST_ACCESO_LISTA,
    AST_ACCESO_MATRIZ,
    AST_ACCESO_MATRIZ3D,
    AST_OPERADOR_BINARIO,
    AST_OPERADOR_UNARIO,
    AST_LLAMADA_FUNCION,
    AST_LLAMADA_FUNCION_MODIFICADORA,

    // Comandos de E/S
    AST_ESCRIBIR,
    AST_LEER,
    AST_LEERCARACTER,
    AST_LEERHASTA,
    AST_LIMPIARPANTALLA,

    // Comandos de formato
    AST_COLORTEXTO,
    AST_COLORFONDO,
    AST_TEXTONEGRITA,
    AST_TEXTOCURSIVA,
    AST_TEXTOSUBRAYADO,
    AST_RESETTEXTO,
    AST_RESETCOLOR,

    // Comandos de cursor y terminal
    AST_CURSOR,
    AST_POSICIONAR,
    AST_OCULTARCURSOR,
    AST_MOSTRARCURSOR,
    AST_ANCHOTERMINAL,
    AST_ALTOTERMINAL,

    // Comandos de tiempo
    AST_ESPERAR,
    AST_TIEMPOMS,
    AST_HORAACTUAL,
    AST_FECHAACTUAL,

    // Comandos de cálculo
    AST_CALCULAR,
    AST_RESULTADO,
    AST_ASIGNAR,

    // Comandos de archivos
    AST_ABRIRARCHIVO,
    AST_CERRARARCHIVO,
    AST_ESCRIBIRARCHIVO,
    AST_LEERARCHIVO,
    AST_USARARCHIVO,

    // Comandos de sistema
    AST_SISTEMA,

    // Comandos de dibujo
    AST_DIBUJARLINEA,
    AST_DIBUJARCIRCULO,
    AST_RELLENARRECTANGULO,

    // Comandos de teclado
    AST_LEERTECLA,
    AST_COLISIONRECTANGULOS,

    // Comandos GPIO
    AST_CONFIGURARPIN,
    AST_ESTADOPIN,
    AST_LEERPIN,
    AST_PWM,
    AST_DETENERPWM,

    // Comandos de base de datos
    AST_CONECTARBD,
    AST_CERRARBD,
    AST_EJECUTARBD,
    AST_CONSULTARBD,
    AST_CERRARCONSULTABD,
    AST_INICIARTRANSACCION,
    AST_CONFIRMARTRANSACCION,
    AST_DESHACERTRANSACCION,

    // Comandos de servidor web
    AST_INICIARSERVER,
    AST_CERRARSERVER,

    // Etiquetas y saltos
    AST_ETIQUETA,
    AST_SALTAR_A,

    // Inclusión de archivos
    AST_INCLUIR,

    // Manejo de errores
    AST_ALERTA,
    AST_INTENTAR_ATRAPAR,

    // Bloque de código
    AST_BLOQUE,

    // Funciones de texto
    AST_COPIARTEXTO,
    AST_CONCATENARTEXTO,
    AST_MAYUSCULAS,
    AST_MINUSCULAS,
    AST_RECORTARTEXTO,
    AST_REEMPLAZARTEXTO,
    AST_REPETIRTEXTO,
    AST_EXTRAERTEXTO,
    AST_DIVIDIRTEXTO,
    AST_ENTEROATEXTO,
    AST_DECIMALATEXTO,
    AST_CARACTERATEXTO
} TipoNodo;

// Tipos de datos
typedef enum
{
    TIPO_ENTERO,
    TIPO_ENTERO_SIN_SIGNO,
    TIPO_DECIMAL,
    TIPO_DECIMAL_SIN_SIGNO,
    TIPO_CARACTER,
    TIPO_CARACTER_SIN_SIGNO,
    TIPO_TEXTO,
    TIPO_TEXTO_EXTENSO,
    TIPO_LOGICA,
    TIPO_LISTA,
    TIPO_MATRIZ,
    TIPO_ARCHIVO,
    TIPO_VACIO 
} TipoDato;

// Operadores binarios
typedef enum {
    OP_SUMA,
    OP_RESTA,
    OP_MULTIPLICACION,
    OP_DIVISION,
    OP_MODULO,
    OP_POTENCIA,
    OP_IGUAL,
    OP_DIFERENTE,
    OP_MAYOR,
    OP_MENOR,
    OP_MAYOR_IGUAL,
    OP_MENOR_IGUAL,
    OP_Y_LOGICO,
    OP_O_LOGICO,
    OP_BIT_AND,
    OP_BIT_OR,
    OP_BIT_XOR,
    OP_DESPLAZAR_IZQ,
    OP_DESPLAZAR_DER,
    OP_ROTAR_IZQ,
    OP_ROTAR_DER,
    OP_LEER_BIT,
    OP_ACTIVAR_BIT,
    OP_DESACTIVAR_BIT
} OperadorBinario;

// Operadores unarios
typedef enum {
    OP_NEGACION,
    OP_NO_LOGICO,
    OP_BIT_NOT
} OperadorUnario;

// Estructura del nodo AST
typedef struct NodoAST {
    TipoNodo tipo;
    int linea;
    int columna;
    
    union {
        // Programa
        struct {
            char* nombre;
            struct NodoAST* declaraciones;
            struct NodoAST* bloque_principal;
            struct NodoAST* funciones;
            struct NodoAST* subprogramas;
        } programa;
        
        // Bloque principal
        struct {
            struct NodoAST* sentencias;
        } bloque_principal;
        
        // Declaración de variable
        struct {
            char* nombre;
            TipoDato tipo_dato;
            struct NodoAST* valor_inicial;
        } declaracion_variable;
        
        // Declaración de constante
        struct {
            char* nombre;
            TipoDato tipo_dato;
            struct NodoAST* valor;
        } declaracion_constante;
        
        // Declaración de lista
        struct {
            char* nombre;
            TipoDato tipo_elemento;
            int tamano;
            struct NodoAST* valores_iniciales;
        } declaracion_lista;
        
        // Declaración de matriz
        struct {
            char* nombre;
            TipoDato tipo_elemento;
            int filas;
            int columnas;
            struct NodoAST* valores_iniciales;
        } declaracion_matriz;

        struct {
            char *nombre;
            TipoDato tipo_elemento;
            int dim1;
            int dim2;
            int dim3;
            struct NodoAST *valores_iniciales;
        } declaracion_matriz3d;

        // Declaración de texto extenso
        struct {
            char* nombre;
            char* valor_inicial;
        } declaracion_texto_extenso;
        
        // Declaración de archivo
        struct {
            char* nombre;
            char* ruta;
            int modo;
        } declaracion_archivo;
        
        // SI
        struct {
            struct NodoAST* condicion;
            struct NodoAST* bloque_si;
            struct NodoAST* sino_si;
            struct NodoAST* bloque_sino;
        } si;
        
        // SINO SI
        struct {
            struct NodoAST* condicion;
            struct NodoAST* bloque;
            struct NodoAST* siguiente_sino_si;
        } sino_si;
        
        // SINO
        struct {
            struct NodoAST* bloque;
        } sino;
        
        // PARA
        struct {
            char* variable;
            struct NodoAST* inicio;
            struct NodoAST* fin;
            struct NodoAST* paso;
            struct NodoAST* bloque;
        } para;
        
        // MIENTRAS
        struct {
            struct NodoAST* condicion;
            struct NodoAST* bloque;
        } mientras;
        
        // REALIZAR
        struct {
            struct NodoAST* bloque;
            struct NodoAST* condicion;
        } realizar;
        
        // SEGUN CASO
        struct {
            struct NodoAST* expresion;
            struct NodoAST* casos;
        } segun_caso;
        
        // CASO
        struct {
            int valor;
            struct NodoAST* bloque;
        } caso;
        
        // POR DEFECTO
        struct {
            struct NodoAST* bloque;
        } por_defecto;
        
        // CORTE
        struct {
            int dummy;
        } corte;
        
        // Función
        struct {
            char* nombre;
            TipoDato tipo_retorno;
            char** parametros;
            int num_parametros;
            struct NodoAST* bloque;
        } funcion;
        
        // Subprograma
        struct {
            char* nombre;
            char** parametros;
            int num_parametros;
            struct NodoAST* bloque;
        } subprograma;
        
        // LLAMAR A
        struct {
            char* nombre;
            struct NodoAST* argumentos;
        } llamar_a;
        
        // RETORNAR
        struct {
            struct NodoAST* valor;
        } retornar;
        
        // Literal número
        struct {
            char *valor_str;
            double valor;
        } literal_numero;
        
        // Literal texto
        struct {
            char* valor;
        } literal_texto;
        
        // Literal caracter
        struct {
            char valor[4];
        } literal_caracter;
        
        // Literal lógico
        struct {
            bool valor;
        } literal_logico;
        
        // Variable
        struct {
            char* nombre;
        } variable;
        
        // Acceso a lista
        struct {
            char* nombre_lista;
            struct NodoAST* indice;
        } acceso_lista;
        
        // Acceso a matriz
        struct {
            char* nombre_matriz;
            struct NodoAST* fila;
            struct NodoAST* columna;
        } acceso_matriz;

        struct {
            char *nombre_matriz;
            struct NodoAST *indice1;
            struct NodoAST *indice2;
            struct NodoAST *indice3;
        } acceso_matriz3d;

        // Operador binario
        struct {
            OperadorBinario operador;
            struct NodoAST* izquierdo;
            struct NodoAST* derecho;
        } operador_binario;
        
        // Operador unario
        struct {
            OperadorUnario operador;
            struct NodoAST* operando;
        } operador_unario;
        
        // Llamada a función
        struct {
            char* nombre_funcion;
            struct NodoAST* argumentos;
        } llamada_funcion;

        struct
        {
            char *nombre_funcion;
            struct NodoAST *argumentos;
        } llamada_funcion_modificadora;

        // ESCRIBIR
        struct
        {
            struct NodoAST *expresion;
            bool salto_linea;
            int *formatos_decimales; // Array con decimales para cada variable
            int num_formatos;        // Cantidad de formatos
        } escribir;

        // LEER
        struct {
            char* variable;
        } leer;

        // LEERCARACTER
        struct {
            char* variable;
        } leercaracter;

        // LEERHASTA
        struct
        {
            char *variable;
            struct NodoAST *delimitador;
        } leerhasta;

        // LIMPIARPANTALLA
        struct {
            int dummy;
        } limpiarpantalla;
        
        // COLORTEXTO
        struct {
            char* color;
        } colortexto;
        
        // COLORFONDO
        struct {
            char* color;
        } colorfondo;
        
        // TEXTONEGRITA
        struct {
            int dummy;
        } textonegrita;
        
        // TEXTOCURSIVA
        struct {
            int dummy;
        } textocursiva;
        
        // TEXTOSUBRAYADO
        struct {
            int dummy;
        } textosubrayado;
        
        // RESETTEXTO
        struct {
            int dummy;
        } resettexto;
        
        // RESETCOLOR
        struct {
            int dummy;
        } resetcolor;
        
        // CURSOR
        struct {
            struct NodoAST* fila;
            struct NodoAST* columna;
        } cursor;
        
        // POSICIONAR
        struct {
            struct NodoAST* fila;
            struct NodoAST* columna;
        } posicionar;
        
        // OCULTARCURSOR
        struct {
            int dummy;
        } ocultarcursor;
        
        // MOSTRARCURSOR
        struct {
            int dummy;
        } mostrarcursor;
        
        // ANCHOTERMINAL
        struct {
            char* variable;
        } anchoterminal;
        
        // ALTOTERMINAL
        struct {
            char* variable;
        } altoterminal;
        
        // ESPERAR
        struct {
            struct NodoAST* cantidad;
            char* unidad;
        } esperar;
        
        // TIEMPOMS
        struct {
            char* variable;
        } tiempoms;
        
        // HORAACTUAL
        struct {
            char* variable;
        } horaactual;
        
        // FECHAACTUAL
        struct {
            char* variable;
        } fechaactual;
        
        // CALCULAR
        struct {
            struct NodoAST *destino; 
            struct NodoAST* expresion;
        } calcular;
        
        // RESULTADO
        struct {
            struct NodoAST* expresion;
        } resultado;
        
        // ASIGNAR
        struct {
            struct NodoAST *destino;
            struct NodoAST* valor;
        } asignar;
        
        // ABRIRARCHIVO
        struct {
            char* variable;
            char* ruta;
            int modo;
        } abrirarchivo;
        
        // CERRARARCHIVO
        struct {
            char* variable;
        } cerrararchivo;
        
        // ESCRIBIRARCHIVO
        struct {
            char* variable;
            struct NodoAST* contenido;
        } escribirarchivo;
        
        // LEERARCHIVO
        struct {
            char* variable_archivo;
            char* variable_destino;
        } leerarchivo;
        
        // USARARCHIVO
        struct {
            char* variable;
            char* ruta;
            int modo;
        } usararchivo;
        
        // SISTEMA
        struct {
            char* comando;
        } sistema;

        // DIBUJARLINEA
        struct
        {
            struct NodoAST *x1;
            struct NodoAST *y1;
            struct NodoAST *x2;
            struct NodoAST *y2;
            struct NodoAST *patron;
        } dibujarlinea;

        // DIBUJARCIRCULO
        struct
        {
            struct NodoAST *centro_x;
            struct NodoAST *centro_y;
            struct NodoAST *radio;
            struct NodoAST *patron;
        } dibujarcirculo;
        
        // RELLENARRECTANGULO
        struct {
            struct NodoAST* x1;
            struct NodoAST* y1;
            struct NodoAST* x2;
            struct NodoAST* y2;
            struct NodoAST *patron;
        } rellenarrectangulo;
        
        // LEERTECLA
        struct {
            char* variable;
        } leertecla;
               
        // COLISIONRECTANGULOS
        struct {
            struct NodoAST* x1;
            struct NodoAST* y1;
            struct NodoAST* ancho1;
            struct NodoAST* alto1;
            struct NodoAST* x2;
            struct NodoAST* y2;
            struct NodoAST* ancho2;
            struct NodoAST* alto2;
            char* variable_resultado;
        } colisionrectangulos;

        // Comandos GPIO
        struct
        {
            struct NodoAST *pin;
            int direccion; // 0=ENTRADA, 1=SALIDA
            int bias;      // 0=ninguno, 1=PULLUP, 2=PULLDOWN
        } configurarpin;

        struct
        {
            struct NodoAST *pin;
            int valor; // 0=NO, 1=SI
        } estadopin;

        struct
        {
            struct NodoAST *pin;
            char *variable_destino; // NULL si no hay variable
        } leerpin;

        // PWM
        struct {
            struct NodoAST *pin;
            struct NodoAST *frecuencia;
            struct NodoAST *duty_cycle;
        } pwm;

        // DETENERPWM
        struct {
            struct NodoAST *pin;
        } detenerpwm;

        // CONECTARBD
        struct {
            char* variable;
            char* ruta;
        } conectarbd;
        
        // CERRARBD
        struct {
            char* variable;
        } cerrarbd;
        
        // EJECUTARBD
        struct {
            char* variable;
            char* consulta;
        } ejecutarbd;
        
        // CONSULTARBD
        struct {
            char* variable;
            char* consulta;
        } consultarbd;
        
        // CERRARCONSULTABD
        struct {
            char* variable;
        } cerrarconsultabd;
        
        // INICIARTRANSACCION
        struct {
            char* variable;
        } iniciartransaccion;
        
        // CONFIRMARTRANSACCION
        struct {
            char* variable;
        } confirmartransaccion;
        
        // DESHACERTRANSACCION
        struct {
            char* variable;
        } deshacertransaccion;
        
        // INICIARSERVER
        struct {
            int puerto;
        } iniciarserver;
        
        // CERRARSERVER
        struct {
            int dummy;
        } cerrarserver;
        
        // ETIQUETA
        struct {
            char* nombre;
        } etiqueta;
        
        // SALTAR A
        struct {
            char* nombre_etiqueta;
        } saltar_a;
        
        // INCLUIR
        struct {
            char* ruta_archivo;
            struct NodoAST* contenido;
        } incluir;

        // ALERTA
        struct
        {
            struct NodoAST *mensaje;
        } alerta;
        // INTENTAR/ATRAPAR
        struct
        {
            struct NodoAST *bloque_intent;
            struct NodoAST *bloque_atrapar;
        } intentatrapar;

        // Bloque
        struct {
            struct NodoAST* primera;
            struct NodoAST* siguiente;
        } bloque;
        
        // Funciones de texto
        struct {
            char* destino;
            struct NodoAST* origen;
        } copiartexto;
        
        struct {
            char* destino;
            struct NodoAST* texto;
        } concatenartexto;
        
        struct {
            char* variable;
        } mayusculas;
        
        struct {
            char* variable;
        } minusculas;
        
        struct {
            char* variable;
        } recortartexto;
        
        struct {
            char* variable;
            struct NodoAST* buscar;
            struct NodoAST* reemplazar;
        } reemplazartexto;
        
        struct {
            struct NodoAST* texto;
            struct NodoAST* veces;
            char* destino;
        } repetirtexto;
        
        struct {
            struct NodoAST* texto;
            struct NodoAST* inicio;
            struct NodoAST* longitud;
            char* destino;
        } extraertexto;
        
        struct {
            struct NodoAST* texto;
            char* separador;
            struct NodoAST* indice;
            char* destino;
        } dividirtexto;
        
        struct {
            struct NodoAST* numero;
            char* destino;
        } enteroatexto;
        
        struct {
            struct NodoAST* numero;
            char* destino;
        } decimalatexto;
        
        struct {
            struct NodoAST* caracter;
            char* destino;
        } caracteratexto;
        
    } datos;
    
    // Punteros para listas enlazadas
    struct NodoAST* siguiente;
    struct NodoAST* anterior;
    
} NodoAST;

// Funciones de creación de nodos
NodoAST* crear_nodo_programa(const char* nombre, int linea);
NodoAST* crear_nodo_bloque_principal(int linea);
NodoAST* crear_nodo_declaracion_variable(const char* nombre, TipoDato tipo, NodoAST* valor_inicial, int linea);
NodoAST* crear_nodo_declaracion_constante(const char* nombre, TipoDato tipo, NodoAST* valor, int linea);
NodoAST* crear_nodo_declaracion_lista(const char* nombre, TipoDato tipo_elemento, int tamano, NodoAST* valores_iniciales, int linea);
NodoAST* crear_nodo_declaracion_matriz(const char* nombre, TipoDato tipo_elemento, int filas, int columnas, NodoAST* valores_iniciales, int linea);
NodoAST *crear_nodo_declaracion_matriz3d(const char *nombre, TipoDato tipo_elemento, int dim1, int dim2, int dim3, NodoAST *valores_iniciales, int linea);
NodoAST* crear_nodo_declaracion_texto_extenso(const char* nombre, const char* valor_inicial, int linea);
NodoAST* crear_nodo_declaracion_archivo(const char* nombre, const char* ruta, int modo, int linea);
NodoAST* crear_nodo_si(NodoAST* condicion, NodoAST* bloque_si, NodoAST* sino_si, NodoAST* bloque_sino, int linea);
NodoAST* crear_nodo_sino_si(NodoAST* condicion, NodoAST* bloque, NodoAST* siguiente_sino_si, int linea);
NodoAST* crear_nodo_sino(NodoAST* bloque, int linea);
NodoAST* crear_nodo_para(const char* variable, NodoAST* inicio, NodoAST* fin, NodoAST* paso, NodoAST* bloque, int linea);
NodoAST* crear_nodo_mientras(NodoAST* condicion, NodoAST* bloque, int linea);
NodoAST* crear_nodo_realizar(NodoAST* bloque, NodoAST* condicion, int linea);
NodoAST* crear_nodo_segun_caso(NodoAST* expresion, NodoAST* casos, int linea);
NodoAST* crear_nodo_caso(int valor, NodoAST* bloque, int linea);
NodoAST* crear_nodo_por_defecto(NodoAST* bloque, int linea);
NodoAST* crear_nodo_corte(int linea);
NodoAST* crear_nodo_funcion(const char* nombre, TipoDato tipo_retorno, char** parametros, int num_parametros, NodoAST* bloque, int linea);
NodoAST *crear_nodo_llamada_funcion_modificadora(char *nombre, NodoAST *argumentos, int linea);
NodoAST* crear_nodo_subprograma(const char* nombre, char** parametros, int num_parametros, NodoAST* bloque, int linea);
NodoAST* crear_nodo_llamar_a(const char* nombre, NodoAST* argumentos, int linea);
NodoAST* crear_nodo_retornar(NodoAST* valor, int linea);
NodoAST *crear_nodo_literal_numero(const char *valor_str, double valor, int linea);
NodoAST* crear_nodo_literal_texto(const char* valor, int linea);
NodoAST *crear_nodo_literal_caracter(const char *valor, int linea);
NodoAST* crear_nodo_literal_logico(bool valor, int linea);
NodoAST* crear_nodo_variable(const char* nombre, int linea);
NodoAST* crear_nodo_acceso_lista(const char* nombre_lista, NodoAST* indice, int linea);
NodoAST* crear_nodo_acceso_matriz(const char* nombre_matriz, NodoAST* fila, NodoAST* columna, int linea);
NodoAST *crear_nodo_acceso_matriz3d(const char *nombre_matriz, NodoAST *indice1, NodoAST *indice2, NodoAST *indice3, int linea);
NodoAST* crear_nodo_operador_binario(OperadorBinario operador, NodoAST* izquierdo, NodoAST* derecho, int linea);
NodoAST* crear_nodo_operador_unario(OperadorUnario operador, NodoAST* operando, int linea);
NodoAST* crear_nodo_llamada_funcion(const char* nombre_funcion, NodoAST* argumentos, int linea);
NodoAST* crear_nodo_escribir(NodoAST* expresion, bool salto_linea, int linea);
NodoAST *crear_nodo_incluir(const char *ruta_archivo, int linea);
NodoAST* crear_nodo_leer(const char* variable, int linea);
NodoAST* crear_nodo_leercaracter(const char* variable, int linea);
NodoAST *crear_nodo_leerhasta(const char *variable, NodoAST *delimitador, int linea);
NodoAST* crear_nodo_limpiarpantalla(int linea);
NodoAST* crear_nodo_colortexto(const char* color, int linea);
NodoAST* crear_nodo_colorfondo(const char* color, int linea);
NodoAST* crear_nodo_textonegrita(int linea);
NodoAST* crear_nodo_textocursiva(int linea);
NodoAST* crear_nodo_textosubrayado(int linea);
NodoAST* crear_nodo_resettexto(int linea);
NodoAST* crear_nodo_resetcolor(int linea);
NodoAST* crear_nodo_cursor(NodoAST* fila, NodoAST* columna, int linea);
NodoAST* crear_nodo_posicionar(NodoAST* fila, NodoAST* columna, int linea);
NodoAST* crear_nodo_ocultarcursor(int linea);
NodoAST* crear_nodo_mostrarcursor(int linea);
NodoAST* crear_nodo_anchoterminal(const char* variable, int linea);
NodoAST* crear_nodo_altoterminal(const char* variable, int linea);
NodoAST* crear_nodo_esperar(NodoAST* cantidad, const char* unidad, int linea);
NodoAST* crear_nodo_tiempoms(const char* variable, int linea);
NodoAST* crear_nodo_horaactual(const char* variable, int linea);
NodoAST* crear_nodo_fechaactual(const char* variable, int linea);
NodoAST* crear_nodo_resultado(NodoAST* expresion, int linea);
NodoAST* crear_nodo_abrirarchivo(const char* variable, const char* ruta, int modo, int linea);
NodoAST* crear_nodo_cerrararchivo(const char* variable, int linea);
NodoAST* crear_nodo_escribirarchivo(const char* variable, NodoAST* contenido, int linea);
NodoAST* crear_nodo_leerarchivo(const char* variable_archivo, const char* variable_destino, int linea);
NodoAST* crear_nodo_usararchivo(const char* variable, const char* ruta, int modo, int linea);
NodoAST* crear_nodo_sistema(const char* comando, int linea);
NodoAST *crear_nodo_dibujarlinea(NodoAST *x1, NodoAST *y1, NodoAST *x2, NodoAST *y2, NodoAST *patron, int linea);
NodoAST *crear_nodo_dibujarcirculo(NodoAST *centro_x, NodoAST *centro_y, NodoAST *radio, NodoAST *patron, int linea);
NodoAST *crear_nodo_rellenarrectangulo(NodoAST *x1, NodoAST *y1, NodoAST *x2, NodoAST *y2, NodoAST *patron, int linea);
NodoAST* crear_nodo_leertecla(const char* variable, int linea);
NodoAST* crear_nodo_colisionrectangulos(NodoAST* x1, NodoAST* y1, NodoAST* ancho1, NodoAST* alto1, NodoAST* x2, NodoAST* y2, NodoAST* ancho2, NodoAST* alto2, const char* variable_resultado, int linea);
NodoAST *crear_nodo_configurarpin(NodoAST *pin, int direccion, int bias, int linea);
NodoAST *crear_nodo_estadopin(NodoAST *pin, int valor, int linea);
NodoAST *crear_nodo_leerpin(NodoAST *pin, const char *variable_destino, int linea);
NodoAST* crear_nodo_pwm(NodoAST *pin, NodoAST *frecuencia, NodoAST *duty_cycle, int linea);
NodoAST* crear_nodo_detenerpwm(NodoAST *pin, int linea);
NodoAST* crear_nodo_enviarrf(NodoAST *pin, NodoAST *codigo, NodoAST *repeticiones, int linea);
NodoAST* crear_nodo_leerrf(NodoAST *pin, NodoAST *timeout, const char *variable_destino, int linea);
NodoAST* crear_nodo_conectarbd(const char* variable, const char* ruta, int linea);
NodoAST* crear_nodo_cerrarbd(const char* variable, int linea);
NodoAST* crear_nodo_ejecutarbd(const char* variable, const char* consulta, int linea);
NodoAST* crear_nodo_consultarbd(const char* variable, const char* consulta, int linea);
NodoAST* crear_nodo_cerrarconsultabd(const char* variable, int linea);
NodoAST* crear_nodo_iniciartransaccion(const char* variable, int linea);
NodoAST* crear_nodo_confirmartransaccion(const char* variable, int linea);
NodoAST* crear_nodo_deshacertransaccion(const char* variable, int linea);
NodoAST* crear_nodo_iniciarserver(int puerto, int linea);
NodoAST* crear_nodo_cerrarserver(int linea);
NodoAST* crear_nodo_etiqueta(const char* nombre, int linea);
NodoAST* crear_nodo_saltar_a(const char* nombre_etiqueta, int linea);
NodoAST* crear_nodo_incluir(const char* ruta_archivo, int linea);
NodoAST *crear_nodo_alerta(NodoAST *mensaje, int linea);
NodoAST *crear_nodo_intentatrapar(NodoAST *bloque_intent, NodoAST *bloque_atrapar, int linea);
NodoAST* crear_nodo_bloque(int linea);
NodoAST *crear_nodo_calcular_nodo(NodoAST *destino, NodoAST *expresion, int linea);
NodoAST *crear_nodo_asignar_nodo(NodoAST *destino, NodoAST *valor, int linea);

// Funciones de manipulación de bloques
void agregar_sentencia_a_bloque(NodoAST* bloque, NodoAST* sentencia);
void agregar_caso_a_segun(NodoAST* segun, NodoAST* caso);
void agregar_argumento_a_llamada(NodoAST* llamada, NodoAST* argumento);

// Funciones de liberación de memoria
void liberar_nodo(NodoAST* nodo);
void liberar_ast(NodoAST* raiz);

// Funciones de utilidad
const char* nombre_tipo_nodo(TipoNodo tipo);
const char* nombre_tipo_dato(TipoDato tipo);
const char* nombre_operador_binario(OperadorBinario op);
const char* nombre_operador_unario(OperadorUnario op);
void nodo_escribir_set_formatos(NodoAST *nodo, int *formatos, int num_formatos);
// Funciones de depuración
void imprimir_ast(NodoAST* nodo, int nivel);
void imprimir_ast_detalle(NodoAST* nodo, int nivel);

#endif // AST_H
