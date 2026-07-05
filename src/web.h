/*
 * Nico v2.0.0 - Intérprete Educativo de Scripting en Español
 * @file:         web.h
 * @author:       Diego Alejandro Majluff (Diseño, Arquitectura y Supervisión)
 * @ai_assist:    Qwen (Alibaba Cloud) - Implementación, Debugging y Optimización
 * @license:      MIT / Personal Use (ver LICENSE)
 * @description:  Definiciones del servidor HTTP y panel de administración web.
 *                Contiene estructura del servidor web, configuración de puertos,
 *                endpoints REST y funciones de gestión del servidor.
 */
#ifndef WEB_H
#define WEB_H

#include <pthread.h>

// Forward declaration
typedef struct Contexto Contexto;

void web_set_contexto(Contexto *ctx);
void cmd_iniciarserver(Contexto *ctx, int puerto);
void cmd_detenerserver(Contexto *ctx);

#endif
