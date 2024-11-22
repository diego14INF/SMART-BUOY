#ifndef TIMERS_H
#define TIMERS_H

#include <stdio.h>
#include "driver/gptimer.h"
#include <stdbool.h>
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"

void init_timer(int timer_interval_sec);
bool is_timer_finished();
bool timer_callback(gptimer_handle_t timer, const gptimer_alarm_event_data_t *event_data, void *user_ctx); //Callback para manejar las interrupciones del temporizador. arg - Puntero a argumentos (si se necesita, se puede pasar NULL).

#endif // TIMER_MANAGER_H