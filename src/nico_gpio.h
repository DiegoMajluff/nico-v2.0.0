/*
 * Nico v2.0.0 - Intérprete Educativo de Scripting en Español
 * @file:         nico_gpio.h
 * @author:       Diego Alejandro Majluff (Diseño, Arquitectura y Supervisión)
 * @ai_assist:    Qwen (Alibaba Cloud) - Implementación, Debugging y Optimización
 * @license:      MIT / Personal Use (ver LICENSE)
 * @description:  Definiciones para control de GPIO en Raspberry Pi. Contiene
 *                funciones para configurar pines como entrada/salida, leer
 *                estados digitales y escribir valores usando libgpiod.
 */
#ifndef NICO_GPIO_H
#define NICO_GPIO_H

#ifdef HAVE_LIBGPIOD
#include <gpiod.h>
#endif

// Inicialización y limpieza
int nico_gpio_init(void);
void nico_gpio_cleanup(void);
int gpio_verificar_disponibilidad(void);

// Comandos GPIO con parámetros tipados
void procesar_gpio_configurar(int pin, int direccion, int bias);
void procesar_gpio_estado_pin(int pin, int valor);
int procesar_gpio_leer(int pin);

#endif
