#ifndef TIMERS_H
#define TIMERS_H

#include <stdio.h>
#include "driver/timer.h"

// Constantes del temporizador
#define TIMER_GROUP    TIMER_GROUP_0  // Grupo de temporizadores (0 o 1)
#define TIMER_INDEX    TIMER_0        // Índice del temporizador (0 o 1)


void init_timer(int timer_interval_sec);
void IRAM_ATTR timer_isr_callback(void *arg); //Callback para manejar las interrupciones del temporizador. arg - Puntero a argumentos (si se necesita, se puede pasar NULL).

#endif // TIMER_MANAGER_H