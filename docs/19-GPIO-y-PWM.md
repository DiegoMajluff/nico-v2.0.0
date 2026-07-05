# 🔌 GPIO y PWM en Raspberry Pi

Nico incluye soporte nativo para control de pines GPIO y PWM en Raspberry Pi, usando `libgpiod` para acceso seguro al hardware.

## 🎯 Comandos Disponibles

### GPIO Digital

| Comando | Sintaxis | Descripción |
|---------|----------|-------------|
| `CONFIGURARPIN` | `CONFIGURARPIN(pin, modo, bias)` | Configura un pin como ENTRADA o SALIDA |
| `ESTADOPIN` | `ESTADOPIN(pin, $estado)` | Asigna un estado a un pin (SI/NO o 0/1) |
| `LEERPIN` | `LEERPIN(pin, EN $destino)` | Alias de ESTADOPIN (compatibilidad) |

### PWM

| Comando | Sintaxis | Descripción |
|---------|----------|-------------|
| `GENERARPWM` | `GENERARPWM(pin, frecuencia, duty_cycle)` | Genera señal PWM (máx 500Hz) |
| `DETENERPWM` | `DETENERPWM(pin)` | Detiene la señal PWM |

## 📋 Parámetros Detallados

### CONFIGURARPIN(pin, modo, bias)

| Parámetro | Tipo | Valores | Descripción |
|-----------|------|---------|-------------|
| `pin` | ENTERA | 0-40 | Número de pin BCM (GPIO) |
| `modo` | Palabra clave | `ENTRADA` o `SALIDA` | Dirección del pin |
| `bias` | Palabra clave | `PULLUP` o `PULLDOWN` (opcional) | Resistencia interna. **Por defecto: PULLUP** |

**Modos:**
- `ENTRADA`: El pin lee señales externas (botones, sensores)
- `SALIDA`: El pin genera señales (LEDs, relés, buzzers)

**Bias (resistencia interna):**
- `PULLUP`: Conecta resistencia a 3.3V (lectura normal = 1, botón presionado = 0) **[Por defecto]**
- `PULLDOWN`: Conecta resistencia a GND (lectura normal = 0, botón presionado = 1)

> 💡 Si omitís el parámetro `bias`, Nico usa `PULLUP` automáticamente. Para salidas digitales, el bias no afecta el funcionamiento.

### LEERPIN(pin, EN $destino)

| Parámetro | Tipo | Descripción |
|-----------|------|-------------|
| `pin` | ENTERA | Número de pin BCM a leer |
| `$destino` | VARIABLE ENTERA | Recibe 0 (LOW) o 1 (HIGH) |

### ESTADOPIN(pin, $estado)

| Parámetro | Tipo | Descripción |
|-----------|------|-------------|
| `pin` | ENTERA | Número de pin BCM a escribir |
| `$estado` | VARIABLE ENTERA | Escribe 0 (NO) o 1 (SI) |


### GENERARPWM(pin, frecuencia, duty_cycle)

| Parámetro | Tipo | Rango | Descripción |
|-----------|------|-------|-------------|
| `pin` | ENTERA | 0-40 | Número de pin BCM |
| `frecuencia` | DECIMAL | 1-500 | Frecuencia en Hz (máximo 500Hz) |
| `duty_cycle` | DECIMAL | 0-100 | Porcentaje de tiempo en HIGH |

### DETENERPWM(pin)

| Parámetro | Tipo | Descripción |
|-----------|------|-------------|
| `pin` | ENTERA | Pin donde detener PWM |

## ⚠️ Limitaciones de Frecuencia

**Frecuencia máxima: 500Hz**

Nico está optimizado para aplicaciones de baja frecuencia:
- ✅ **Servos estándar**: 50Hz
- ✅ **LEDs (dimmer)**: 50-200Hz
- ✅ **Indicadores visuales**: 1-10Hz
- ❌ **Audio/buzzers**: No soportado (requiere >1kHz)
- ❌ **Motores DC rápidos**: No recomendado

