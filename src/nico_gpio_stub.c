/*
 * Nico v2.0.0 - Intérprete Educativo de Scripting en Español
 * @file:         nico_gpio_stub.c
 * @author:       Diego Alejandro Majluff (Diseño, Arquitectura y Supervisión)
 * @ai_assist:    Qwen (Alibaba Cloud) - Implementación, Debugging y Optimización
 * @license:      MIT / Personal Use (ver LICENSE)
 * @description:  Implementación stub de GPIO para compilación en sistemas sin
 *                Raspberry Pi (Linux/Windows). Proporciona funciones dummy que
 *                permiten compilar y ejecutar código sin hardware real.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nico_gpio.h"

static int gpio_inicializado = 0;
static int modo_simulacion = 0;
static int valores_pines[40] = {0};
static int contador_leerpin = 0;

// Verificar si estamos en modo simulación
static void verificar_modo_simulacion(void)
{
    const char *env = getenv("NICO_GPIO_SIMULATE");
    modo_simulacion = (env && strcmp(env, "1") == 0);
}

int nico_gpio_init(void)
{
    verificar_modo_simulacion();

    // Inicializar todos los pines con valor 1 (PULLUP = alto = botón no presionado)
    for (int i = 0; i < 40; i++)
    {
        valores_pines[i] = 1;
    }

    contador_leerpin = 0;
    gpio_inicializado = 1;
    if (modo_simulacion)
    {
        printf("[GPIO SIM] Inicializado en modo simulación\n");
    }
    return 0;
}

void nico_gpio_cleanup(void)
{
    if (gpio_inicializado && modo_simulacion)
    {
        printf("[GPIO SIM] Cleanup\n");
    }
    gpio_inicializado = 0;
    contador_leerpin = 0;
}

int gpio_verificar_disponibilidad(void)
{
    verificar_modo_simulacion();
    return 1; // Siempre disponible en stub para testing
}

void procesar_gpio_configurar(int pin, int direccion, int bias)
{
    if (!gpio_inicializado)
    {
        nico_gpio_init();
    }

    if (modo_simulacion)
    {
        const char *dir_str = direccion ? "SALIDA" : "ENTRADA";
        const char *bias_str = "";
        if (!direccion)
        {
            if (bias == 1)
                bias_str = " PULLUP";
            else if (bias == 2)
                bias_str = " PULLDOWN";
        }
        printf("[GPIO SIM] CONFIGURARPIN: pin %d %s%s\n", pin, dir_str, bias_str);
    }
}

void procesar_gpio_estado_pin(int pin, int valor)
{
    if (!gpio_inicializado)
    {
        nico_gpio_init();
    }

    if (pin >= 0 && pin < 40)
    {
        valores_pines[pin] = valor;
    }

    if (modo_simulacion)
    {
        printf("[GPIO SIM] ESTADOPIN: pin %d = %s\n", pin, valor ? "SI" : "NO");
    }
}

int procesar_gpio_leer(int pin)
{
    if (!gpio_inicializado)
    {
        nico_gpio_init();
    }

    int valor = 1; // Por defecto: botón NO presionado (PULLUP = alto)

    contador_leerpin++;

    // Simular botón presionado solo en lecturas 11-15, luego vuelve a no presionado
    if (contador_leerpin >= 11 && contador_leerpin <= 15)
    {
        valor = 0; // Simular botón presionado
    }
    else if (contador_leerpin > 15)
    {
        // Resetear contador para permitir más ciclos de prueba
        contador_leerpin = 0;
    }

    if (modo_simulacion)
    {
        printf("[GPIO SIM] LEERPIN: pin %d = %d (lectura #%d)\n", pin, valor, contador_leerpin);
    }

    return valor;
}
