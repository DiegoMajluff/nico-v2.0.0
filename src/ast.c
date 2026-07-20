/*
 * Nico v2.0.0 - Intérprete Educativo de Scripting en Español
 * @file:         ast.c
 * @author:       Diego Alejandro Majluff (Diseño, Arquitectura y Supervisión)
 * @ai_assist:    Qwen (Alibaba Cloud) - Implementación, Debugging y Optimización
 * @license:      MIT / Personal Use (ver LICENSE)
 * @description:  Implementación de funciones para crear, manipular y liberar
 *                nodos del AST. Incluye constructores para cada tipo de nodo
 *                (literales, variables, operaciones, control de flujo, etc.)
 *                y gestión de memoria para evitar fugas.
 */
#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Función auxiliar para duplicar strings
static char* duplicar_string(const char* str) {
    if (!str) return NULL;
    char* dup = malloc(strlen(str) + 1);
    if (dup) strcpy(dup, str);
    return dup;
}

// Función auxiliar para crear un nodo genérico
static NodoAST* crear_nodo_base(TipoNodo tipo, int linea) {
    NodoAST* nodo = calloc(1, sizeof(NodoAST));
    if (!nodo) {
        fprintf(stderr, "Error: Memoria insuficiente para crear nodo AST\n");
        exit(1);
    }
    nodo->tipo = tipo;
    nodo->linea = linea;
    nodo->columna = 0;
    nodo->siguiente = NULL;
    nodo->anterior = NULL;
    return nodo;
}

void nodo_escribir_set_formatos(NodoAST *nodo, int *formatos, int num_formatos)
{
    if (nodo && nodo->tipo == AST_ESCRIBIR && formatos && num_formatos > 0)
    {
        nodo->datos.escribir.formatos_decimales = malloc(sizeof(int) * num_formatos);
        for (int i = 0; i < num_formatos; i++)
        {
            nodo->datos.escribir.formatos_decimales[i] = formatos[i];
        }
        nodo->datos.escribir.num_formatos = num_formatos;
    }
}

// Implementación de funciones de creación de nodos

NodoAST* crear_nodo_programa(const char* nombre, int linea) {
    NodoAST* nodo = crear_nodo_base(AST_PROGRAMA, linea);
    nodo->datos.programa.nombre = duplicar_string(nombre);
    nodo->datos.programa.declaraciones = NULL;
    nodo->datos.programa.bloque_principal = NULL;
    nodo->datos.programa.funciones = NULL;
    nodo->datos.programa.subprogramas = NULL;
    return nodo;
}

NodoAST* crear_nodo_bloque_principal(int linea) {
    NodoAST* nodo = crear_nodo_base(AST_BLOQUE_PRINCIPAL, linea);
    nodo->datos.bloque_principal.sentencias = NULL;
    return nodo;
}

NodoAST* crear_nodo_declaracion_variable(const char* nombre, TipoDato tipo, NodoAST* valor_inicial, int linea) {
    NodoAST* nodo = crear_nodo_base(AST_DECLARACION_VARIABLE, linea);
    nodo->datos.declaracion_variable.nombre = duplicar_string(nombre);
    nodo->datos.declaracion_variable.tipo_dato = tipo;
    nodo->datos.declaracion_variable.valor_inicial = valor_inicial;
    return nodo;
}

NodoAST* crear_nodo_declaracion_constante(const char* nombre, TipoDato tipo, NodoAST* valor, int linea) {
    NodoAST* nodo = crear_nodo_base(AST_DECLARACION_CONSTANTE, linea);
    nodo->datos.declaracion_constante.nombre = duplicar_string(nombre);
    nodo->datos.declaracion_constante.tipo_dato = tipo;
    nodo->datos.declaracion_constante.valor = valor;
    return nodo;
}

NodoAST* crear_nodo_declaracion_lista(const char* nombre, TipoDato tipo_elemento, int tamano, NodoAST* valores_iniciales, int linea) {
    NodoAST* nodo = crear_nodo_base(AST_DECLARACION_LISTA, linea);
    nodo->datos.declaracion_lista.nombre = duplicar_string(nombre);
    nodo->datos.declaracion_lista.tipo_elemento = tipo_elemento;
    nodo->datos.declaracion_lista.tamano = tamano;
    nodo->datos.declaracion_lista.valores_iniciales = valores_iniciales;
    return nodo;
}

NodoAST* crear_nodo_declaracion_matriz(const char* nombre, TipoDato tipo_elemento, int filas, int columnas, NodoAST* valores_iniciales, int linea) {
    NodoAST* nodo = crear_nodo_base(AST_DECLARACION_MATRIZ, linea);
    nodo->datos.declaracion_matriz.nombre = duplicar_string(nombre);
    nodo->datos.declaracion_matriz.tipo_elemento = tipo_elemento;
    nodo->datos.declaracion_matriz.filas = filas;
    nodo->datos.declaracion_matriz.columnas = columnas;
    nodo->datos.declaracion_matriz.valores_iniciales = valores_iniciales;
    return nodo;
}

NodoAST *crear_nodo_declaracion_matriz3d(const char *nombre, TipoDato tipo_elemento, int dim1, int dim2, int dim3, NodoAST *valores_iniciales, int linea)
{
    NodoAST *nodo = crear_nodo_base(AST_DECLARACION_MATRIZ3D, linea);
    nodo->datos.declaracion_matriz3d.nombre = duplicar_string(nombre);
    nodo->datos.declaracion_matriz3d.tipo_elemento = tipo_elemento;
    nodo->datos.declaracion_matriz3d.dim1 = dim1;
    nodo->datos.declaracion_matriz3d.dim2 = dim2;
    nodo->datos.declaracion_matriz3d.dim3 = dim3;
    nodo->datos.declaracion_matriz3d.valores_iniciales = valores_iniciales;
    return nodo;
}

NodoAST* crear_nodo_declaracion_texto_extenso(const char* nombre, const char* valor_inicial, int linea) {
    NodoAST* nodo = crear_nodo_base(AST_DECLARACION_TEXTO_EXTENSO, linea);
    nodo->datos.declaracion_texto_extenso.nombre = duplicar_string(nombre);
    nodo->datos.declaracion_texto_extenso.valor_inicial = duplicar_string(valor_inicial);
    return nodo;
}

NodoAST* crear_nodo_declaracion_archivo(const char* nombre, const char* ruta, int modo, int linea) {
    NodoAST* nodo = crear_nodo_base(AST_DECLARACION_ARCHIVO, linea);
    nodo->datos.declaracion_archivo.nombre = duplicar_string(nombre);
    nodo->datos.declaracion_archivo.ruta = duplicar_string(ruta);
    nodo->datos.declaracion_archivo.modo = modo;
    return nodo;
}

NodoAST* crear_nodo_si(NodoAST* condicion, NodoAST* bloque_si, NodoAST* sino_si, NodoAST* bloque_sino, int linea) {
    NodoAST* nodo = crear_nodo_base(AST_SI, linea);
    nodo->datos.si.condicion = condicion;
    nodo->datos.si.bloque_si = bloque_si;
    nodo->datos.si.sino_si = sino_si;
    nodo->datos.si.bloque_sino = bloque_sino;
    return nodo;
}

NodoAST* crear_nodo_sino_si(NodoAST* condicion, NodoAST* bloque, NodoAST* siguiente_sino_si, int linea) {
    NodoAST* nodo = crear_nodo_base(AST_SINO_SI, linea);
    nodo->datos.sino_si.condicion = condicion;
    nodo->datos.sino_si.bloque = bloque;
    nodo->datos.sino_si.siguiente_sino_si = siguiente_sino_si;
    return nodo;
}

NodoAST* crear_nodo_sino(NodoAST* bloque, int linea) {
    NodoAST* nodo = crear_nodo_base(AST_SINO, linea);
    nodo->datos.sino.bloque = bloque;
    return nodo;
}

NodoAST* crear_nodo_para(const char* variable, NodoAST* inicio, NodoAST* fin, NodoAST* paso, NodoAST* bloque, int linea) {
    NodoAST* nodo = crear_nodo_base(AST_PARA, linea);
    nodo->datos.para.variable = duplicar_string(variable);
    nodo->datos.para.inicio = inicio;
    nodo->datos.para.fin = fin;
    nodo->datos.para.paso = paso;
    nodo->datos.para.bloque = bloque;
    return nodo;
}

NodoAST* crear_nodo_mientras(NodoAST* condicion, NodoAST* bloque, int linea) {
    NodoAST* nodo = crear_nodo_base(AST_MIENTRAS, linea);
    nodo->datos.mientras.condicion = condicion;
    nodo->datos.mientras.bloque = bloque;
    return nodo;
}

