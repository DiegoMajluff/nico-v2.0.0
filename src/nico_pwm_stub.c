/*
 * Nico v2.0.0 - Intérprete Educativo de Scripting en Español
 * @file:         nico_pwm_stub.c
 * @author:       Diego Alejandro Majluff (Diseño, Arquitectura y Supervisión)
 * @ai_assist:    Qwen (Alibaba Cloud) - Implementación, Debugging y Optimización
 * @license:      MIT / Personal Use (ver LICENSE)
 * @description:  Implementación stub de PWM para compilación en sistemas sin
 *                Raspberry Pi. Proporciona funciones dummy que permiten
 *                compilar y ejecutar código sin hardware real.
 */
#include <stdio.h>
#include "nico_pwm.h"

int pwm_init(void) {
    return 0;
}

void pwm_cleanup(void) {
}

int pwm_configurar(int pin, unsigned int frecuencia, int duty_cycle) {
    printf("[PWM SIM] Pin %d, Frecuencia: %u Hz, Duty: %d%%\n", pin, frecuencia, duty_cycle);
    return 0;
}

int pwm_detener(int pin) {
    printf("[PWM SIM] Deteniendo pin %d\n", pin);
    return 0;
}