> 💡 **¿Por qué 500Hz?**: Frecuencias más altas causan inestabilidad en el temporizador por software de Nico. Para servos y LEDs, 500Hz es más que suficiente.

## 🔌 Pines Recomendados

### Pines PWM (hardware)

| Pin BCM | Función PWM | Notas |
|---------|-------------|-------|
| 18 | PWM0 | **Recomendado** - PWM por hardware |
| 12 | PWM0 | Alternativa |
| 13 | PWM1 | Alternativa |
| 19 | PWM1 | Alternativa |

### Pines GPIO (cualquier función)

Todos los pines GPIO (2-27) pueden usarse como entrada/salida digital. Los pines especiales son:

| Pin BCM | Función Especial | Notas |
|---------|------------------|-------|
| 0, 1 | I2C | Usados por EEPROM, no recomendados |
| 2, 3 | I2C | Pull-up interno fijo |
| 7-11 | SPI | Usados por EEPROM, no recomendados |
| 14, 15 | UART | Consola serial, evitar uso general |
| 18, 12, 13, 19 | PWM | Soportan PWM por hardware |

## 🧪 Ejemplos de Uso

### LED Parpadeante (Blink)

    PROGRAMA Blink
        VARIABLE ENTERA $estado = 0
    BLOQUE PRINCIPAL
        CONFIGURARPIN(18, SALIDA)
        
        MIENTRAS(VERDADERO) HACER
            SI($estado IGUAL 0) ENTONCES
                ASIGNAR EN $estado = 1
            SINO
                ASIGNAR EN $estado = 0
            FIN SI
            
            ESCRIBIRPIN(18, $estado)
            ESCRIBIR("LED: $estado") SALTO
            ESPERAR(500, MILISEGUNDOS)
        FIN MIENTRAS
    FIN PRINCIPAL
    FINAL

### Botón con PULLUP (por defecto)

    PROGRAMA LeerBoton
        VARIABLE ENTERA $estado = 0
    BLOQUE PRINCIPAL
        // Botón conectado entre GPIO 17 y GND
        // PULLUP por defecto: estado normal = 1, presionado = 0
        CONFIGURARPIN(17, ENTRADA)
        
        ESCRIBIR("Presioná el botón (Ctrl+C para salir)") SALTO
        
        MIENTRAS(VERDADERO) HACER
            ESTADOPIN(17, $estado)
            SI($estado IGUAL 0) ENTONCES
                ESCRIBIR("Botón presionado!") SALTO
            SINO
                ESCRIBIR("Botón suelto") SALTO
            FIN SI
            ESPERAR(100, MILISEGUNDOS)
        FIN MIENTRAS
    FIN PRINCIPAL
    FINAL

### Botón con PULLDOWN (explícito)

    PROGRAMA LeerBotonPulldown
        VARIABLE ENTERA $estado = 0
    BLOQUE PRINCIPAL
        // Botón conectado entre GPIO 27 y 3.3V
        // PULLDOWN: estado normal = 0, presionado = 1
        CONFIGURARPIN(27, ENTRADA, PULLDOWN)
        
        MIENTRAS(VERDADERO) HACER
            ESTADOPIN(27, $estado)
            SI($estado IGUAL 1) ENTONCES
                ESCRIBIR("Botón presionado!") SALTO
            FIN SI
            ESPERAR(100, MILISEGUNDOS)
        FIN MIENTRAS
    FIN PRINCIPAL
    FINAL

### Dimmer LED (100Hz)

    PROGRAMA DimmerLED
        VARIABLE ENTERA $brillo = 0
    BLOQUE PRINCIPAL
        // LED con PWM a 100Hz
        
        PARA($brillo DESDE 0 HASTA 100 PASO 5) HACER
            GENERARPWM(18, 100, $brillo)
            ESCRIBIR("Brillo: $brillo%") SALTO
            ESPERAR(100, MILISEGUNDOS)
        FIN PARA
        
        ESPERAR(2, SEGUNDOS)
        
        PARA($brillo DESDE 100 HASTA 0 PASO -5) HACER
            GENERARPWM(18, 100, $brillo)
            ESCRIBIR("Brillo: $brillo%") SALTO
            ESPERAR(100, MILISEGUNDOS)
        FIN PARA
        
        DETENERPWM(18)
    FIN PRINCIPAL
    FINAL

