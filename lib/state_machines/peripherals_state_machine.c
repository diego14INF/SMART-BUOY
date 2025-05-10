
#include "peripherals_state_machine.h"
#include "siren_rgb.h"
#include "bluetooth_link.h"
#include "energy_control.h"
#include "driver/gpio.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"

#define BOTON_SOCORRO GPIO_NUM_1
#define BOTON_AYUDA GPIO_NUM_2
#define BOTON_SALGO_FUERA GPIO_NUM_3
#define BOTON_EMPAREJAMIENTO GPIO_NUM_4

static int64_t ultima_pulsacion[4] = {0};


static QueueHandle_t gpio_evt_queue = NULL;
static QueueHandle_t baliza_queue;

static estado_boya_t estado_actual = ESTADO_INICIAL;
static const char *TAG = "Peripherals Machine";


// void manejar_mensaje_entrante(const char *mensaje) {
//     if (strcmp(mensaje, "acercate") == 0) {
//         set_baliza_color(0, 0, 1); // Azul
//     } else if (strcmp(mensaje, "ayuda") == 0) {
//         baliza_sirena_bajo_bateria(); // Amarillo
//     } else if (strcmp(mensaje, "socorro") == 0) {
//         baliza_sirena_socorro(); // Rojo + sirena
//     } else if (strcmp(mensaje, "salgo_fuera") == 0) {
//         baliza_sirena_fuera_limites(); // Naranja
//     }
// }

// esp_err_t manejador_post_mensaje(httpd_req_t *req) {
//     char buf[32];
//     int ret = httpd_req_recv(req, buf, sizeof(buf));
//     if (ret > 0) {
//         buf[ret] = '\0';
//         ESP_LOGI(TAG, "Mensaje recibido: %s", buf);
//         manejar_mensaje_entrante(buf);
//         httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
//         return ESP_OK;
//     }
//     return ESP_FAIL;
// }

// void registrar_manejadores_http(httpd_handle_t server) {
//     httpd_uri_t mensaje_uri = {
//         .uri = "/mensaje",
//         .method = HTTP_POST,
//         .handler = manejador_post_mensaje,
//         .user_ctx = NULL
//     };
//     httpd_register_uri_handler(server, &mensaje_uri);
// }
bool boton_acercate, boton_ayuda, boton_socorro, boton_fuera, boton_emparejar;


static void tarea_baliza(void *arg) {
    evento_baliza_t evento;

    while (1) {
        if (xQueueReceive(baliza_queue, &evento, portMAX_DELAY)) {
            switch (evento) {
                case EVENTO_BALIZA_SOCORRO:
                    baliza_sirena_socorro();
                    break;
                case EVENTO_BALIZA_ESTADO_OK:
                    baliza_sirena_estado_ok();
                    break;
                case EVENTO_BALIZA_BAJA_BATERIA:
                    baliza_sirena_bajo_bateria();
                    break;
                case EVENTO_BALIZA_EMPAREJAMIENTO:
                    baliza_sirena_emparejamiento();
                    break;
                case EVENTO_BALIZA_TRANSMISION:
                    baliza_sirena_transmision_datos();
                    break;
                case EVENTO_BALIZA_ERROR:
                    baliza_sirena_error_sistema();
                    break;
            }
        }
    }
}

void procesar_botones(void) {
    boton_emparejar = gpio_get_level(BOTON_EMPAREJAMIENTO) == 1;
   // boton_acercate = gpio_get_level(BOTON_ACERCATE) == 1;
    boton_ayuda = gpio_get_level(BOTON_AYUDA) == 1;
    boton_socorro = gpio_get_level(BOTON_SOCORRO) == 1;
    boton_fuera = gpio_get_level(BOTON_SALGO_FUERA) == 1;
    
}
static void IRAM_ATTR gpio_isr_handler(void* arg) {
    boton_evento_t evt = (boton_evento_t)(uintptr_t)arg;
    int64_t ahora = esp_timer_get_time(); // Microsegundos

    if (ahora - ultima_pulsacion[evt] > 20000) { // 20 ms de margen
        ultima_pulsacion[evt] = ahora;
        xQueueSendFromISR(gpio_evt_queue, &evt, NULL);
    }
}

