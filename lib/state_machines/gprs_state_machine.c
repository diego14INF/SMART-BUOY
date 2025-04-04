#include "timers.h"
#include "gprs_state_machine.h"
#include <string.h>
#include "gsm_data.h"
#include "sim808_gprs.h"
#include "freertos/FreeRTOS.h" // Incluir para FreeRTOS
#include "freertos/task.h" // Incluir para vTaskDelay y pdMS_TO_TICKS
#include <stdbool.h>

// Configuración
#define GSM_BATCH_SIZE 5
#define GSM_MAX_RETRIES 3
#define MEMORY_SIZE 100
#define MAX_ATTEMPTS 3

static State current_state;
static GPRSConnectionSubstate gprs_connection_substate;
static HTTPState http_state;

static int retry_count;
static int processed_index; // Índice del último registro procesado
static char buffer_salida[2760];
static bool last_send_successful; // Indica si el último envío fue exitoso
bool timer_finished;
static int network_check_attempts;

// Variables globales
// Inicializa la máquina de estados
void gprs_state_machine_init(void) {
 current_state = ESPERA;
 gprs_connection_substate = GPRS_SUBSTATE_SIM;
 http_state = HTTP_STATE_INIT; // Comenzar en el estado inicial
//static int retry_count = 0;
//static int processed_index = 0; // Índice del último registro procesado
//static bool last_send_successful = true; // Indica si el último envío fue exitoso
//static int network_check_attempts = 0;// Definir un contador global o dentro de la máquina de estado (según el diseño actual)
}