NodoAST* crear_nodo_realizar(NodoAST* bloque, NodoAST* condicion, int linea) {
    NodoAST* nodo = crear_nodo_base(AST_REALIZAR, linea);
    nodo->datos.realizar.bloque = bloque;
    nodo->datos.realizar.condicion = condicion;
    return nodo;
}

NodoAST* crear_nodo_segun_caso(NodoAST* expresion, NodoAST* casos, int linea) {
    NodoAST* nodo = crear_nodo_base(AST_SEGUN_CASO, linea);
    nodo->datos.segun_caso.expresion = expresion;
    nodo->datos.segun_caso.casos = casos;
    return nodo;
}

NodoAST* crear_nodo_caso(int valor, NodoAST* bloque, int linea) {
    NodoAST* nodo = crear_nodo_base(AST_CASO, linea);
    nodo->datos.caso.valor = valor;
    nodo->datos.caso.bloque = bloque;
    return nodo;
}

NodoAST* crear_nodo_por_defecto(NodoAST* bloque, int linea) {
    NodoAST* nodo = crear_nodo_base(AST_POR_DEFECTO, linea);
    nodo->datos.por_defecto.bloque = bloque;
    return nodo;
}

NodoAST* crear_nodo_corte(int linea) {
    return crear_nodo_base(AST_CORTE, linea);
}

NodoAST* crear_nodo_funcion(const char* nombre, TipoDato tipo_retorno, char** parametros, int num_parametros, NodoAST* bloque, int linea) {
    NodoAST* nodo = crear_nodo_base(AST_FUNCION, linea);
    nodo->datos.funcion.nombre = duplicar_string(nombre);
    nodo->datos.funcion.tipo_retorno = tipo_retorno;
    nodo->datos.funcion.num_parametros = num_parametros;
    if (num_parametros > 0 && parametros) {
        nodo->datos.funcion.parametros = malloc(num_parametros * sizeof(char*));
        for (int i = 0; i < num_parametros; i++) {
            nodo->datos.funcion.parametros[i] = duplicar_string(parametros[i]);
        }
    } else {
        nodo->datos.funcion.parametros = NULL;
    }
    nodo->datos.funcion.bloque = bloque;
    return nodo;
}

NodoAST* crear_nodo_subprograma(const char* nombre, char** parametros, int num_parametros, NodoAST* bloque, int linea) {
    NodoAST* nodo = crear_nodo_base(AST_SUBPROGRAMA, linea);
    nodo->datos.subprograma.nombre = duplicar_string(nombre);
    nodo->datos.subprograma.num_parametros = num_parametros;
    if (num_parametros > 0 && parametros) {
        nodo->datos.subprograma.parametros = malloc(num_parametros * sizeof(char*));
        for (int i = 0; i < num_parametros; i++) {
            nodo->datos.subprograma.parametros[i] = duplicar_string(parametros[i]);
        }
    } else {
        nodo->datos.subprograma.parametros = NULL;
    }
    nodo->datos.subprograma.bloque = bloque;
    return nodo;
}

NodoAST* crear_nodo_llamar_a(const char* nombre, NodoAST* argumentos, int linea) {
    NodoAST* nodo = crear_nodo_base(AST_LLAMAR_A, linea);
    nodo->datos.llamar_a.nombre = duplicar_string(nombre);
    nodo->datos.llamar_a.argumentos = argumentos;
    return nodo;
}

NodoAST* crear_nodo_retornar(NodoAST* valor, int linea) {
    NodoAST* nodo = crear_nodo_base(AST_RETORNAR, linea);
    nodo->datos.retornar.valor = valor;
    return nodo;
}

NodoAST *crear_nodo_literal_numero(const char *valor_str, double valor, int linea)
{
    NodoAST *nodo = crear_nodo_base(AST_LITERAL_NUMERO, linea);
    nodo->datos.literal_numero.valor_str = valor_str ? strdup(valor_str) : NULL;
    nodo->datos.literal_numero.valor = valor;
    return nodo;
}

NodoAST* crear_nodo_literal_texto(const char* valor, int linea) {
    NodoAST* nodo = crear_nodo_base(AST_LITERAL_TEXTO, linea);
    nodo->datos.literal_texto.valor = duplicar_string(valor);
    return nodo;
}

NodoAST *crear_nodo_literal_caracter(const char *valor, int linea)
{
    NodoAST *nodo = crear_nodo_base(AST_LITERAL_CARACTER, linea);
    strncpy(nodo->datos.literal_caracter.valor, valor, 4);
    nodo->datos.literal_caracter.valor[3] = '\0';
    return nodo;
}

NodoAST* crear_nodo_literal_logico(bool valor, int linea) {
    NodoAST* nodo = crear_nodo_base(AST_LITERAL_LOGICO, linea);
    nodo->datos.literal_logico.valor = valor;
    return nodo;
}

NodoAST* crear_nodo_variable(const char* nombre, int linea) {
    NodoAST* nodo = crear_nodo_base(AST_VARIABLE, linea);
    nodo->datos.variable.nombre = duplicar_string(nombre);
    return nodo;
}

NodoAST* crear_nodo_acceso_lista(const char* nombre_lista, NodoAST* indice, int linea) {
    NodoAST* nodo = crear_nodo_base(AST_ACCESO_LISTA, linea);
    nodo->datos.acceso_lista.nombre_lista = duplicar_string(nombre_lista);
    nodo->datos.acceso_lista.indice = indice;
    return nodo;
}

NodoAST* crear_nodo_acceso_matriz(const char* nombre_matriz, NodoAST* fila, NodoAST* columna, int linea) {
    NodoAST* nodo = crear_nodo_base(AST_ACCESO_MATRIZ, linea);
    nodo->datos.acceso_matriz.nombre_matriz = duplicar_string(nombre_matriz);
    nodo->datos.acceso_matriz.fila = fila;
    nodo->datos.acceso_matriz.columna = columna;
    return nodo;
}

NodoAST *crear_nodo_acceso_matriz3d(const char *nombre_matriz, NodoAST *indice1, NodoAST *indice2, NodoAST *indice3, int linea)
{
    NodoAST *nodo = crear_nodo_base(AST_ACCESO_MATRIZ3D, linea);
    nodo->datos.acceso_matriz3d.nombre_matriz = duplicar_string(nombre_matriz);
    nodo->datos.acceso_matriz3d.indice1 = indice1;
    nodo->datos.acceso_matriz3d.indice2 = indice2;
    nodo->datos.acceso_matriz3d.indice3 = indice3;
    return nodo;
}

NodoAST* crear_nodo_operador_binario(OperadorBinario operador, NodoAST* izquierdo, NodoAST* derecho, int linea) {
    NodoAST* nodo = crear_nodo_base(AST_OPERADOR_BINARIO, linea);
    nodo->datos.operador_binario.operador = operador;
    nodo->datos.operador_binario.izquierdo = izquierdo;
    nodo->datos.operador_binario.derecho = derecho;
    return nodo;
}

NodoAST* crear_nodo_operador_unario(OperadorUnario operador, NodoAST* operando, int linea) {
    NodoAST* nodo = crear_nodo_base(AST_OPERADOR_UNARIO, linea);
    nodo->datos.operador_unario.operador = operador;
    nodo->datos.operador_unario.operando = operando;
    return nodo;
}

NodoAST* crear_nodo_llamada_funcion(const char* nombre_funcion, NodoAST* argumentos, int linea) {
    NodoAST* nodo = crear_nodo_base(AST_LLAMADA_FUNCION, linea);
    nodo->datos.llamada_funcion.nombre_funcion = duplicar_string(nombre_funcion);
    nodo->datos.llamada_funcion.argumentos = argumentos;
    return nodo;
}

NodoAST *crear_nodo_llamada_funcion_modificadora(char *nombre, NodoAST *argumentos, int linea)
{
    NodoAST *nodo = crear_nodo_base(AST_LLAMADA_FUNCION_MODIFICADORA, linea);
    nodo->datos.llamada_funcion_modificadora.nombre_funcion = nombre;
    nodo->datos.llamada_funcion_modificadora.argumentos = argumentos;
    return nodo;
}

// Comandos de E/S
NodoAST *crear_nodo_escribir(NodoAST *expresion, bool salto_linea, int linea)
{
    NodoAST *nodo = crear_nodo_base(AST_ESCRIBIR, linea);
    nodo->datos.escribir.expresion = expresion;
    nodo->datos.escribir.salto_linea = salto_linea;
    nodo->datos.escribir.formatos_decimales = NULL;
    nodo->datos.escribir.num_formatos = 0;
    return nodo;
}

