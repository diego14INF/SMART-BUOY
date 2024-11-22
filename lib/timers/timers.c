#include "timers.h"

static gptimer_handle_t gptimer = NULL; // Manejador del temporizador, lo inicializo a NULL para indicar que no hay ningun temporizador negado
static volatile bool timer_finished = false; // Bandera para indicar que el temporizador alcanzó 0 ( Volatile hace que el compilador no optimice el acesso a esta variable, que puede cambiar repentinamente dentro de una interrupcion)

// Callback de interrupción del temporizador
bool IRAM_ATTR timer_callback(gptimer_handle_t timer, const gptimer_alarm_event_data_t *event_data, void *user_ctx) {
    // Marca el temporizador como finalizado
    timer_finished = true;
    printf("¡Temporizador alcanzó 0!\n");

    return false; // Indica que no se debe recargar automaticamente la alarma (auto-reload)
}

// Configura e inicia el temporizador
void init_timer(int timer_interval_sec) {
    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_APB, // Fuente de reloj
        .direction = GPTIMER_COUNT_DOWN, // Cuenta regresiva
        .resolution_hz = 1000000        // Resolución en Hz (1 MHz, 1 tick = 1 µs)
    };

    // Crea el temporizador
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));

    // Configura la alarma
    gptimer_alarm_config_t alarm_config = {
        .reload_count = timer_interval_sec * 1000000, // Valor de recarga en microsegundos
        .alarm_count = 0,                            // Alarma al llegar a 0
        .flags.auto_reload_on_alarm = false           // Habilitar auto-reload
    };
    ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer, &alarm_config));

    // Registra el callback
    gptimer_event_callbacks_t cbs = {
        .on_alarm = timer_callback // Callback cuando se activa la alarma
    };
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer, &cbs, NULL));

    // Inicia el temporizador
    ESP_ERROR_CHECK(gptimer_start(gptimer));
    printf("Temporizador iniciado por %d segundos.\n", timer_interval_sec);
}

// Verifica si el temporizador alcanzó 0
bool is_timer_finished() {
    return timer_finished;
}