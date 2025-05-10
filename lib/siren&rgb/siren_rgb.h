#ifndef SIREN_RGB_CONTROL_H
#define SIREN_RGB_CONTROL_H

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "htcw_rmt_led_strip.h"

// GPIOs para el relé de la sirena y la baliza RGB
#define RELE_SIRENA_GPIO GPIO_NUM_15
#define RGB_RED_GPIO GPIO_NUM_35
#define RGB_GREEN_GPIO GPIO_NUM_36
#define RGB_BLUE_GPIO GPIO_NUM_37

#define LED_STRIP_NUM_LEDS 16
#define LED_STRIP_GPIO     GPIO_NUM_18  // Ajusta al pin usado para el anillo

//led_strip_handle_t led_strip;  // Variable global para el controlador



// Funciones de inicialización y control
void baliza_sirena_init(void);
void activar_sirena(void);
void desactivar_sirena(void);
void set_baliza_color(uint8_t red, uint8_t green, uint8_t blue);
void baliza_sirena_socorro(void);
void baliza_sirena_estado_ok(void);
void baliza_sirena_emparejamiento(void);
void baliza_sirena_bajo_bateria(void);
void baliza_sirena_error_sistema(void);
void baliza_sirena_transmision_datos(void);

#endif // SIREN_RGB_CONTROL_H