NodoAST* crear_nodo_leer(const char* variable, int linea) {
    NodoAST* nodo = crear_nodo_base(AST_LEER, linea);
    nodo->datos.leer.variable = duplicar_string(variable);
    return nodo;
}

NodoAST* crear_nodo_leercaracter(const char* variable, int linea) {
    NodoAST* nodo = crear_nodo_base(AST_LEERCARACTER, linea);
    nodo->datos.leercaracter.variable = duplicar_string(variable);
    return nodo;
}

NodoAST *crear_nodo_leerhasta(const char *variable, NodoAST *delimitador, int linea)
{
    NodoAST *nodo = crear_nodo_base(AST_LEERHASTA, linea);
    nodo->datos.leerhasta.variable = strdup(variable);
    nodo->datos.leerhasta.delimitador = delimitador;
    return nodo;
}

NodoAST* crear_nodo_limpiarpantalla(int linea) {
    return crear_nodo_base(AST_LIMPIARPANTALLA, linea);
}

// Comandos de formato
NodoAST* crear_nodo_colortexto(const char* color, int linea) {
    NodoAST* nodo = crear_nodo_base(AST_COLORTEXTO, linea);
    nodo->datos.colortexto.color = duplicar_string(color);
    return nodo;
}

NodoAST* crear_nodo_colorfondo(const char* color, int linea) {
    NodoAST* nodo = crear_nodo_base(AST_COLORFONDO, linea);
    nodo->datos.colorfondo.color = duplicar_string(color);
    return nodo;
}

NodoAST* crear_nodo_textonegrita(int linea) {
    return crear_nodo_base(AST_TEXTONEGRITA, linea);
}

NodoAST* crear_nodo_textocursiva(int linea) {
    return crear_nodo_base(AST_TEXTOCURSIVA, linea);
}

NodoAST* crear_nodo_textosubrayado(int linea) {
    return crear_nodo_base(AST_TEXTOSUBRAYADO, linea);
}

NodoAST* crear_nodo_resettexto(int linea) {
    return crear_nodo_base(AST_RESETTEXTO, linea);
}

NodoAST* crear_nodo_resetcolor(int linea) {
    return crear_nodo_base(AST_RESETCOLOR, linea);
}

// Comandos de cursor y terminal
NodoAST* crear_nodo_cursor(NodoAST* fila, NodoAST* columna, int linea) {
    NodoAST* nodo = crear_nodo_base(AST_CURSOR, linea);
    nodo->datos.cursor.fila = fila;
    nodo->datos.cursor.columna = columna;
    return nodo;
}

NodoAST* crear_nodo_posicionar(NodoAST* fila, NodoAST* columna, int linea) {
    NodoAST* nodo = crear_nodo_base(AST_POSICIONAR, linea);
    nodo->datos.posicionar.fila = fila;
    nodo->datos.posicionar.columna = columna;
    return nodo;
}

NodoAST* crear_nodo_ocultarcursor(int linea) {
    return crear_nodo_base(AST_OCULTARCURSOR, linea);
}

NodoAST* crear_nodo_mostrarcursor(int linea) {
    return crear_nodo_base(AST_MOSTRARCURSOR, linea);
}

NodoAST* crear_nodo_anchoterminal(const char* variable, int linea) {
    NodoAST* nodo = crear_nodo_base(AST_ANCHOTERMINAL, linea);
    nodo->datos.anchoterminal.variable = duplicar_string(variable);
    return nodo;
}

NodoAST* crear_nodo_altoterminal(const char* variable, int linea) {
    NodoAST* nodo = crear_nodo_base(AST_ALTOTERMINAL, linea);
    nodo->datos.altoterminal.variable = duplicar_string(variable);
    return nodo;
}

// Comandos de tiempo
NodoAST* crear_nodo_esperar(NodoAST* cantidad, const char* unidad, int linea) {
    NodoAST* nodo = crear_nodo_base(AST_ESPERAR, linea);
    nodo->datos.esperar.cantidad = cantidad;
    nodo->datos.esperar.unidad = duplicar_string(unidad);
    return nodo;
}

NodoAST* crear_nodo_tiempoms(const char* variable, int linea) {
    NodoAST* nodo = crear_nodo_base(AST_TIEMPOMS, linea);
    nodo->datos.tiempoms.variable = duplicar_string(variable);
    return nodo;
}

NodoAST* crear_nodo_horaactual(const char* variable, int linea) {
    NodoAST* nodo = crear_nodo_base(AST_HORAACTUAL, linea);
    nodo->datos.horaactual.variable = duplicar_string(variable);
    return nodo;
}

NodoAST* crear_nodo_fechaactual(const char* variable, int linea) {
    NodoAST* nodo = crear_nodo_base(AST_FECHAACTUAL, linea);
    nodo->datos.fechaactual.variable = duplicar_string(variable);
    return nodo;
}

// Comandos de cálculo
NodoAST *crear_nodo_calcular(const char *variable, NodoAST *expresion, int linea)
{
    NodoAST *nodo = crear_nodo_base(AST_CALCULAR, linea);
    nodo->datos.calcular.destino = crear_nodo_variable(variable, linea);
    nodo->datos.calcular.expresion = expresion;
    return nodo;
}

NodoAST* crear_nodo_resultado(NodoAST* expresion, int linea) {
    NodoAST* nodo = crear_nodo_base(AST_RESULTADO, linea);
    nodo->datos.resultado.expresion = expresion;
    return nodo;
}

NodoAST *crear_nodo_asignar(const char *variable, NodoAST *valor, int linea)
{
    NodoAST *nodo = crear_nodo_base(AST_ASIGNAR, linea);
    nodo->datos.asignar.destino = crear_nodo_variable(variable, linea);
    nodo->datos.asignar.valor = valor;
    return nodo;
}

// Comandos de archivos
NodoAST* crear_nodo_abrirarchivo(const char* variable, const char* ruta, int modo, int linea) {
    NodoAST* nodo = crear_nodo_base(AST_ABRIRARCHIVO, linea);
    nodo->datos.abrirarchivo.variable = duplicar_string(variable);
    nodo->datos.abrirarchivo.ruta = duplicar_string(ruta);
    nodo->datos.abrirarchivo.modo = modo;
    return nodo;
}

NodoAST* crear_nodo_cerrararchivo(const char* variable, int linea) {
    NodoAST* nodo = crear_nodo_base(AST_CERRARARCHIVO, linea);
    nodo->datos.cerrararchivo.variable = duplicar_string(variable);
    return nodo;
}

NodoAST* crear_nodo_escribirarchivo(const char* variable, NodoAST* contenido, int linea) {
    NodoAST* nodo = crear_nodo_base(AST_ESCRIBIRARCHIVO, linea);
    nodo->datos.escribirarchivo.variable = duplicar_string(variable);
    nodo->datos.escribirarchivo.contenido = contenido;
    return nodo;
}

NodoAST* crear_nodo_leerarchivo(const char* variable_archivo, const char* variable_destino, int linea) {
    NodoAST* nodo = crear_nodo_base(AST_LEERARCHIVO, linea);
    nodo->datos.leerarchivo.variable_archivo = duplicar_string(variable_archivo);
    nodo->datos.leerarchivo.variable_destino = duplicar_string(variable_destino);
    return nodo;
}

NodoAST* crear_nodo_usararchivo(const char* variable, const char* ruta, int modo, int linea) {
    NodoAST* nodo = crear_nodo_base(AST_USARARCHIVO, linea);
    nodo->datos.usararchivo.variable = duplicar_string(variable);
    nodo->datos.usararchivo.ruta = duplicar_string(ruta);
    nodo->datos.usararchivo.modo = modo;
    return nodo;
}

// Comandos de sistema
NodoAST* crear_nodo_sistema(const char* comando, int linea) {
    NodoAST* nodo = crear_nodo_base(AST_SISTEMA, linea);
    nodo->datos.sistema.comando = duplicar_string(comando);
    return nodo;
}

// Comandos de dibujo
NodoAST *crear_nodo_dibujarlinea(NodoAST *x1, NodoAST *y1, NodoAST *x2, NodoAST *y2, NodoAST *patron, int linea)
{
    NodoAST *nodo = crear_nodo_base(AST_DIBUJARLINEA, linea);
    nodo->datos.dibujarlinea.x1 = x1;
    nodo->datos.dibujarlinea.y1 = y1;
    nodo->datos.dibujarlinea.x2 = x2;
    nodo->datos.dibujarlinea.y2 = y2;
    nodo->datos.dibujarlinea.patron = patron;
    return nodo;
}

