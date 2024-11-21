#include "timers.h"

// Handler de interrupción
// void IRAM_ATTR timer_isr_callback(void *arg) {
//     // Limpia la interrupción del temporizador
//     timer_group_clr_intr_status_in_isr(TIMER_GROUP, TIMER_INDEX);

//     // Reactiva el temporizador (si es necesario)
//     timer_group_enable_alarm_in_isr(TIMER_GROUP, TIMER_INDEX);

//     // Realiza alguna acción
//     printf("¡Temporizador alcanzó 0!\n");
// }

// Configura el temporizador
void init_timer(int timer_interval_sec) {
    timer_config_t config = {
        .alarm_en = TIMER_ALARM_EN,           // Habilitar alarma
        .counter_en = TIMER_PAUSE,           // Inicialmente pausado
        .intr_type = TIMER_INTR_LEVEL,       // Interrupción de nivel
        .counter_dir = TIMER_COUNT_DOWN,     // Cuenta regresiva
        .auto_reload = true,                 // Recargar automáticamente
        .divider = 80                        // Divide frecuencia de 80 MHz por 80 (1 MHz, 1 tick = 1 µs)
    };
    timer_init(TIMER_GROUP, TIMER_INDEX, &config);

    // Carga el valor inicial del temporizador
    timer_set_counter_value(TIMER_GROUP, TIMER_INDEX, timer_interval_sec * 1000000); // En microsegundos

    // Configura la alarma
   // timer_set_alarm_value(TIMER_GROUP, TIMER_INDEX, 0); // Alarma al llegar a 0
   // timer_enable_intr(TIMER_GROUP, TIMER_INDEX);        // Habilita interrupción

    // Registra la interrupción y el callback
  //  timer_isr_callback_add(TIMER_GROUP, TIMER_INDEX, timer_isr_callback, NULL, 0);

    // Inicia el temporizador
    timer_start(TIMER_GROUP, TIMER_INDEX);
}

bool is_timer_finished() {
    uint64_t counter_value = 0;
    timer_get_counter_value(TIMER_GROUP, TIMER_INDEX, &counter_value);

    // Si el contador está en 0, el temporizador terminó
    return counter_value <= 0;
}