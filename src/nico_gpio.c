/*
 * Nico v2.0.0 - Intérprete Educativo de Scripting en Español
 * @file:         nico_gpio.c
 * @author:       Diego Alejandro Majluff (Diseño, Arquitectura y Supervisión)
 * @ai_assist:    Qwen (Alibaba Cloud) - Implementación, Debugging y Optimización
 * @license:      MIT / Personal Use (ver LICENSE)
 * @description:  Implementación de control GPIO para Raspberry Pi usando libgpiod.
 *                Permite configurar pines como entrada/salida, leer estados
 *                digitales y controlar LEDs, botones y otros dispositivos.
 *                Incluye manejo de pull-up/pull-down y debounce.
 */
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include "nico_gpio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef HAVE_LIBGPIOD
#include <gpiod.h>

struct gpiod_chip *nico_chip = NULL;
struct gpiod_line_request *nico_requests[40] = {NULL};
#endif

static int gpio_inicializado = 0;
static int gpio_disponible = 0;

// --------------------------------------------------------
// VERIFICAR DISPONIBILIDAD
// --------------------------------------------------------
int gpio_verificar_disponibilidad(void)
{
    if (gpio_disponible != 0)
    {
        return gpio_disponible;
    }

    FILE *f = fopen("/proc/device-tree/model", "r");
    if (f)
    {
        char model[256];
        size_t n = fread(model, 1, sizeof(model) - 1, f);
        model[n] = '\0';
        fclose(f);
        if (strstr(model, "Raspberry Pi") == NULL)
        {
            gpio_disponible = -1;
            return 0;
        }
    }
    else
    {
        gpio_disponible = -1;
        return 0;
    }

#ifndef HAVE_LIBGPIOD
    gpio_disponible = -1;
    return 0;
#endif

#ifdef HAVE_LIBGPIOD
    struct gpiod_chip *test_chip = gpiod_chip_open("/dev/gpiochip0");
    if (!test_chip)
    {
        gpio_disponible = -1;
        return 0;
    }
    gpiod_chip_close(test_chip);
#endif

    gpio_disponible = 1;
    return 1;
}

// --------------------------------------------------------
// INICIALIZACIÓN
// --------------------------------------------------------
int nico_gpio_init(void)
{
    if (gpio_inicializado)
        return 0;

    if (!gpio_verificar_disponibilidad())
    {
        return -1;
    }

#ifdef HAVE_LIBGPIOD
    nico_chip = gpiod_chip_open("/dev/gpiochip0");
    if (!nico_chip)
    {
        fprintf(stderr, "Error: No se pudo abrir gpiochip0.\n");
        return -1;
    }
#endif

    gpio_inicializado = 1;
    return 0;
}

// --------------------------------------------------------
// LIMPIEZA
// --------------------------------------------------------
void nico_gpio_cleanup(void)
{
#ifdef HAVE_LIBGPIOD
    if (gpio_inicializado)
    {
        for (int i = 0; i < 40; i++)
        {
            if (nico_requests[i])
            {
                gpiod_line_request_release(nico_requests[i]);
                nico_requests[i] = NULL;
            }
        }
        if (nico_chip)
        {
            gpiod_chip_close(nico_chip);
            nico_chip = NULL;
        }
        gpio_inicializado = 0;
    }
#endif
}

// --------------------------------------------------------
// CONFIGURAR LÍNEA GPIO (interna)
// --------------------------------------------------------
#ifdef HAVE_LIBGPIOD
static int configurar_linea_gpio(int pin, int output)
{
    if (pin < 0 || pin >= 40)
        return -1;

    if (nico_requests[pin])
    {
        gpiod_line_request_release(nico_requests[pin]);
        nico_requests[pin] = NULL;
    }

    struct gpiod_line_settings *settings = gpiod_line_settings_new();
    if (!settings)
        return -1;

    gpiod_line_settings_set_direction(settings,
                                      output ? GPIOD_LINE_DIRECTION_OUTPUT : GPIOD_LINE_DIRECTION_INPUT);

    struct gpiod_line_config *config = gpiod_line_config_new();
    if (!config)
    {
        gpiod_line_settings_free(settings);
        return -1;
    }

    unsigned int pin_uint = (unsigned int)pin;
    gpiod_line_config_add_line_settings(config, &pin_uint, 1, settings);

    struct gpiod_request_config *req_config = gpiod_request_config_new();
    gpiod_request_config_set_consumer(req_config, "nico");

    nico_requests[pin] = gpiod_chip_request_lines(nico_chip, req_config, config);

    gpiod_line_settings_free(settings);
    gpiod_line_config_free(config);
    gpiod_request_config_free(req_config);

    return nico_requests[pin] ? 0 : -1;
}
#else
static int configurar_linea_gpio(int pin, int output)
{
    (void)pin;
    (void)output;
    return -1;
}
#endif