NodoAST *crear_nodo_dibujarcirculo(NodoAST *centro_x, NodoAST *centro_y, NodoAST *radio, NodoAST *patron, int linea)
{
    NodoAST *nodo = crear_nodo_base(AST_DIBUJARCIRCULO, linea);
    nodo->datos.dibujarcirculo.centro_x = centro_x;
    nodo->datos.dibujarcirculo.centro_y = centro_y;
    nodo->datos.dibujarcirculo.radio = radio;
    nodo->datos.dibujarcirculo.patron = patron;
    return nodo;
}

NodoAST *crear_nodo_rellenarrectangulo(NodoAST *x1, NodoAST *y1, NodoAST *x2, NodoAST *y2, NodoAST *patron, int linea)
{
    NodoAST *nodo = crear_nodo_base(AST_RELLENARRECTANGULO, linea);
    nodo->datos.rellenarrectangulo.x1 = x1;
    nodo->datos.rellenarrectangulo.y1 = y1;
    nodo->datos.rellenarrectangulo.x2 = x2;
    nodo->datos.rellenarrectangulo.y2 = y2;
    nodo->datos.rellenarrectangulo.patron = patron;
    return nodo;
}

// Comandos de teclado
NodoAST* crear_nodo_leertecla(const char* variable, int linea) {
    NodoAST* nodo = crear_nodo_base(AST_LEERTECLA, linea);
    nodo->datos.leertecla.variable = duplicar_string(variable);
    return nodo;
}

NodoAST* crear_nodo_colisionrectangulos(NodoAST* x1, NodoAST* y1, NodoAST* ancho1, NodoAST* alto1, 
                                         NodoAST* x2, NodoAST* y2, NodoAST* ancho2, NodoAST* alto2, 
                                         const char* variable_resultado, int linea) {
    NodoAST* nodo = crear_nodo_base(AST_COLISIONRECTANGULOS, linea);
    nodo->datos.colisionrectangulos.x1 = x1;
    nodo->datos.colisionrectangulos.y1 = y1;
    nodo->datos.colisionrectangulos.ancho1 = ancho1;
    nodo->datos.colisionrectangulos.alto1 = alto1;
    nodo->datos.colisionrectangulos.x2 = x2;
    nodo->datos.colisionrectangulos.y2 = y2;
    nodo->datos.colisionrectangulos.ancho2 = ancho2;
    nodo->datos.colisionrectangulos.alto2 = alto2;
    nodo->datos.colisionrectangulos.variable_resultado = duplicar_string(variable_resultado);
    return nodo;
}

// Comandos GPIO
NodoAST *crear_nodo_configurarpin(NodoAST *pin, int direccion, int bias, int linea)
{
    NodoAST *nodo = crear_nodo_base(AST_CONFIGURARPIN, linea);
    nodo->datos.configurarpin.pin = pin;
    nodo->datos.configurarpin.direccion = direccion;
    nodo->datos.configurarpin.bias = bias;
    return nodo;
}

NodoAST *crear_nodo_estadopin(NodoAST *pin, int valor, int linea)
{
    NodoAST *nodo = crear_nodo_base(AST_ESTADOPIN, linea);
    nodo->datos.estadopin.pin = pin;
    nodo->datos.estadopin.valor = valor;
    return nodo;
}

NodoAST *crear_nodo_leerpin(NodoAST *pin, const char *variable_destino, int linea)
{
    NodoAST *nodo = crear_nodo_base(AST_LEERPIN, linea);
    nodo->datos.leerpin.pin = pin;
    nodo->datos.leerpin.variable_destino = variable_destino ? strdup(variable_destino) : NULL;
    return nodo;
}

NodoAST* crear_nodo_pwm(NodoAST *pin, NodoAST *frecuencia, NodoAST *duty_cycle, int linea) {
    NodoAST* nodo = crear_nodo_base(AST_PWM, linea);
    nodo->datos.pwm.pin = pin;
    nodo->datos.pwm.frecuencia = frecuencia;
    nodo->datos.pwm.duty_cycle = duty_cycle;
    return nodo;
}

NodoAST* crear_nodo_detenerpwm(NodoAST *pin, int linea) {
    NodoAST* nodo = crear_nodo_base(AST_DETENERPWM, linea);
    nodo->datos.detenerpwm.pin = pin;
    return nodo;
}

// Comandos de base de datos
NodoAST* crear_nodo_conectarbd(const char* variable, const char* ruta, int linea) {
    NodoAST* nodo = crear_nodo_base(AST_CONECTARBD, linea);
    nodo->datos.conectarbd.variable = duplicar_string(variable);
    nodo->datos.conectarbd.ruta = duplicar_string(ruta);
    return nodo;
}

NodoAST* crear_nodo_cerrarbd(const char* variable, int linea) {
    NodoAST* nodo = crear_nodo_base(AST_CERRARBD, linea);
    nodo->datos.cerrarbd.variable = duplicar_string(variable);
    return nodo;
}

NodoAST* crear_nodo_ejecutarbd(const char* variable, const char* consulta, int linea) {
    NodoAST* nodo = crear_nodo_base(AST_EJECUTARBD, linea);
    nodo->datos.ejecutarbd.variable = duplicar_string(variable);
    nodo->datos.ejecutarbd.consulta = duplicar_string(consulta);
    return nodo;
}

NodoAST* crear_nodo_consultarbd(const char* variable, const char* consulta, int linea) {
    NodoAST* nodo = crear_nodo_base(AST_CONSULTARBD, linea);
    nodo->datos.consultarbd.variable = duplicar_string(variable);
    nodo->datos.consultarbd.consulta = duplicar_string(consulta);
    return nodo;
}

NodoAST* crear_nodo_cerrarconsultabd(const char* variable, int linea) {
    NodoAST* nodo = crear_nodo_base(AST_CERRARCONSULTABD, linea);
    nodo->datos.cerrarconsultabd.variable = duplicar_string(variable);
    return nodo;
}

NodoAST* crear_nodo_iniciartransaccion(const char* variable, int linea) {
    NodoAST* nodo = crear_nodo_base(AST_INICIARTRANSACCION, linea);
    nodo->datos.iniciartransaccion.variable = duplicar_string(variable);
    return nodo;
}

NodoAST* crear_nodo_confirmartransaccion(const char* variable, int linea) {
    NodoAST* nodo = crear_nodo_base(AST_CONFIRMARTRANSACCION, linea);
    nodo->datos.confirmartransaccion.variable = duplicar_string(variable);
    return nodo;
}

NodoAST* crear_nodo_deshacertransaccion(const char* variable, int linea) {
    NodoAST* nodo = crear_nodo_base(AST_DESHACERTRANSACCION, linea);
    nodo->datos.deshacertransaccion.variable = duplicar_string(variable);
    return nodo;
}

// Comandos de servidor web
NodoAST* crear_nodo_iniciarserver(int puerto, int linea) {
    NodoAST* nodo = crear_nodo_base(AST_INICIARSERVER, linea);
    nodo->datos.iniciarserver.puerto = puerto;
    return nodo;
}

NodoAST* crear_nodo_cerrarserver(int linea) {
    return crear_nodo_base(AST_CERRARSERVER, linea);
}

// Etiquetas y saltos
NodoAST* crear_nodo_etiqueta(const char* nombre, int linea) {
    NodoAST* nodo = crear_nodo_base(AST_ETIQUETA, linea);
    nodo->datos.etiqueta.nombre = duplicar_string(nombre);
    return nodo;
}

NodoAST* crear_nodo_saltar_a(const char* nombre_etiqueta, int linea) {
    NodoAST* nodo = crear_nodo_base(AST_SALTAR_A, linea);
    nodo->datos.saltar_a.nombre_etiqueta = duplicar_string(nombre_etiqueta);
    return nodo;
}

// Inclusión de archivos
NodoAST* crear_nodo_incluir(const char* ruta_archivo, int linea) {
    NodoAST* nodo = crear_nodo_base(AST_INCLUIR, linea);
    nodo->datos.incluir.ruta_archivo = duplicar_string(ruta_archivo);
    nodo->datos.incluir.contenido = NULL;  // Se llena al procesar
    return nodo;
}

