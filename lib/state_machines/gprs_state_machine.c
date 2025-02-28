#include "gprs_state_machine.h"
#include <string.h>
#include "gsm_data.h"
#include "sim808_gprs.h"
#include "freertos/FreeRTOS.h" // Incluir para FreeRTOS
#include "freertos/task.h" // Incluir para vTaskDelay y pdMS_TO_TICKS
#include "timers.h"
#include <stdbool.h>

// Configuración
#define GSM_BATCH_SIZE 1
#define GSM_MAX_RETRIES 5
#define MEMORY_SIZE 100
#define MAX_ATTEMPTS 3

static State current_state;
static GPRSConnectionSubstate gprs_connection_substate;

static int retry_count;
static int processed_index; // Índice del último registro procesado
static char buffer_salida[2760];
static bool last_send_successful; // Indica si el último envío fue exitoso
bool timer_finished;

// Variables globales
// Inicializa la máquina de estados
void gprs_state_machine_init(void) {
static State current_state = ESPERA;
static GPRSConnectionSubstate gprs_connection_substate = GPRS_SUBSTATE_SIM;
static int retry_count = 0;
static int processed_index = 0; // Índice del último registro procesado
static bool last_send_successful = true; // Indica si el último envío fue exitoso
}

// Función de reintento genérica
int retry_function(int (*func)(void), const char *func_name) {
    int retry_delay = 4000; // 4 segundos de retraso inicial
    const int max_timeout = 20000; // 20 segundos de tiempo máximo
    int total_delay = 0;

    while (total_delay <= max_timeout) {
        int result = func();
        if (result == 1) {
            printf("PASSED: %s after %d seconds.\n", func_name, total_delay / 1000);
            return 1;
        } else {
            printf("FAILED: %s. Retrying in %d seconds...\n", func_name, retry_delay / 1000);
            total_delay += retry_delay;
            vTaskDelay(pdMS_TO_TICKS(retry_delay));
            retry_delay += 4000; // Incrementar el retraso en 4 segundos
            if (retry_delay > 20000) {
                retry_delay = 20000; // Mantener el retraso máximo en 20 segundos
            }
        }
    }
    printf("FAILED: %s after %d seconds.\n", func_name, max_timeout / 1000);
    return 0; // Falló después del tiempo máximo
}

