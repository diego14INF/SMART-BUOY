#include "gprs_state_machine.h"
#include <string.h>
#include "gsm_data.h"
#include "sim808_gprs.h"

// Configuración
#define GSM_BATCH_SIZE 1
#define GSM_MAX_RETRIES 5
#define MEMORY_SIZE 100

static State current_state;
static int retry_count;
static int processed_index; // Índice del último registro procesado
static char buffer_salida[2760];
static bool last_send_successful; // Indica si el último envío fue exitoso

// Variables globales
// Inicializa la máquina de estados
void gprs_state_machine_init(void) {
static State current_state = ESPERA;
static int retry_count = 0;
static int processed_index = 0; // Índice del último registro procesado
static bool last_send_successful = true; // Indica si el último envío fue exitoso
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
                current_state = ENVIAR_DATOS;
            } else {
                printf("Error al preparar el buffer.\n");
                current_state = ERROR;
            }
            break;
        

        case ENVIAR_DATOS: 
            printf("ESTADO MÁQUINA GPRS: Envio de datos.------\n");
            if (sim808_gprs_connect()) {
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