// Manejo de errores
NodoAST *crear_nodo_alerta(NodoAST *mensaje, int linea)
{
    NodoAST *nodo = crear_nodo_base(AST_ALERTA, linea);
    nodo->datos.alerta.mensaje = mensaje;
    return nodo;
}

NodoAST *crear_nodo_intentatrapar(NodoAST *bloque_intent, NodoAST *bloque_atrapar, int linea)
{
    NodoAST *nodo = crear_nodo_base(AST_INTENTAR_ATRAPAR, linea);
    nodo->datos.intentatrapar.bloque_intent = bloque_intent;
    nodo->datos.intentatrapar.bloque_atrapar = bloque_atrapar;
    return nodo;
}

// Bloque de código
NodoAST* crear_nodo_bloque(int linea) {
    NodoAST* nodo = crear_nodo_base(AST_BLOQUE, linea);
    nodo->datos.bloque.primera = NULL;
    nodo->datos.bloque.siguiente = NULL;
    return nodo;
}

NodoAST *crear_nodo_calcular_nodo(NodoAST *destino, NodoAST *expresion, int linea)
{
    NodoAST *nodo = crear_nodo_base(AST_CALCULAR, linea);
    nodo->datos.calcular.destino = destino;
    nodo->datos.calcular.expresion = expresion;
    return nodo;
}

NodoAST *crear_nodo_asignar_nodo(NodoAST *destino, NodoAST *valor, int linea)
{
    NodoAST *nodo = crear_nodo_base(AST_ASIGNAR, linea);
    nodo->datos.asignar.destino = destino;
    nodo->datos.asignar.valor = valor;
    return nodo;
}

// Funciones de manipulación de bloques
void agregar_sentencia_a_bloque(NodoAST* bloque, NodoAST* sentencia) {
    if (!bloque || bloque->tipo != AST_BLOQUE || !sentencia) return;
    
    if (!bloque->datos.bloque.primera) {
        bloque->datos.bloque.primera = sentencia;
        sentencia->anterior = NULL;
        sentencia->siguiente = NULL;
    } else {
        NodoAST* actual = bloque->datos.bloque.primera;
        while (actual->siguiente) {
            actual = actual->siguiente;
        }
        actual->siguiente = sentencia;
        sentencia->anterior = actual;
        sentencia->siguiente = NULL;
    }
}

void agregar_caso_a_segun(NodoAST* segun, NodoAST* caso) {
    if (!segun || segun->tipo != AST_SEGUN_CASO || !caso) return;
    
    if (!segun->datos.segun_caso.casos) {
        segun->datos.segun_caso.casos = caso;
    } else {
        NodoAST* actual = segun->datos.segun_caso.casos;
        while (actual->siguiente) {
            actual = actual->siguiente;
        }
        actual->siguiente = caso;
    }
}

void agregar_argumento_a_llamada(NodoAST* llamada, NodoAST* argumento) {
    if (!llamada || !argumento) return;
    
    NodoAST** lista_args = NULL;
    if (llamada->tipo == AST_LLAMAR_A) {
        lista_args = &llamada->datos.llamar_a.argumentos;
    } else if (llamada->tipo == AST_LLAMADA_FUNCION) {
        lista_args = &llamada->datos.llamada_funcion.argumentos;
    } else {
        return;
    }
    
    if (!*lista_args) {
        *lista_args = argumento;
    } else {
        NodoAST* actual = *lista_args;
        while (actual->siguiente) {
            actual = actual->siguiente;
        }
        actual->siguiente = argumento;
    }
}