## ⚠️ Notas Técnicas

### Validaciones
- **Pin**: Debe ser un número entero válido (0-40)
- **Frecuencia PWM**: Debe estar entre 1 y 500 Hz
- **Duty cycle**: Debe estar entre 0 y 100 (porcentaje)

### Consideraciones de Hardware
- **Voltaje**: GPIO opera a 3.3V. Para cargas mayores, usá transistores.
- **Corriente máxima**: 16mA por pin. Usá MOSFET para cargas mayores.
- **PWM hardware**: Más preciso, limitado a pines 12, 13, 18, 19.
- **PWM software**: Funciona en cualquier pin, pero con jitter.

## 🔧 Troubleshooting

| Problema | Causa | Solución |
|----------|-------|----------|
| Error "Frecuencia máxima: 500 Hz" | Frecuencia > 500Hz | Reducí a 500Hz o menos |
| PWM no funciona | Pin incorrecto | Usá pines 12, 13, 18 o 19 |
| Servo no se mueve | Frecuencia incorrecta | Usá 50Hz |
| LED parpadea | Frecuencia muy baja | Aumentá a 50Hz+ |
| Error "error al configurar" | Pin en uso | Liberá el pin con DETENERPWM |
| Lectura inestable | Sin pull-up/down | Agregá PULLUP o PULLDOWN explícito |

## 🖥️ Compatibilidad

| Plataforma | Soporte GPIO | Soporte PWM | Notas |
|------------|--------------|-------------|-------|
| **Raspberry Pi** | ✅ Completo | ✅ Completo | Requiere `libgpiod` |
| **Linux** | ⚪ Stub (no-op) | ⚪ Stub (no-op) | Comandos válidos, sin efecto |
| **Windows** | ⚪ Stub (no-op) | ⚪ Stub (no-op) | Comandos válidos, sin efecto |

> 💡 En plataformas sin GPIO, los comandos son no-ops seguros (no generan error).

## 🎯 Buenas Prácticas

    // ✅ Configurar pin antes de usar
    CONFIGURARPIN(18, SALIDA)
    
    // ✅ Usar PULLUP/PULLDOWN para entradas
    CONFIGURARPIN(17, ENTRADA, PULLUP)
    CONFIGURARPIN(27, ENTRADA, PULLDOWN)
    
    // ✅ Siempre detener PWM al finalizar
    DETENERPWM(18)
    
    // ✅ Validar rango de duty cycle
    SI($duty MENOR 0 O $duty MAYOR 100) ENTONCES
        ESCRIBIR("Error: duty debe estar entre 0 y 100") SALTO
    SINO
        GENERARPWM(18, 100, $duty)
    FIN SI
    
    // ❌ Evitar frecuencias > 500Hz
    // GENERARPWM(18, 1000, 50)  // ← Error
    
    // ❌ Evitar pin flotante sin bias en entradas sensibles
    // CONFIGURARPIN(17, ENTRADA)  // ← OK, usa PULLUP por defecto

## 📊 Cálculos Útiles

### Servo: Ángulo a Duty Cycle
    Duty Cycle = 2.5 + (Ángulo / 180) × 10
    Ejemplos:
    - 0° → 2.5%
    - 90° → 7.5%
    - 180° → 12.5%

### Periodo y Frecuencia
    Periodo (segundos) = 1 / Frecuencia (Hz)
    Ejemplo: 50Hz → Periodo = 0.02 segundos (20ms)

---

📝 **Documentación validada con Nico v2.0.0 en Raspberry Pi 4/5**