void gprs_state_machine_run(void) {
    switch (current_state) {
        case ESPERA: 
            printf("ESTADO MÁQUINA GPRS: Espera lote de registros------\n");
            int count = data_storage_get_count();
            if (count >= GSM_BATCH_SIZE) {
                current_state = PREPARAR_BUFFER;
            }
            break;
        
        case PREPARAR_BUFFER: 
            printf("ESTADO MÁQUINA GPRS: Preparación de buffer de salida------\n");
            int prepared_size = gsm_prepare_batch(buffer_salida, sizeof(buffer_salida), processed_index);
            if (prepared_size > 0) {
                printf("Buffer preparado con %d bytes.\n", prepared_size);
                current_state = PREPARAR_RED;
            } else {
                printf("Error al preparar el buffer.\n");
                current_state = ERROR;
            }
            break;

            case PREPARAR_RED:
            printf("ESTADO MÁQUINA GPRS: Preparación de red de datos.------\n");
            switch (gprs_connection_substate) {
                case GPRS_SUBSTATE_SIM:
                    if (retry_function(sim808_config_sim, "sim808_config_sim")) {
                        gprs_connection_substate = GPRS_SUBSTATE_INIT;
                    } else {
                        gprs_connection_substate = GPRS_SUBSTATE_ERROR;
                    }
                    break;

                case GPRS_SUBSTATE_INIT:
                    if (retry_function(sim808_gprs_connect_init, "sim808_gprs_connect_init")) {
                        gprs_connection_substate = GPRS_SUBSTATE_APN;
                    } else {
                        gprs_connection_substate = GPRS_SUBSTATE_ERROR;
                    }
                    break;

                case GPRS_SUBSTATE_APN:
                    if (sim808_check_apn_present()){
                        gprs_connection_substate = GPRS_SUBSTATE_ACTIVATE;

                    }else{
                        if (retry_function(sim808_gprs_connect_apn, "sim808_gprs_connect_apn")) {
                          gprs_connection_substate = GPRS_SUBSTATE_ACTIVATE;
                        } else {
                          gprs_connection_substate = GPRS_SUBSTATE_ERROR;
                        }
                    }
                    break;

                case GPRS_SUBSTATE_ACTIVATE:
                    if (retry_function(sim808_gprs_activate_data, "sim808_gprs_activate_data")) {
                        gprs_connection_substate = GPRS_SUBSTATE_PPP;
                    } else {
                        gprs_connection_substate = GPRS_SUBSTATE_ERROR;
                    }
                    break;

                case GPRS_SUBSTATE_PPP:
                    if (sim808_check_apn_present()){
                       gprs_connection_substate = GPRS_SUBSTATE_IP;

                    }else{
                       if (retry_function(sim808_gprs_establish_ppp, "sim808_gprs_establish_ppp")) {
                           gprs_connection_substate = GPRS_SUBSTATE_IP;
                        } else {
                            gprs_connection_substate = GPRS_SUBSTATE_ERROR;
                        }
                    }
                    break;

                case GPRS_SUBSTATE_IP:
                    if (retry_function(sim808_gprs_get_ip, "sim808_gprs_get_ip")) {
                        gprs_connection_substate = GPRS_SUBSTATE_TCP_CONNECT;
                    } else {
                        gprs_connection_substate = GPRS_SUBSTATE_ERROR;
                    }
                    break;

                case GPRS_SUBSTATE_TCP_CONNECT:
                    if (retry_function(sim808_gprs_tcp_connect, "sim808_gprs_tcp_connect")) {
                        gprs_connection_substate = GPRS_SUBSTATE_CONNECTED;
                    } else {
                        gprs_connection_substate = GPRS_SUBSTATE_ERROR;
                    }
                    break;

                case GPRS_SUBSTATE_ERROR:
                    printf("Error en la conexión GPRS. Intentando reiniciar...............\n");
                    sim808_full_reset();
                    init_timer(10); // Iniciar temporizador de 10 segundos
                    gprs_connection_substate = GPRS_SUBSTATE_RESET_WAIT; // Transición al subestado de espera
                    break;
        
                case GPRS_SUBSTATE_RESET_WAIT:
                    if (is_timer_finished()) {
                        timer_finished = false; // Resetear la bandera del temporizador
                        gprs_connection_substate = GPRS_SUBSTATE_SIM; // Volver al subestado inicial
                        printf("Reinicio completo. Continuando con la conexión.\n");
                    } else {
                        printf("Esperando %d segundos para el reinicio...\n", 10);
                        vTaskDelay(pdMS_TO_TICKS(1000)); // Esperar 1 segundo y verificar de nuevo
                    }
                    break;
        

                case GPRS_SUBSTATE_CONNECTED:
                    current_state = COMPROBACION_RED; // Transición al siguiente estado
                    gprs_connection_substate = GPRS_SUBSTATE_INIT;
                    break;
            }
            break;

        case COMPROBACION_RED:
            printf("ESTADO MÁQUINA GPRS: Comprobando la red de datos.------\n");
            if (retry_function(sim808_check_network_status, "sim808_check_network_status")) {
                current_state = ENVIAR_DATOS;
            } else {
                current_state = PREPARAR_RED;
            }
            break;

        case ENVIAR_DATOS: 
            printf("ESTADO MÁQUINA GPRS: Envio de datos.------\n");
            if (sim808_check_network_status()) {  //Provisional
                sim808_gprs_send_data(&buffer_salida);
                current_state = CONFIRMAR_ENVIO;
            } else {
                printf("Error al conectar al GPRS.\n");
                current_state = ERROR;
            }
            break;

        case CONFIRMAR_ENVIO: {
            printf("ESTADO MÁQUINA GPRS: Confirmanción de envio------\n");
            if (last_send_successful) {
                printf("Datos enviados correctamente.\n");
                retry_count = 0;
                processed_index += GSM_BATCH_SIZE; // Avanzamos 20 posiciones
                current_state = ESPERA;

                // Si llegamos al límite, reiniciamos el índice
                if (processed_index >= MEMORY_SIZE) {
                    processed_index = MEMORY_SIZE - GSM_BATCH_SIZE;
                }
            } else {
                printf("Error en el envío. Reintentando...\n");
                retry_count++;
                if (retry_count >= GSM_MAX_RETRIES) {
                    printf("Máximo de reintentos alcanzado. Abortando envío.\n");
                    current_state = ERROR;
                } else {
                    current_state = ENVIAR_DATOS;
                }
            }
            break;
        }

        case ERROR: {
            printf("ESTADO MÁQUINA GPRS: Error. Reiniciando...-----\n");
            retry_count = 0;
            last_send_successful = false;
            current_state = ESPERA;
            break;
        }
    }
}