// Funciones de liberación de memoria
void liberar_nodo(NodoAST* nodo) {
    if (!nodo) return;
    
    // Liberar hijos recursivamente según el tipo
    switch (nodo->tipo) {
        case AST_PROGRAMA:
            free(nodo->datos.programa.nombre);
            liberar_nodo(nodo->datos.programa.declaraciones);
            liberar_nodo(nodo->datos.programa.bloque_principal);
            liberar_nodo(nodo->datos.programa.funciones);
            liberar_nodo(nodo->datos.programa.subprogramas);
            break;
        case AST_DECLARACION_VARIABLE:
            free(nodo->datos.declaracion_variable.nombre);
            liberar_nodo(nodo->datos.declaracion_variable.valor_inicial);
            break;
        case AST_DECLARACION_CONSTANTE:
            free(nodo->datos.declaracion_constante.nombre);
            liberar_nodo(nodo->datos.declaracion_constante.valor);
            break;
        case AST_DECLARACION_LISTA:
            free(nodo->datos.declaracion_lista.nombre);
            liberar_nodo(nodo->datos.declaracion_lista.valores_iniciales);
            break;
        case AST_DECLARACION_MATRIZ:
            free(nodo->datos.declaracion_matriz.nombre);
            liberar_nodo(nodo->datos.declaracion_matriz.valores_iniciales);
            break;
        case AST_DECLARACION_TEXTO_EXTENSO:
            free(nodo->datos.declaracion_texto_extenso.nombre);
            free(nodo->datos.declaracion_texto_extenso.valor_inicial);
            break;
        case AST_DECLARACION_ARCHIVO:
            free(nodo->datos.declaracion_archivo.nombre);
            free(nodo->datos.declaracion_archivo.ruta);
            break;
        case AST_SI:
            liberar_nodo(nodo->datos.si.condicion);
            liberar_nodo(nodo->datos.si.bloque_si);
            liberar_nodo(nodo->datos.si.sino_si);
            liberar_nodo(nodo->datos.si.bloque_sino);
            break;
        case AST_SINO_SI:
            liberar_nodo(nodo->datos.sino_si.condicion);
            liberar_nodo(nodo->datos.sino_si.bloque);
            liberar_nodo(nodo->datos.sino_si.siguiente_sino_si);
            break;
        case AST_SINO:
            liberar_nodo(nodo->datos.sino.bloque);
            break;
        case AST_PARA:
            free(nodo->datos.para.variable);
            liberar_nodo(nodo->datos.para.inicio);
            liberar_nodo(nodo->datos.para.fin);
            liberar_nodo(nodo->datos.para.paso);
            liberar_nodo(nodo->datos.para.bloque);
            break;
        case AST_MIENTRAS:
            liberar_nodo(nodo->datos.mientras.condicion);
            liberar_nodo(nodo->datos.mientras.bloque);
            break;
        case AST_REALIZAR:
            liberar_nodo(nodo->datos.realizar.bloque);
            liberar_nodo(nodo->datos.realizar.condicion);
            break;
        case AST_SEGUN_CASO:
            liberar_nodo(nodo->datos.segun_caso.expresion);
            liberar_nodo(nodo->datos.segun_caso.casos);
            break;
        case AST_CASO:
            liberar_nodo(nodo->datos.caso.bloque);
            break;
        case AST_POR_DEFECTO:
            liberar_nodo(nodo->datos.por_defecto.bloque);
            break;
        case AST_FUNCION:
            free(nodo->datos.funcion.nombre);
            if (nodo->datos.funcion.parametros) {
                for (int i = 0; i < nodo->datos.funcion.num_parametros; i++) {
                    free(nodo->datos.funcion.parametros[i]);
                }
                free(nodo->datos.funcion.parametros);
            }
            liberar_nodo(nodo->datos.funcion.bloque);
            break;
        case AST_SUBPROGRAMA:
            free(nodo->datos.subprograma.nombre);
            if (nodo->datos.subprograma.parametros) {
                for (int i = 0; i < nodo->datos.subprograma.num_parametros; i++) {
                    free(nodo->datos.subprograma.parametros[i]);
                }
                free(nodo->datos.subprograma.parametros);
            }
            liberar_nodo(nodo->datos.subprograma.bloque);
            break;
        case AST_LLAMAR_A:
            free(nodo->datos.llamar_a.nombre);
            liberar_nodo(nodo->datos.llamar_a.argumentos);
            break;
        case AST_RETORNAR:
            liberar_nodo(nodo->datos.retornar.valor);
            break;
        case AST_LITERAL_NUMERO:
            if (nodo->datos.literal_numero.valor_str)
            {
                free(nodo->datos.literal_numero.valor_str);
            }
            break;
        case AST_LITERAL_TEXTO:
            free(nodo->datos.literal_texto.valor);
            break;
        case AST_VARIABLE:
            free(nodo->datos.variable.nombre);
            break;
        case AST_ACCESO_LISTA:
            free(nodo->datos.acceso_lista.nombre_lista);
            liberar_nodo(nodo->datos.acceso_lista.indice);
            break;
        case AST_ACCESO_MATRIZ:
            free(nodo->datos.acceso_matriz.nombre_matriz);
            liberar_nodo(nodo->datos.acceso_matriz.fila);
            liberar_nodo(nodo->datos.acceso_matriz.columna);
            break;
        case AST_OPERADOR_BINARIO:
            liberar_nodo(nodo->datos.operador_binario.izquierdo);
            liberar_nodo(nodo->datos.operador_binario.derecho);
            break;
        case AST_OPERADOR_UNARIO:
            liberar_nodo(nodo->datos.operador_unario.operando);
            break;
        case AST_LLAMADA_FUNCION:
            free(nodo->datos.llamada_funcion.nombre_funcion);
            liberar_nodo(nodo->datos.llamada_funcion.argumentos);
            break;
        case AST_LLAMADA_FUNCION_MODIFICADORA:
            free(nodo->datos.llamada_funcion_modificadora.nombre_funcion);
            if (nodo->datos.llamada_funcion_modificadora.argumentos)
            {
                liberar_nodo(nodo->datos.llamada_funcion_modificadora.argumentos);
            }
            break;
        case AST_ESCRIBIR:
            liberar_nodo(nodo->datos.escribir.expresion);
            break;
        case AST_LEER:
            free(nodo->datos.leer.variable);
            break;
        case AST_LEERCARACTER:
            free(nodo->datos.leercaracter.variable);
            break;
        case AST_LEERHASTA:
            free(nodo->datos.leerhasta.variable);
            liberar_nodo(nodo->datos.leerhasta.delimitador);
            break;
        case AST_COLORTEXTO:
            free(nodo->datos.colortexto.color);
            break;
        case AST_COLORFONDO:
            free(nodo->datos.colorfondo.color);
            break;
        case AST_CURSOR:
        case AST_POSICIONAR:
            liberar_nodo(nodo->datos.cursor.fila);
            liberar_nodo(nodo->datos.cursor.columna);
            break;
        case AST_ANCHOTERMINAL:
            free(nodo->datos.anchoterminal.variable);
            break;
        case AST_ALTOTERMINAL:
            free(nodo->datos.altoterminal.variable);
            break;
        case AST_ESPERAR:
            liberar_nodo(nodo->datos.esperar.cantidad);
            free(nodo->datos.esperar.unidad);
            break;
        case AST_TIEMPOMS:
            free(nodo->datos.tiempoms.variable);
            break;
        case AST_HORAACTUAL:
            free(nodo->datos.horaactual.variable);
            break;
        case AST_FECHAACTUAL:
            free(nodo->datos.fechaactual.variable);
            break;
        case AST_CALCULAR:
            if (nodo->datos.calcular.destino)
            {
                liberar_nodo(nodo->datos.calcular.destino);
            }
            if (nodo->datos.calcular.expresion)
            {
                liberar_nodo(nodo->datos.calcular.expresion);
            }
            break;
        case AST_RESULTADO:
            liberar_nodo(nodo->datos.resultado.expresion);
            break;
        case AST_ASIGNAR:
            if (nodo->datos.asignar.destino)
            {
                liberar_nodo(nodo->datos.asignar.destino);
            }
            if (nodo->datos.asignar.valor)
            {
                liberar_nodo(nodo->datos.asignar.valor);
            }
            break;
        case AST_ABRIRARCHIVO:
        case AST_USARARCHIVO:
            free(nodo->datos.abrirarchivo.variable);
            free(nodo->datos.abrirarchivo.ruta);
            break;
        case AST_CERRARARCHIVO:
            free(nodo->datos.cerrararchivo.variable);
            break;
        case AST_ESCRIBIRARCHIVO:
            free(nodo->datos.escribirarchivo.variable);
            liberar_nodo(nodo->datos.escribirarchivo.contenido);
            break;
        case AST_LEERARCHIVO:
            free(nodo->datos.leerarchivo.variable_archivo);
            free(nodo->datos.leerarchivo.variable_destino);
            break;
        case AST_SISTEMA:
            free(nodo->datos.sistema.comando);
            break;
        case AST_DIBUJARLINEA:
            liberar_nodo(nodo->datos.dibujarlinea.x1);
            liberar_nodo(nodo->datos.dibujarlinea.y1);
            liberar_nodo(nodo->datos.dibujarlinea.x2);
            liberar_nodo(nodo->datos.dibujarlinea.y2);
            free(nodo->datos.dibujarlinea.patron);
            break;
        case AST_DIBUJARCIRCULO:
            liberar_nodo(nodo->datos.dibujarcirculo.centro_x);
            liberar_nodo(nodo->datos.dibujarcirculo.centro_y);
            liberar_nodo(nodo->datos.dibujarcirculo.radio);
            free(nodo->datos.dibujarcirculo.patron);
            break;
        case AST_RELLENARRECTANGULO:
            liberar_nodo(nodo->datos.rellenarrectangulo.x1);
            liberar_nodo(nodo->datos.rellenarrectangulo.y1);
            liberar_nodo(nodo->datos.rellenarrectangulo.x2);
            liberar_nodo(nodo->datos.rellenarrectangulo.y2);
            free(nodo->datos.rellenarrectangulo.patron);
            break;
        case AST_LEERTECLA:
            free(nodo->datos.leertecla.variable);
            break;
        case AST_COLISIONRECTANGULOS:
            liberar_nodo(nodo->datos.colisionrectangulos.x1);
            liberar_nodo(nodo->datos.colisionrectangulos.y1);
            liberar_nodo(nodo->datos.colisionrectangulos.ancho1);
            liberar_nodo(nodo->datos.colisionrectangulos.alto1);
            liberar_nodo(nodo->datos.colisionrectangulos.x2);
            liberar_nodo(nodo->datos.colisionrectangulos.y2);
            liberar_nodo(nodo->datos.colisionrectangulos.ancho2);
            liberar_nodo(nodo->datos.colisionrectangulos.alto2);
            free(nodo->datos.colisionrectangulos.variable_resultado);
            break;
        case AST_CONFIGURARPIN:
            liberar_nodo(nodo->datos.configurarpin.pin);
            break;
        case AST_ESTADOPIN:
            liberar_nodo(nodo->datos.estadopin.pin);
            break;
        case AST_LEERPIN:
            liberar_nodo(nodo->datos.leerpin.pin);
            if (nodo->datos.leerpin.variable_destino)
                free(nodo->datos.leerpin.variable_destino);
            break;
        case AST_PWM:
            liberar_nodo(nodo->datos.pwm.pin);
            liberar_nodo(nodo->datos.pwm.frecuencia);
            liberar_nodo(nodo->datos.pwm.duty_cycle);
            break;
        case AST_DETENERPWM:
            liberar_nodo(nodo->datos.detenerpwm.pin);
            break;
        case AST_CONECTARBD:
            free(nodo->datos.conectarbd.variable);
            free(nodo->datos.conectarbd.ruta);
            break;
        case AST_CERRARBD:
            free(nodo->datos.cerrarbd.variable);
            break;
        case AST_EJECUTARBD:
            free(nodo->datos.ejecutarbd.variable);
            free(nodo->datos.ejecutarbd.consulta);
            break;
        case AST_CONSULTARBD:
            free(nodo->datos.consultarbd.variable);
            free(nodo->datos.consultarbd.consulta);
            break;
        case AST_CERRARCONSULTABD:
            free(nodo->datos.cerrarconsultabd.variable);
            break;
        case AST_INICIARTRANSACCION:
            free(nodo->datos.iniciartransaccion.variable);
            break;
        case AST_CONFIRMARTRANSACCION:
            free(nodo->datos.confirmartransaccion.variable);
            break;
        case AST_DESHACERTRANSACCION:
            free(nodo->datos.deshacertransaccion.variable);
            break;
        case AST_ETIQUETA:
            free(nodo->datos.etiqueta.nombre);
            break;
        case AST_SALTAR_A:
            free(nodo->datos.saltar_a.nombre_etiqueta);
            break;
        case AST_INCLUIR:
            free(nodo->datos.incluir.ruta_archivo);
            liberar_nodo(nodo->datos.incluir.contenido);
            break;
        case AST_ALERTA:
            liberar_nodo(nodo->datos.alerta.mensaje);
            break;
        case AST_INTENTAR_ATRAPAR:
            liberar_nodo(nodo->datos.intentatrapar.bloque_intent);
            liberar_nodo(nodo->datos.intentatrapar.bloque_atrapar);
            break;
        case AST_BLOQUE:
            liberar_nodo(nodo->datos.bloque.primera);
            break;
        default:
            break;
    }
    
    // Liberar hermanos
    liberar_nodo(nodo->siguiente);
    
    // Liberar el nodo mismo
    free(nodo);
}

