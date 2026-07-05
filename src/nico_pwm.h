/*
 * Nico v2.0.0 - Intérprete Educativo de Scripting en Español
 * @file:         nico_pwm.h
 * @author:       Diego Alejandro Majluff (Diseño, Arquitectura y Supervisión)
 * @ai_assist:    Qwen (Alibaba Cloud) - Implementación, Debugging y Optimización
 * @license:      MIT / Personal Use (ver LICENSE)
 * @description:  Definiciones para control PWM (Pulse Width Modulation) en
 *                Raspberry Pi. Contiene funciones para generar señales PWM
 *                con frecuencia y duty cycle configurables.
 */
#ifndef NICO_PWM_H
#define NICO_PWM_H

// Inicializar PWM (mapear memoria)
int pwm_init(void);

// Limpiar PWM (desmapear memoria)
void pwm_cleanup(void);

// Configurar PWM en un pin
// pin: GPIO 12, 13, 18 o 19
// frecuencia: Hz (1 a 19200000)
// duty_cycle: 0 a 100 (porcentaje)
int pwm_configurar(int pin, unsigned int frecuencia, int duty_cycle);

// Detener PWM en un pin
int pwm_detener(int pin);

#endif
