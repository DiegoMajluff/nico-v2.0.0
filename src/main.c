/*
 * Nico v2.0.0 - Intérprete Educativo de Scripting en Español
 * @file:         main.c
 * @author:       Diego Alejandro Majluff (Diseño, Arquitectura y Supervisión)
 * @ai_assist:    Qwen (Alibaba Cloud) - Implementación, Debugging y Optimización
 * @license:      MIT / Personal Use (ver LICENSE)
 * @description:  Punto de entrada principal del intérprete. Gestiona argumentos
 *                de línea de comandos, modo interactivo (REPL) y ejecución de
 *                archivos .nico. Inicializa lexer, parser y evaluador, maneja
 *                señales (Ctrl+C) y limpieza de recursos al finalizar.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <float.h>
#include <signal.h>
#include "lexer.h"
#include "parser.h"
#include "evaluator.h"
#include <unistd.h>
#ifndef _WIN32
#include <sys/resource.h>
#endif
#include <errno.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <termios.h>
#endif

#define MAX_LINEA 1024
#define MAX_PROGRAMA 100000

// Forward declarations
void mostrar_ayuda(void);
void comando_rangos(void);
void evaluar_expresion(const char* expr);
void ejecutar_archivo(const char* ruta);
void repl(void);
void handler_sigint(int sig);

// Variable global para el programa cargado
char* programa_cargado = NULL;
int archivo_cargado = 0;

void handler_sigint(int sig)
{
    (void)sig;
    // Restaurar terminal ANTES de salir
    teclado_restaurar_modo();
    printf("\n> Programa interrumpido.\n");
    if (programa_cargado)
    {
        free(programa_cargado);
        programa_cargado = NULL;
    }
    exit(0);
}

void mostrar_ayuda(void) {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════╗\n");
    printf("║          COMANDOS DEL INTÉRPRETE NICO v2.0.0           ║\n");
    printf("╠════════════════════════════════════════════════════════╣\n");
    printf("║  USAR archivo.nico   Carga un archivo .nico            ║\n");
    printf("║  CORRER              Ejecuta el programa cargado       ║\n");
    printf("║  CERRAR              Cierra el programa actual         ║\n");
    printf("║  RANGOS              Muestra rangos de variables       ║\n");
    printf("║  SALIR               Sale del intérprete               ║\n");
    printf("║  ?                   Muestra esta ayuda                ║\n");
    printf("╚════════════════════════════════════════════════════════╝\n");
    printf("\n");
}

void comando_rangos(void) {
    printf("\n");
    printf("   RANGOS DE VARIABLES - NICO v2.0.0\n");
    printf("\n");
    printf("   VARIABLES ENTERAS:\n");
    printf("     Rango: %lld a %lld\n", LLONG_MIN, LLONG_MAX);
    printf("\n");
    printf("   VARIABLES ENTERAS SIN SIGNO:\n");
    printf("     Rango: 0 a %llu\n", ULLONG_MAX);
    printf("\n");
    printf("   VARIABLES DECIMALES:\n");
    printf("     Rango: %e a %e\n", -DBL_MAX, DBL_MAX);
    printf("\n");
    printf("   VARIABLES DECIMALES SIN SIGNO:\n");
    printf("     Rango: 0.0 a %e (mismo tipo que DECIMAL, solo valores >= 0)\n", DBL_MAX);
    printf("\n");
    printf("   VARIABLES CARACTER:\n");
    printf("     Rango: %d a %d (ASCII extendido, signed char)\n", SCHAR_MIN, SCHAR_MAX);
    printf("\n");
    printf("   VARIABLES CARACTER SIN SIGNO:\n");
    printf("     Rango: 0 a %u (ASCII extendido, unsigned char)\n", UCHAR_MAX);
    printf("\n");
    printf("   VARIABLES TEXTO y TEXTO EXTENSO:\n");
    printf("     Largo: Dinamico (limitado por RAM disponible)\n");
    printf("\n");
    printf("   VARIABLES LOGICA:\n");
    printf("     Para valores o estados logicos: VERDADERO/FALSO\n");
    printf("\n");
    printf("   ARCHIVOS:\n");
    printf("     Modos: ESCRITURA, AGREGAR, LECTURA, LECTOESCRITURA\n");
    printf("\n");
    printf("   FUNCIONES Y SUBPROGRAMAS:\n");
    printf("     Soporte completo con parametros y retorno\n");
    printf("\n");
}

void evaluar_expresion(const char* expr) {
    Lexer* lexer = lexer_crear(expr);
    if (!lexer) {
        fprintf(stderr, "Error: No se pudo crear el lexer.\n");
        return;
    }
    
    Parser* parser = parser_crear(lexer);
    if (!parser) {
        fprintf(stderr, "Error: No se pudo crear el parser.\n");
        lexer_destruir(lexer);
        return;
    }
    
    Contexto* ctx = contexto_crear();
    if (!ctx) {
        fprintf(stderr, "Error: No se pudo crear el contexto.\n");
        parser_destruir(parser);
        lexer_destruir(lexer);
        return;
    }
       
    NodoAST* ast = parsear_expresion(parser);
    
    if (parser_tiene_error(parser)) {
        fprintf(stderr, "Error de parsing: %s\n", parser_obtener_error(parser));
    } else if (ast) {
        Valor resultado = evaluar_nodo(ast, ctx);
        if (!ctx->hay_error) {
            valor_imprimir(resultado);
            printf("\n");
        } else {
            fprintf(stderr, "Error de ejecución.\n");
        }
        valor_destruir(&resultado);
        liberar_nodo(ast);
    }
    
    contexto_destruir(ctx);
    parser_destruir(parser);
    lexer_destruir(lexer);
}

void ejecutar_archivo(const char* ruta) {
#ifndef _WIN32
    obtener_terminal_original();
#endif

    FILE* archivo = fopen(ruta, "r");
    if (!archivo) {
        fprintf(stderr, "Error: No se pudo abrir el archivo '%s'\n", ruta);
        return;
    }
    
    fseek(archivo, 0, SEEK_END);
    long tamano = ftell(archivo);
    fseek(archivo, 0, SEEK_SET);
    
    char* codigo = malloc(tamano + 1);
    if (!codigo) {
        fprintf(stderr, "Error: No se pudo asignar memoria.\n");
        fclose(archivo);
        return;
    }
    
    fread(codigo, 1, tamano, archivo);
    codigo[tamano] = '\0';
    fclose(archivo);
    
    Lexer* lexer = lexer_crear(codigo);
    Parser* parser = parser_crear(lexer);
    Contexto* ctx = contexto_crear();
    
    if (!lexer || !parser || !ctx) {
        fprintf(stderr, "Error: No se pudo inicializar el intérprete.\n");
        if (ctx) contexto_destruir(ctx);
        if (parser) parser_destruir(parser);
        if (lexer) lexer_destruir(lexer);
        free(codigo);
        return;
    }
       
    NodoAST* ast = parser_parsear(parser);
    
    if (parser_tiene_error(parser)) {
        fprintf(stderr, "Error de parsing: %s\n", parser_obtener_error(parser));
    } else if (ast) {

        // Obtener nombre del programa desde el AST
        const char *nombre_prog = (ast->tipo == AST_PROGRAMA && ast->datos.programa.nombre)
                                      ? ast->datos.programa.nombre
                                      : ruta;

        printf("\n> Corriendo programa '%s'\n", nombre_prog);
        printf("\n");

        teclado_iniciar_modo_raw();
        evaluar_nodo(ast, ctx);
        
        // Restaurar terminal ANTES de imprimir mensajes finales
        teclado_restaurar_modo();
        printf("\n");
        
        // Verificar si hubo error durante la ejecución
        if (ctx->hay_error) {
            fprintf(stderr, "\n❌ Error de ejecución: %s\n", ctx->mensaje_error[0] ? ctx->mensaje_error : "Error desconocido");
        } else {
            fprintf(stderr, "\n> Programa '%s' finalizado\n", nombre_prog);
        }
        liberar_nodo(ast);
    }
    
    contexto_destruir(ctx);
    parser_destruir(parser);
    lexer_destruir(lexer);
    free(codigo);
}

void repl(void) {
    printf("\n> Intérprete del lenguaje Nico v2.0.0\n");
    printf("> Modo interactivo. Escribí SALIR para terminar.\n");
    printf("> Escribí ? para ver ayuda.\n\n");
    
    char comando[MAX_LINEA];
    
    while (1) {
        printf("\n>>> ");
        fflush(stdout);
        
        if (!fgets(comando, MAX_LINEA, stdin)) {
            printf("\n");
            break;
        }
        
        // Remover salto de línea
        comando[strcspn(comando, "\n")] = '\0';
        
        // Remover espacios al inicio
        char* ptr = comando;
        while (*ptr == ' ' || *ptr == '\t') ptr++;
        
        if (strlen(ptr) == 0) continue;
        
        if (strcmp(ptr, "?") == 0) {
            mostrar_ayuda();
            continue;
        }
        
        if (strncmp(ptr, "USAR ", 5) == 0) {
            if (archivo_cargado) {
                fprintf(stderr, "Error: Ya hay un programa cargado. Usá CERRAR primero.\n");
                continue;
            }
            
            char* archivo = ptr + 5;
            while (*archivo == ' ' || *archivo == '\t') archivo++;
            
            char* ext = strrchr(archivo, '.');
            if (!ext || strcmp(ext, ".nico") != 0) {
                fprintf(stderr, "Error: El archivo debe tener extensión .nico\n");
                continue;
            }
            
            FILE* f = fopen(archivo, "r");
            if (!f) {
                fprintf(stderr, "Error: No se pudo abrir '%s'.\n", archivo);
                continue;
            }
            
            fseek(f, 0, SEEK_END);
            long tamano = ftell(f);
            fseek(f, 0, SEEK_SET);
            
            programa_cargado = malloc(tamano + 1);
            if (!programa_cargado) {
                fprintf(stderr, "Error: No se pudo asignar memoria.\n");
                fclose(f);
                continue;
            }
            
            fread(programa_cargado, 1, tamano, f);
            programa_cargado[tamano] = '\0';
            fclose(f);
            
            archivo_cargado = 1;
            printf("> Archivo '%s' cargado con éxito.\n", archivo);
            continue;
        }
        
        if (strcmp(ptr, "CORRER") == 0) {
            if (!archivo_cargado || !programa_cargado) {
                fprintf(stderr, "Error: No hay programa cargado. Usá USAR primero.\n");
                continue;
            }

#ifndef _WIN32
            obtener_terminal_original();
#endif

            Lexer* lexer = lexer_crear(programa_cargado);
            Parser* parser = parser_crear(lexer);
            Contexto* ctx = contexto_crear();
            
            if (!lexer || !parser || !ctx) {
                fprintf(stderr, "Error: No se pudo inicializar el intérprete.\n");
                if (ctx) contexto_destruir(ctx);
                if (parser) parser_destruir(parser);
                if (lexer) lexer_destruir(lexer);
                continue;
            }
            
            NodoAST* ast = parser_parsear(parser);
            
            if (parser_tiene_error(parser)) {
                fprintf(stderr, "Error de parsing: %s\n", parser_obtener_error(parser));
            } else if (ast) {
                printf("\n> Ejecutando programa cargado.\n");
                printf("\n");
                teclado_iniciar_modo_raw();
                evaluar_nodo(ast, ctx);
                teclado_restaurar_modo();
                printf("\n");
                printf("\n> Ejecución finalizada.\n");
                liberar_nodo(ast);
            }
            
            contexto_destruir(ctx);
            parser_destruir(parser);
            lexer_destruir(lexer);
            continue;
        }
        
        if (strcmp(ptr, "CERRAR") == 0) {
            if (!archivo_cargado) {
                fprintf(stderr, "Error: No hay programa cargado.\n");
                continue;
            }
            
            if (programa_cargado) {
                free(programa_cargado);
                programa_cargado = NULL;
            }
            archivo_cargado = 0;
            printf("> Programa cerrado.\n");
            continue;
        }
        
        if (strcmp(ptr, "RANGOS") == 0) {
            comando_rangos();
            continue;
        }
        
        if (strcmp(ptr, "SALIR") == 0) {
            if (programa_cargado) {
                free(programa_cargado);
                programa_cargado = NULL;
            }
            printf("> Intérprete finalizado.\n");
            break;
        }
        
        fprintf(stderr, "Comando desconocido. Escribí ? para ayuda.\n");
    }
}

int main(int argc, char* argv[]) {

#ifndef _WIN32
    struct rlimit rl;
    getrlimit(RLIMIT_STACK, &rl);
#ifdef DEBUG
    printf("Stack actual: cur=%lu, max=%lu\n", (unsigned long)rl.rlim_cur, (unsigned long)rl.rlim_max);
#endif
    if (rl.rlim_cur < 256 * 1024 * 1024)
    {
        rl.rlim_cur = 256 * 1024 * 1024;
        if (rl.rlim_cur > rl.rlim_max)
        {
            rl.rlim_max = 256 * 1024 * 1024;
        }
#ifdef DEBUG
        int result = setrlimit(RLIMIT_STACK, &rl);
        if (result != 0)
        {
            printf("Error al aumentar stack: %d (errno=%d)\n", result, errno);
        }
        else
        {
            printf("Stack aumentado a 256MB\n");
        }
#else
        setrlimit(RLIMIT_STACK, &rl);
#endif
    }
#endif
    
#ifdef _WIN32
        // Configurar consola Windows para UTF-8
        SetConsoleOutputCP(65001);
        SetConsoleCP(65001);
#endif
        
        signal(SIGINT, handler_sigint);
        signal(SIGTERM, handler_sigint);
        // Registrar función de restauración de terminal para ejecución normal
        atexit(teclado_restaurar_modo);

        // Modo expresión: ./nico -e "expresión"
        if (argc == 3 && strcmp(argv[1], "-e") == 0)
        {
            evaluar_expresion(argv[2]);
            return 0;
        }
    // Ayuda
    else if (argc == 2 && (strcmp(argv[1], "-a") == 0 || strcmp(argv[1], "--ayuda") == 0))
    {
        {
            printf("Uso: nico [opciones] [archivo.nico]\n");
            printf("Opciones:\n");
            printf("  -e <expr>    Evaluar expresión y salir\n");
            printf("  -a, --ayuda  Mostrar esta ayuda\n");
            printf("Sin opciones:  Iniciar REPL interactivo\n");
            return 0;
        }
        }
    
    // Modo archivo: ./nico archivo.nico
    if (argc == 2)
    {
        char *ext = strrchr(argv[1], '.');
        if (!ext || strcmp(ext, ".nico") != 0)
        {
            fprintf(stderr, "Error: El archivo debe tener extensión .nico\n");
            return 1;
        }
        ejecutar_archivo(argv[1]);
        return 0;
    }

    // Modo REPL: ./nico
    if (argc == 1)
    {
        repl();
        return 0;
    }

    fprintf(stderr, "Uso: nico [opciones] [archivo.nico]\n");
    fprintf(stderr, "Escribí 'nico -h' para más información.\n");
    return 1;
    }