void liberar_ast(NodoAST* raiz) {
    liberar_nodo(raiz);
}

// Funciones de utilidad
const char* nombre_tipo_nodo(TipoNodo tipo) {
    static const char *nombres[] = {
        // Programa y bloques
        "PROGRAMA", "BLOQUE PRINCIPAL",

        // Declaraciones
        "VARIABLE", "CONSTANTE", "LISTA",
        "MATRIZ", "TEXTO EXTENSO", "ARCHIVO",

        // Estructuras de control
        "SI", "SINO SI", "SINO", "PARA", "MIENTRAS", "REALIZAR",
        "SEGUN CASO", "CASO", "POR DEFECTO", "CORTE",

        // Funciones y subprogramas
        "FUNCION", "SUBPROGRAMA", "LLAMAR A", "RETORNAR",

        // Literales (nodos internos del AST)
        "número", "cadena_texto", "caracter", "lógico",

        // Accesos y operadores (nodos internos del AST)
        "variable", "[índice]", "[fila][col]",
        "operador", "operador_unario", "llamada_a_funcion",

        // Comandos de E/S
        "ESCRIBIR", "LEER", "LEERCARACTER", "LEERHASTA", "LIMPIARPANTALLA",

        // Formato de texto
        "COLORTEXTO", "COLORFONDO", "TEXTONEGRITA", "TEXTOCURSIVA", "TEXTOSUBRAYADO",
        "RESETTEXTO", "RESETCOLOR",

        // Cursor y terminal
        "CURSOR", "POSICIONAR", "OCULTARCURSOR", "MOSTRARCURSOR",
        "ANCHOTERMINAL", "ALTOTERMINAL",

        // Tiempo
        "ESPERAR", "TIEMPOMS", "HORAACTUAL", "FECHAACTUAL",

        // Cálculo
        "CALCULAR EN", "RESULTADO EN", "ASIGNAR EN",

        // Archivos
        "ABRIRARCHIVO", "CERRARARCHIVO", "ESCRIBIRARCHIVO", "LEERARCHIVO", "USARARCHIVO",

        // Sistema
        "SISTEMA",

        // Dibujo
        "DIBUJARLINEA", "DIBUJARCIRCULO", "RELLENARRECTANGULO",

        // Teclado
        "LEERTECLA", "COLISIONRECTANGULOS",

        // GPIO
        "CONFIGURARPIN", "ESTADOPIN", "LEERPIN",

        // Base de datos
        "CONECTARBD", "CERRARBD", "EJECUTARBD", "CONSULTARBD", "CERRARCONSULTABD",
        "INICIARTRANSACCION", "CONFIRMARTRANSACCION", "DESHACERTRANSACCION",

        // Servidor web
        "INICIARSERVER", "CERRARSERVER",

        // Etiquetas y saltos
        "ETIQUETA", "SALTAR A", "INCLUIR", "BLOQUE",

        // Funciones de texto
        "COPIARTEXTO", "CONCATENARTEXTO", "MAYUSCULAS", "MINUSCULAS",
        "RECORTARTEXTO", "REEMPLAZARTEXTO", "REPETIRTEXTO", "EXTRAERTEXTO",
        "DIVIDIRTEXTO", "ENTEROATEXTO", "DECIMALATEXTO", "CARACTERATEXTO",

        // Manejo de errores
        "ALERTA", "INTENTAR/ATRAPAR"
    };
    if (tipo >= 0 && tipo < sizeof(nombres)/sizeof(nombres[0])) {
        return nombres[tipo];
    }
    return "DESCONOCIDO";
}

const char* nombre_tipo_dato(TipoDato tipo) {
    static const char* nombres[] = {
        "ENTERA", "ENTERA SIN SIGNO", "DECIMAL", "DECIMAL SIN SIGNO",
        "CARACTER", "CARACTER SIN SIGNO", "TEXTO", "TEXTO EXTENSO",
        "ARCHIVO", "LOGICA"
    };
    if (tipo >= 0 && tipo < sizeof(nombres)/sizeof(nombres[0])) {
        return nombres[tipo];
    }
    return "DESCONOCIDO";
}

const char* nombre_operador_binario(OperadorBinario op) {
    static const char* nombres[] = {
        "+", "-", "*", "/", "%", "^",
        "==", "!=", ">", "<", ">=", "<=",
        "Y", "O",
        "BITY", "BITO", "BITXOR",
        "DESPLAZARIZQUIERDA", "DESPLAZARDERECHA",
        "ROTARIZQUIERDA", "ROTARDERECHA",
        "LEERBIT", "ACTIVARBIT", "DESACTIVARBIT"
    };
    if (op >= 0 && op < sizeof(nombres)/sizeof(nombres[0])) {
        return nombres[op];
    }
    return "?";
}

const char* nombre_operador_unario(OperadorUnario op) {
    static const char* nombres[] = {
        "-", "NO", "BITNO"
    };
    if (op >= 0 && op < sizeof(nombres)/sizeof(nombres[0])) {
        return nombres[op];
    }
    return "?";
}