void peripherals_state_machine_init(void) {
    baliza_sirena_init();
    baliza_sirena_bajo_bateria(); //prueba
    
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_NEGEDGE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << BOTON_SOCORRO) |
                        (1ULL << BOTON_EMPAREJAMIENTO) |
                        (1ULL << BOTON_AYUDA) |
                        (1ULL << BOTON_SALGO_FUERA),
                        //(1ULL << BOTON_ACERCATE),
        .pull_down_en = 0,
        .pull_up_en = 1
    };
    gpio_config(&io_conf);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(BOTON_SOCORRO, gpio_isr_handler, (void*)EVENT_SOCORRO);
    gpio_isr_handler_add(BOTON_EMPAREJAMIENTO, gpio_isr_handler, (void*)EVENT_EMPAREJAR);
    gpio_isr_handler_add(BOTON_AYUDA, gpio_isr_handler, (void*)EVENT_AYUDA);
    gpio_isr_handler_add(BOTON_SALGO_FUERA, gpio_isr_handler, (void*)EVENT_FUERA);
    //gpio_isr_handler_add(BOTON_ACERCATE, gpio_isr_handler, (void*)EVENT_ACERCATE);

    gpio_evt_queue = xQueueCreate(10, sizeof(boton_evento_t));
    baliza_queue = xQueueCreate(5, sizeof(evento_baliza_t));

    xTaskCreate(tarea_baliza, "tarea_baliza", 2048, NULL, 5, NULL);
}

void peripherals_state_machine_run(void) {
    boton_evento_t evento;
    uint8_t bateria ;

    //La variable de estado se actualiza con la atencion de los eventos de las pulsaciones
    if (xQueueReceive(gpio_evt_queue, &evento, pdMS_TO_TICKS(100))) {
        switch (evento) {
            case EVENT_SOCORRO:
                estado_actual = ESTADO_SOCORRO;
                break;
            case EVENT_AYUDA:
                estado_actual = ESTADO_ENVIAR_MENSAJE;
                break;
            case EVENT_FUERA:
                estado_actual = ESTADO_ENVIAR_MENSAJE;
                break;
            case EVENT_EMPAREJAR:
                estado_actual = ESTADO_EMPAREJAMIENTO;
                break;
        }
    }


    switch (estado_actual) {
        case ESTADO_INICIAL:
            printf("ESTADO MÁQUINA PERFIERICOS: INICIAL------\n");
            evento_baliza_t evento_ok = EVENTO_BALIZA_ESTADO_OK; //Activacion del evento de baliza correspondiente a cada estado (De esta forma sigue funcionando sin ocupar la ejecucion ppal)
            xQueueSend(baliza_queue, &evento_ok, 0);
            break;

        case ESTADO_BAJO_BATERIA:
            printf("ESTADO MÁQUINA PERFIERICOS: Bajo bateria------\n");
            evento_baliza_t evento_bat = EVENTO_BALIZA_BAJA_BATERIA;
            xQueueSend(baliza_queue, &evento_bat, 0);

            
            if (bateria >= 30) estado_actual = ESTADO_INICIAL;
            break;

        case ESTADO_EMPAREJAMIENTO:
            printf("ESTADO MÁQUINA PERFIERICOS: Emparejamiento------\n");
            evento_baliza_t evento_emparejar = EVENTO_BALIZA_EMPAREJAMIENTO;
            xQueueSend(baliza_queue, &evento_emparejar, 0);
            estado_actual = ESTADO_INICIAL;
            break;

        case ESTADO_ENVIAR_MENSAJE:
            printf("ESTADO MÁQUINA PERFIERICOS: Enviar mensaje------\n");
            evento_baliza_t evento_envio = EVENTO_BALIZA_TRANSMISION;
            xQueueSend(baliza_queue, &evento_envio, 0);
            //vTaskDelay(pdMS_TO_TICKS(3000));
            estado_actual = ESTADO_INICIAL;
            break;

        case ESTADO_SOCORRO:
            printf("ESTADO MÁQUINA PERFIERICOS: Socorro------\n");
            evento_baliza_t evento = EVENTO_BALIZA_SOCORRO;
            xQueueSend(baliza_queue, &evento, 0);
            estado_actual = ESTADO_INICIAL;
            break;

        case ESTADO_ERROR:
            printf("ESTADO MÁQUINA PERFIERICOS: Error------\n");
             evento_baliza_t evento_error = EVENTO_BALIZA_ERROR;
            xQueueSend(baliza_queue, &evento_error, 0);
            estado_actual = ESTADO_INICIAL;
            break;
    }
}