// Función de reintento genérica
int retry_function(int (*func)(void), const char *func_name) {
    int retry_delay = 2000; // 2 segundos de retraso inicial
    const int max_timeout = 8000; // 8 segundos de tiempo máximo
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
            retry_delay += 2000; // Incrementar el retraso en 4 segundos
            if (retry_delay > 8000) {
                retry_delay = 8000; // Mantener el retraso máximo en 20 segundos
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
                   if (sim808_check_gprs_attachment()){
                        gprs_connection_substate = GPRS_SUBSTATE_APN;
                   }else{  
                      if (retry_function(sim808_gprs_connect_init, "sim808_gprs_connect_init")) {
                          gprs_connection_substate = GPRS_SUBSTATE_APN;
                      } else {
                          gprs_connection_substate = GPRS_SUBSTATE_ERROR;
                      }
                   }
                  break;

                case GPRS_SUBSTATE_APN:
                    //if (sim808_check_apn_present()){
                     //   gprs_connection_substate = GPRS_SUBSTATE_ACTIVATE;

                    //}else{
                        if (retry_function(sim808_gprs_connect_apn, "sim808_gprs_connect_apn")) {
                          gprs_connection_substate = GPRS_SUBSTATE_ACTIVATE;
                        } else {
                          gprs_connection_substate = GPRS_SUBSTATE_ERROR;
                        }
                    //}
                    break;

                case GPRS_SUBSTATE_ACTIVATE: 
                    if (retry_function(sim808_gprs_wireless_activate, "sim808_gprs_wireless_activate")) {
                        gprs_connection_substate = GPRS_SUBSTATE_IP;
                    } else {
                        gprs_connection_substate = GPRS_SUBSTATE_ERROR;
                    }
                    break;

                case GPRS_SUBSTATE_PPP: //******NO ES NECESARIO PARA CONEXION TCP */
                    if (sim808_check_ppp_status()==6){  
                       gprs_connection_substate = GPRS_SUBSTATE_CONNECTED;

                    }else{
                       if (retry_function(sim808_gprs_establish_ppp, "sim808_gprs_establish_ppp")) {
                           gprs_connection_substate = GPRS_SUBSTATE_TCP_CONNECT;
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
                    network_check_attempts++;
                    if (network_check_attempts < 3) {
                    sim808_gprs_disconnect(); //Matamos la conexión GPRS
                    init_timer(5); // Iniciar temporizador de 5 segundos
                    gprs_connection_substate = GPRS_SUBSTATE_RESET_WAIT; // Transición al subestado de espera
                    } else {
                        current_state = ERROR;
                        network_check_attempts=0;
                    }
                    break;
        
                case GPRS_SUBSTATE_RESET_WAIT:
                    if (is_timer_finished()) {
                        timer_finished = false; // Resetear la bandera del temporizador
                        gprs_connection_substate = GPRS_SUBSTATE_SIM; // Volver al subestado inicial
                        printf("Reinicio de enlace. Conectando de nuevo.\n");
                    } else {
                        printf("Esperando %d segundos para el reinicio...\n", 5);
                        vTaskDelay(pdMS_TO_TICKS(1000)); // Esperar 1 segundo y verificar de nuevo
                    }
                    break;

                case GPRS_SUBSTATE_CONNECTED:
                    current_state = ENVIAR_DATOS; // Transición al siguiente estado
                    //gprs_connection_substate = GPRS_SUBSTATE_SIM;
                    break;
            }
            break;

        case COMPROBACION_RED:
            printf("ESTADO MÁQUINA GPRS: Comprobando la red de datos.-------------------------------------\n");
            // Incrementar el contador de intentos
            network_check_attempts++;
            if (network_check_attempts > 5) {
               printf("Se alcanzó el número máximo de intentos de comprobación de red. Transición al estado de error.\n");
               gprs_connection_substate = GPRS_SUBSTATE_ERROR;
               current_state = PREPARAR_RED;
               network_check_attempts = 0; // Reiniciar el contador después de ingresar al estado de error
            break;
            }
            // Verificar el estado de conexión PPP mediante sim808_check_ppp_status()
            switch (sim808_check_ppp_status()) {
                case 1: // IP INITIAL
                    printf("Estado: IP INITIAL. Preparando configuración del APN.\n");
                    current_state = PREPARAR_RED;
                    gprs_connection_substate = GPRS_SUBSTATE_APN;
                    break;

                case 2: // IP START
                    printf("Estado: IP START. Configurando conexión GPRS.\n");
                    current_state = PREPARAR_RED;
                    gprs_connection_substate = GPRS_SUBSTATE_INIT;
                    break;

                case 3: // IP CONFIG
                    printf("Estado: IP CONFIG. Configuración de red en progreso.\n");
                    current_state = PREPARAR_RED;
                    gprs_connection_substate = GPRS_SUBSTATE_ACTIVATE;
                    break;

                case 4: // IP GPRSACT
                    printf("Estado: IP GPRSACT. Contexto PDP activado.\n");
                    current_state = PREPARAR_RED;
                    gprs_connection_substate = GPRS_SUBSTATE_PPP;
                    break;

                case 5: // IP STATUS
                    printf("Estado: IP STATUS. Listo para iniciar la conexión TCP.\n");
                    current_state = PREPARAR_RED;
                    gprs_connection_substate = GPRS_SUBSTATE_TCP_CONNECT;
                    break;

                case 6: // CONNECT OK
                    printf("Estado: CONNECT OK. Conexión TCP establecida.\n");
                    current_state = ENVIAR_DATOS;
                    break;

                case 7: // PDP DEACT
                    printf("Estado: PDP DEACT. Contexto PDP desactivado. Reintentando...\n");
                    current_state = PREPARAR_RED;
                    gprs_connection_substate = GPRS_SUBSTATE_ACTIVATE;
                    break;

                case 8: // TCP CLOSED
                    printf("Estado: TCP CLOSED. Conexión TCP cerrada.\n");
                    current_state = PREPARAR_RED;
                    gprs_connection_substate = GPRS_SUBSTATE_TCP_CONNECT;
                    break;

                case 9: // CONNECT FAIL
                    printf("Estado: CONNECT FAIL. Fallo al conectar. Reiniciando módulo...\n");
                    current_state = PREPARAR_RED;
                    gprs_connection_substate = GPRS_SUBSTATE_ERROR;
                    break;

                case 10: // TCP CONNECTING
                    printf("Estado: TCP CONNECTING. Estableciendo conexión TCP...\n");
                    // Quedarse en este estado y esperar a que cambie
                    current_state = COMPROBACION_RED;
                    break;

                case -1: // Error o estado desconocido
                    printf("Estado desconocido. Reiniciando el proceso de conexión.\n");
                    current_state = PREPARAR_RED;
                    gprs_connection_substate = GPRS_SUBSTATE_SIM;
                    break;
            }
            break;

        case ENVIAR_DATOS: 
            printf("ESTADO MÁQUINA GPRS: Envio de datos.-------------------------------------\n");
            //if (sim808_check_ppp_status()==6) {  
                if(sim808_gprs_send_data(buffer_salida)){
                    
                    last_send_successful=true;
                    current_state = CONFIRMAR_ENVIO;
                }else {
                printf("Error en el envío.\n");
                current_state = COMPROBACION_RED;
                }
            break;
        

        case CONFIRMAR_ENVIO: {
            printf("ESTADO MÁQUINA GPRS: Confirmanción de envio------------------------------\n");
            if (last_send_successful) {
                printf("Datos enviados correctamente.\n");
                retry_count = 0;
                processed_index += GSM_BATCH_SIZE; // Avanzamos X posiciones
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
            sim808_full_reset();
            init_timer(10); // Iniciar temporizador de 10 segundos
            retry_count = 0;
            last_send_successful = false;
            current_state = STATE_WAIT_TIMER;
            break;
        }

        case STATE_WAIT_TIMER:{
            if (is_timer_finished()) {
                timer_finished = false; // Resetear la bandera del temporizador
                current_state = ESPERA; // Volver al subestado inicial
                printf("Reinicio completo.\n");
            } else {
                printf("Esperando %d segundos para el reinicio...\n", 10);
                vTaskDelay(pdMS_TO_TICKS(1000)); // Esperar 1 segundo y verificar de nuevo
            }
            break;
        }
    }
}