// Funciones de depuración
// Función auxiliar que NO imprime hermanos (para usar dentro de bloques)
static void imprimir_nodo_sin_hermanos(NodoAST* nodo, int nivel) {
    if (!nodo) return;
    
    // Imprimir indentación
    for (int i = 0; i < nivel; i++) printf("  ");
    
    printf("[%s] línea %d", nombre_tipo_nodo(nodo->tipo), nodo->linea);
    
    // Imprimir información específica según el tipo
    switch (nodo->tipo) {
        case AST_PROGRAMA:
            printf(" - nombre: %s\n", nodo->datos.programa.nombre ? nodo->datos.programa.nombre : "(sin nombre)");
            if (nodo->datos.programa.declaraciones) {
                for (int i = 0; i < nivel + 1; i++) printf("  ");
                printf("├─ DECLARACIONES:\n");
                imprimir_nodo_sin_hermanos(nodo->datos.programa.declaraciones, nivel + 2);
            }
            if (nodo->datos.programa.bloque_principal) {
                for (int i = 0; i < nivel + 1; i++) printf("  ");
                printf("├─ BLOQUE_PRINCIPAL:\n");
                imprimir_nodo_sin_hermanos(nodo->datos.programa.bloque_principal, nivel + 2);
            }
            if (nodo->datos.programa.funciones) {
                for (int i = 0; i < nivel + 1; i++) printf("  ");
                printf("├─ FUNCIONES:\n");
                imprimir_nodo_sin_hermanos(nodo->datos.programa.funciones, nivel + 2);
            }
            if (nodo->datos.programa.subprogramas) {
                for (int i = 0; i < nivel + 1; i++) printf("  ");
                printf("└─ SUBPROGRAMAS:\n");
                imprimir_nodo_sin_hermanos(nodo->datos.programa.subprogramas, nivel + 2);
            }
            return;
            
        case AST_DECLARACION_VARIABLE:
            printf(" - $%s : %s\n", nodo->datos.declaracion_variable.nombre,
                   nombre_tipo_dato(nodo->datos.declaracion_variable.tipo_dato));
            if (nodo->datos.declaracion_variable.valor_inicial) {
                imprimir_nodo_sin_hermanos(nodo->datos.declaracion_variable.valor_inicial, nivel + 1);
            }
            break;
            
        case AST_DECLARACION_CONSTANTE:
            printf(" - $%s : %s\n", nodo->datos.declaracion_constante.nombre,
                   nombre_tipo_dato(nodo->datos.declaracion_constante.tipo_dato));
            if (nodo->datos.declaracion_constante.valor) {
                imprimir_nodo_sin_hermanos(nodo->datos.declaracion_constante.valor, nivel + 1);
            }
            break;

        case AST_LITERAL_NUMERO:
            if (nodo->datos.literal_numero.valor_str)
            {
                printf(" - valor_str: %s (double: %g)\n",
                       nodo->datos.literal_numero.valor_str,
                       nodo->datos.literal_numero.valor);
            }
            else
            {
                printf(" - valor: %g\n", nodo->datos.literal_numero.valor);
            }
            break;

        case AST_LITERAL_TEXTO:
            printf(" - valor: \"%s\"\n", nodo->datos.literal_texto.valor ? nodo->datos.literal_texto.valor : "");
            break;
            
        case AST_LITERAL_CARACTER:
            printf(" - valor: '%s'\n", nodo->datos.literal_caracter.valor);
            break;
            
        case AST_LITERAL_LOGICO:
            printf(" - valor: %s\n", nodo->datos.literal_logico.valor ? "VERDADERO" : "FALSO");
            break;
            
        case AST_VARIABLE:
            printf(" - $%s\n", nodo->datos.variable.nombre);
            break;
            
        case AST_OPERADOR_BINARIO:
            printf(" - op: %s\n", nombre_operador_binario(nodo->datos.operador_binario.operador));
            if (nodo->datos.operador_binario.izquierdo) {
                for (int i = 0; i < nivel + 1; i++) printf("  ");
                printf("├─ IZQ:\n");
                imprimir_nodo_sin_hermanos(nodo->datos.operador_binario.izquierdo, nivel + 2);
            }
            if (nodo->datos.operador_binario.derecho) {
                for (int i = 0; i < nivel + 1; i++) printf("  ");
                printf("└─ DER:\n");
                imprimir_nodo_sin_hermanos(nodo->datos.operador_binario.derecho, nivel + 2);
            }
            return;
            
        case AST_OPERADOR_UNARIO:
            printf(" - op: %s\n", nombre_operador_unario(nodo->datos.operador_unario.operador));
            if (nodo->datos.operador_unario.operando) {
                imprimir_nodo_sin_hermanos(nodo->datos.operador_unario.operando, nivel + 1);
            }
            break;
            
        case AST_FUNCION:
            printf(" - $%s retorna %s, params: %d\n", 
                   nodo->datos.funcion.nombre,
                   nombre_tipo_dato(nodo->datos.funcion.tipo_retorno),
                   nodo->datos.funcion.num_parametros);
            for (int i = 0; i < nodo->datos.funcion.num_parametros; i++) {
                for (int j = 0; j < nivel + 1; j++) printf("  ");
                printf("├─ param: $%s\n", nodo->datos.funcion.parametros[i]);
            }
            if (nodo->datos.funcion.bloque) {
                for (int i = 0; i < nivel + 1; i++) printf("  ");
                printf("└─ BLOQUE:\n");
                imprimir_nodo_sin_hermanos(nodo->datos.funcion.bloque, nivel + 2);
            }
            return;
            
        case AST_SUBPROGRAMA:
            printf(" - $%s, params: %d\n", 
                   nodo->datos.subprograma.nombre,
                   nodo->datos.subprograma.num_parametros);
            for (int i = 0; i < nodo->datos.subprograma.num_parametros; i++) {
                for (int j = 0; j < nivel + 1; j++) printf("  ");
                printf("├─ param: $%s\n", nodo->datos.subprograma.parametros[i]);
            }
            if (nodo->datos.subprograma.bloque) {
                for (int i = 0; i < nivel + 1; i++) printf("  ");
                printf("└─ BLOQUE:\n");
                imprimir_nodo_sin_hermanos(nodo->datos.subprograma.bloque, nivel + 2);
            }
            return;
            
        case AST_SI:
            printf("\n");
            if (nodo->datos.si.condicion) {
                for (int i = 0; i < nivel + 1; i++) printf("  ");
                printf("├─ COND:\n");
                imprimir_nodo_sin_hermanos(nodo->datos.si.condicion, nivel + 2);
            }
            if (nodo->datos.si.bloque_si) {
                for (int i = 0; i < nivel + 1; i++) printf("  ");
                printf("├─ ENTONCES:\n");
                imprimir_nodo_sin_hermanos(nodo->datos.si.bloque_si, nivel + 2);
            }
            if (nodo->datos.si.sino_si) {
                for (int i = 0; i < nivel + 1; i++) printf("  ");
                printf("├─ SINO_SI:\n");
                imprimir_nodo_sin_hermanos(nodo->datos.si.sino_si, nivel + 2);
            }
            if (nodo->datos.si.bloque_sino) {
                for (int i = 0; i < nivel + 1; i++) printf("  ");
                printf("└─ SINO:\n");
                imprimir_nodo_sin_hermanos(nodo->datos.si.bloque_sino, nivel + 2);
            }
            return;
            
        case AST_PARA:
            printf(" - $%s\n", nodo->datos.para.variable);
            if (nodo->datos.para.inicio) {
                for (int i = 0; i < nivel + 1; i++) printf("  ");
                printf("├─ DESDE:\n");
                imprimir_nodo_sin_hermanos(nodo->datos.para.inicio, nivel + 2);
            }
            if (nodo->datos.para.fin) {
                for (int i = 0; i < nivel + 1; i++) printf("  ");
                printf("├─ HASTA:\n");
                imprimir_nodo_sin_hermanos(nodo->datos.para.fin, nivel + 2);
            }
            if (nodo->datos.para.paso) {
                for (int i = 0; i < nivel + 1; i++) printf("  ");
                printf("├─ PASO:\n");
                imprimir_nodo_sin_hermanos(nodo->datos.para.paso, nivel + 2);
            }
            if (nodo->datos.para.bloque) {
                for (int i = 0; i < nivel + 1; i++) printf("  ");
                printf("└─ HACER:\n");
                imprimir_nodo_sin_hermanos(nodo->datos.para.bloque, nivel + 2);
            }
            return;
            
        case AST_MIENTRAS:
            printf("\n");
            if (nodo->datos.mientras.condicion) {
                for (int i = 0; i < nivel + 1; i++) printf("  ");
                printf("├─ COND:\n");
                imprimir_nodo_sin_hermanos(nodo->datos.mientras.condicion, nivel + 2);
            }
            if (nodo->datos.mientras.bloque) {
                for (int i = 0; i < nivel + 1; i++) printf("  ");
                printf("└─ HACER:\n");
                imprimir_nodo_sin_hermanos(nodo->datos.mientras.bloque, nivel + 2);
            }
            return;
            
        case AST_INCLUIR:
            printf(" - \"%s\"\n", nodo->datos.incluir.ruta_archivo);
            if (nodo->datos.incluir.contenido) {
                for (int i = 0; i < nivel + 1; i++) printf("  ");
                printf("└─ CONTENIDO:\n");
                imprimir_nodo_sin_hermanos(nodo->datos.incluir.contenido, nivel + 2);
            }
            return;
            
        case AST_BLOQUE:
            printf(" - sentencias:\n");
            NodoAST* actual = nodo->datos.bloque.primera;
            while (actual) {
                imprimir_nodo_sin_hermanos(actual, nivel + 1);
                actual = actual->siguiente;
            }
            return;
            
        case AST_ESCRIBIR:
            printf("\n");
            if (nodo->datos.escribir.expresion) {
                for (int i = 0; i < nivel + 1; i++) printf("  ");
                printf("└─ expr:\n");
                imprimir_nodo_sin_hermanos(nodo->datos.escribir.expresion, nivel + 2);
            }
            return;

        case AST_CALCULAR:
            if (nodo->datos.calcular.destino && nodo->datos.calcular.destino->tipo == AST_VARIABLE)
            {
                printf(" - $%s\n", nodo->datos.calcular.destino->datos.variable.nombre);
            }
            else
            {
                printf(" - <destino>\n");
            }
            if (nodo->datos.calcular.expresion)
            {
                for (int i = 0; i < nivel + 1; i++)
                    printf("  ");
                printf("└─ expr:\n");
                imprimir_nodo_sin_hermanos(nodo->datos.calcular.expresion, nivel + 2);
            }
            return;

        case AST_ASIGNAR:
            if (nodo->datos.asignar.destino && nodo->datos.asignar.destino->tipo == AST_VARIABLE)
            {
                printf(" - $%s\n", nodo->datos.asignar.destino->datos.variable.nombre);
            }
            else
            {
                printf(" - <destino>\n");
            }
            if (nodo->datos.asignar.valor)
            {
                for (int i = 0; i < nivel + 1; i++)
                    printf("  ");
                printf("└─ valor:\n");
                imprimir_nodo_sin_hermanos(nodo->datos.asignar.valor, nivel + 2);
            }
            break;

        case AST_HORAACTUAL:
            printf(" - %s\n", nodo->datos.horaactual.variable);
            return;
            
        case AST_FECHAACTUAL:
            printf(" - %s\n", nodo->datos.fechaactual.variable);
            return;
            
        default:
            printf("\n");
            break;
    }
}

// Función pública que SÍ imprime hermanos (para el primer nivel)
void imprimir_ast(NodoAST* nodo, int nivel) {
    while (nodo) {
        imprimir_nodo_sin_hermanos(nodo, nivel);
        nodo = nodo->siguiente;
    }
}

void imprimir_ast_detalle(NodoAST* nodo, int nivel) {
    imprimir_ast(nodo, nivel);
}

