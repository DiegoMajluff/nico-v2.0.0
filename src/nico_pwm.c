/*
 * Nico v2.0.0 - Intérprete Educativo de Scripting en Español
 * @file:         nico_pwm.c
 * @author:       Diego Alejandro Majluff (Diseño, Arquitectura y Supervisión)
 * @ai_assist:    Qwen (Alibaba Cloud) - Implementación, Debugging y Optimización
 * @license:      MIT / Personal Use (ver LICENSE)
 * @description:  Implementación de control PWM para Raspberry Pi. Permite
 *                generar señales PWM para control de motores, servos, LEDs
 *                con brillo variable y otros actuadores. Usa software PWM
 *                con timing preciso.
 */
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include "nico_pwm.h"
#include "nico_gpio.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#define MAX_PWM_CANALES 4
#define FRECUENCIA_MAX 500  // Hz

typedef struct {
    int pin;
    unsigned int frecuencia;
    int duty_cycle;
    pthread_t thread;
    int activo;
    int corriendo;
} CanalPWM;

static CanalPWM canales[MAX_PWM_CANALES];
static int pwm_inicializado = 0;
static pthread_mutex_t pwm_mutex = PTHREAD_MUTEX_INITIALIZER;

// Función del thread PWM con compensación de tiempo
static void* pwm_thread_func(void* arg) {
    CanalPWM* canal = (CanalPWM*)arg;
    struct timespec ts_inicio, ts_actual;
    long long tiempo_ejecucion_ns;
    
    while (canal->corriendo) {
        if (canal->frecuencia == 0 || canal->duty_cycle == 0) {
            procesar_gpio_estado_pin(canal->pin, 0);
            usleep(10000);
            continue;
        }
        
        // Calcular períodos en nanosegundos
        long long periodo_total_ns = 1000000000LL / canal->frecuencia;
        long long periodo_alto_ns = (periodo_total_ns * canal->duty_cycle) / 100;
        long long periodo_bajo_ns = periodo_total_ns - periodo_alto_ns;
        
        // Estado HIGH
        clock_gettime(CLOCK_MONOTONIC, &ts_inicio);
        procesar_gpio_estado_pin(canal->pin, 1);
        
        // Calcular tiempo que tomó ejecutar el cambio de estado
        clock_gettime(CLOCK_MONOTONIC, &ts_actual);
        tiempo_ejecucion_ns = (ts_actual.tv_sec - ts_inicio.tv_sec) * 1000000000LL + 
                              (ts_actual.tv_nsec - ts_inicio.tv_nsec);
        
        // Compensar el tiempo de ejecución
        long long sleep_alto_ns = periodo_alto_ns - tiempo_ejecucion_ns;
        if (sleep_alto_ns > 0) {
            struct timespec ts;
            ts.tv_sec = sleep_alto_ns / 1000000000;
            ts.tv_nsec = sleep_alto_ns % 1000000000;
            clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, NULL);
        }
        
        if (!canal->corriendo) break;
        
        // Estado LOW
        clock_gettime(CLOCK_MONOTONIC, &ts_inicio);
        procesar_gpio_estado_pin(canal->pin, 0);
        
        // Compensar tiempo de ejecución
        clock_gettime(CLOCK_MONOTONIC, &ts_actual);
        tiempo_ejecucion_ns = (ts_actual.tv_sec - ts_inicio.tv_sec) * 1000000000LL + 
                              (ts_actual.tv_nsec - ts_inicio.tv_nsec);
        
        long long sleep_bajo_ns = periodo_bajo_ns - tiempo_ejecucion_ns;
        if (sleep_bajo_ns > 0) {
            struct timespec ts;
            ts.tv_sec = sleep_bajo_ns / 1000000000;
            ts.tv_nsec = sleep_bajo_ns % 1000000000;
            clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, NULL);
        }
    }
    
    return NULL;
}


int pwm_init(void) {
    if (pwm_inicializado) return 0;
    
    for (int i = 0; i < MAX_PWM_CANALES; i++) {
        canales[i].pin = -1;
        canales[i].frecuencia = 0;
        canales[i].duty_cycle = 0;
        canales[i].activo = 0;
        canales[i].corriendo = 0;
    }
    
    pwm_inicializado = 1;
    return 0;
}

void pwm_cleanup(void) {
    if (!pwm_inicializado) return;
    
    pthread_mutex_lock(&pwm_mutex);
    
    for (int i = 0; i < MAX_PWM_CANALES; i++) {
        if (canales[i].activo) {
            canales[i].corriendo = 0;
            pthread_join(canales[i].thread, NULL);
            canales[i].activo = 0;
        }
    }
    
    pthread_mutex_unlock(&pwm_mutex);
    pwm_inicializado = 0;
}

int pwm_configurar(int pin, unsigned int frecuencia, int duty_cycle) {
    if (!pwm_inicializado) {
        if (pwm_init() < 0) return -1;
    }
    
    if (frecuencia > FRECUENCIA_MAX) {
        fprintf(stderr, "Error: Frecuencia máxima soportada: %d Hz\n", FRECUENCIA_MAX);
        return -1;
    }
    
    if (duty_cycle < 0 || duty_cycle > 100) {
        fprintf(stderr, "Error: Duty cycle debe estar entre 0 y 100\n");
        return -1;
    }
    
    pthread_mutex_lock(&pwm_mutex);
    
    // Buscar canal existente o libre
    int indice = -1;
    for (int i = 0; i < MAX_PWM_CANALES; i++) {
        if (canales[i].pin == pin) {
            indice = i;
            break;
        }
    }
    
    if (indice == -1) {
        // Buscar canal libre
        for (int i = 0; i < MAX_PWM_CANALES; i++) {
            if (!canales[i].activo) {
                indice = i;
                break;
            }
        }
    }
    
    if (indice == -1) {
        fprintf(stderr, "Error: Máximo %d canales PWM simultáneos\n", MAX_PWM_CANALES);
        pthread_mutex_unlock(&pwm_mutex);
        return -1;
    }
    
    // Si el canal ya estaba activo, detenerlo
    if (canales[indice].activo) {
        canales[indice].corriendo = 0;
        pthread_join(canales[indice].thread, NULL);
    }
    
    // Configurar nuevo canal
    canales[indice].pin = pin;
    canales[indice].frecuencia = frecuencia;
    canales[indice].duty_cycle = duty_cycle;
    canales[indice].activo = 1;
    canales[indice].corriendo = 1;
    
    // Configurar pin como salida
    procesar_gpio_configurar(pin, 1, 0);  // SALIDA, sin bias
    
    // Iniciar thread
    if (pthread_create(&canales[indice].thread, NULL, pwm_thread_func, &canales[indice]) != 0) {
        fprintf(stderr, "Error: No se pudo crear thread PWM\n");
        canales[indice].activo = 0;
        pthread_mutex_unlock(&pwm_mutex);
        return -1;
    }
    
    pthread_mutex_unlock(&pwm_mutex);
    return 0;
}

int pwm_detener(int pin) {
    if (!pwm_inicializado) return 0;
    
    pthread_mutex_lock(&pwm_mutex);
    
    for (int i = 0; i < MAX_PWM_CANALES; i++) {
        if (canales[i].pin == pin && canales[i].activo) {
            canales[i].corriendo = 0;
            pthread_join(canales[i].thread, NULL);
            canales[i].activo = 0;
            procesar_gpio_estado_pin(pin, 0);
            break;
        }
    }
    
    pthread_mutex_unlock(&pwm_mutex);
    return 0;
}