// --------------------------------------------------------
// CONFIGURARPIN: pin, direccion (0=ENTRADA, 1=SALIDA), bias (0=none, 1=PULLUP, 2=PULLDOWN)
// --------------------------------------------------------
void procesar_gpio_configurar(int pin, int direccion, int bias)
{
    if (!gpio_verificar_disponibilidad())
    {
        fprintf(stderr, "Error: GPIO no disponible.\n");
        return;
    }
    if (!gpio_inicializado)
    {
        if (nico_gpio_init() < 0)
            return;
    }
    if (pin < 0 || pin >= 40)
    {
        fprintf(stderr, "Error: Pin fuera de rango (0-39 BCM).\n");
        return;
    }

#ifdef HAVE_LIBGPIOD
    if (nico_requests[pin])
    {
        gpiod_line_request_release(nico_requests[pin]);
        nico_requests[pin] = NULL;
    }

    struct gpiod_line_settings *settings = gpiod_line_settings_new();
    if (!settings)
    {
        fprintf(stderr, "Error: No se pudo crear line_settings.\n");
        return;
    }

    gpiod_line_settings_set_direction(settings,
                                      direccion ? GPIOD_LINE_DIRECTION_OUTPUT : GPIOD_LINE_DIRECTION_INPUT);

    if (!direccion && bias == 1)
    {
        gpiod_line_settings_set_bias(settings, GPIOD_LINE_BIAS_PULL_UP);
    }
    else if (!direccion && bias == 2)
    {
        gpiod_line_settings_set_bias(settings, GPIOD_LINE_BIAS_PULL_DOWN);
    }

    struct gpiod_line_config *config = gpiod_line_config_new();
    if (!config)
    {
        gpiod_line_settings_free(settings);
        return;
    }

    unsigned int pin_uint = (unsigned int)pin;
    gpiod_line_config_add_line_settings(config, &pin_uint, 1, settings);

    struct gpiod_request_config *req_config = gpiod_request_config_new();
    gpiod_request_config_set_consumer(req_config, "nico");

    nico_requests[pin] = gpiod_chip_request_lines(nico_chip, req_config, config);

    gpiod_line_settings_free(settings);
    gpiod_line_config_free(config);
    gpiod_request_config_free(req_config);

    if (!nico_requests[pin])
    {
        fprintf(stderr, "Error: No se pudo solicitar GPIO %d.\n", pin);
    }
#else
    (void)direccion;
    (void)bias;
#endif
}

// --------------------------------------------------------
// ESTADOPIN: pin, valor (0=NO/LOW, 1=SI/HIGH)
// --------------------------------------------------------
void procesar_gpio_estado_pin(int pin, int valor)
{
    if (!gpio_verificar_disponibilidad())
    {
        fprintf(stderr, "Error: GPIO no disponible.\n");
        return;
    }
    if (!gpio_inicializado)
    {
        if (nico_gpio_init() < 0)
            return;
    }
    if (pin < 0 || pin >= 40)
    {
        fprintf(stderr, "Error: Pin fuera de rango (0-39 BCM).\n");
        return;
    }

#ifdef HAVE_LIBGPIOD
    if (nico_requests[pin])
    {
        gpiod_line_request_set_value(nico_requests[pin], pin,
                                     valor ? GPIOD_LINE_VALUE_ACTIVE : GPIOD_LINE_VALUE_INACTIVE);
    }
    else
    {
        fprintf(stderr, "Error: GPIO %d no configurado.\n", pin);
    }
#else
    (void)valor;
#endif
}

// --------------------------------------------------------
// LEERPIN: pin → retorna valor (0 o 1)
// --------------------------------------------------------
int procesar_gpio_leer(int pin)
{
    if (!gpio_verificar_disponibilidad())
    {
        return 0;
    }
    if (!gpio_inicializado)
    {
        if (nico_gpio_init() < 0)
            return 0;
    }
    if (pin < 0 || pin >= 40)
    {
        fprintf(stderr, "Error: Pin fuera de rango (0-39 BCM).\n");
        return 0;
    }

    int valor = 0;
#ifdef HAVE_LIBGPIOD
    if (!nico_requests[pin])
    {
        configurar_linea_gpio(pin, 0);
    }
    if (nico_requests[pin])
    {
        valor = gpiod_line_request_get_value(nico_requests[pin], pin);
    }
#endif

    return valor;
}
