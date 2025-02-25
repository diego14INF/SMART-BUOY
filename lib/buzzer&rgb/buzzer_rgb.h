#ifndef BUZZER_RGB_CONTROL_H
#define BUZZER_RGB_CONTROL_H

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"

// GPIOs para el zumbador y la baliza RGB
#define BUZZER_GPIO GPIO_NUM_21
#define RGB_RED_GPIO GPIO_NUM_35
#define RGB_GREEN_GPIO GPIO_NUM_36
#define RGB_BLUE_GPIO GPIO_NUM_37

// Canales y timers para el PWM
#define BUZZER_LEDC_TIMER LEDC_TIMER_0
#define BUZZER_LEDC_MODE LEDC_LOW_SPEED_MODE
#define BUZZER_LEDC_CHANNEL LEDC_CHANNEL_0
#define BUZZER_FREQUENCY 2000 // Frecuencia inicial en Hz

// Funciones de inicialización y control
void baliza_zumbador_init(void);
void activar_zumbador(int frecuencia);
void desactivar_zumbador(void);
void set_baliza_color(uint8_t red, uint8_t green, uint8_t blue);
void baliza_zumbador_socorro(void);
void baliza_zumbador_estado_ok(void);
void baliza_zumbador_fuera_limites(void);
void baliza_zumbador_bajo_bateria(void);
void baliza_zumbador_error_sistema(void);
void baliza_zumbador_transmision_datos(void);
void activar_zumbador_tono(int frecuencia, int duracion_ms);

#endif // BALIZA_ZUMBADOR_H