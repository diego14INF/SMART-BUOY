#include "gps_state_machine.h"
#include <stdio.h>
#include "sim808_gps.h"
#include "timers.h"
#include "data_storage.h"

// Configuración de la máquina de estados
#define MAX_GPS_RETRIES 5
#define MAX_ACQUISITION_RETRIES 3
#define GPS_COLD_RESET 2
#define GPS_WARM_RESET 1
#define GPS_HOT_RESET 0

// Tiempos de espera en milisegundos para cada tipo de reinicio
#define TIMER_COLD_RESET 5
#define TIMER_WARM_RESET 3
#define TIMER_HOT_RESET 2

// Estado actual y variables de control
static GPSState current_state = STATE_VERIFY_GPS;
static int gps_retry_count = 0;
static int acquisition_retry_count = 0;
static int gps_mode = 0;
int reset_type;
GPSData gps_data;


// Inicializa la máquina de estados
void gps_state_machine_init(void) {
    current_state = STATE_VERIFY_GPS;
    gps_retry_count = 0;
    acquisition_retry_count = 0;
    data_storage_init();
    printf("Máquina de estados del GPS inicializada.\n");
}

// Función para iniciar un temporizador según el tipo de reinicio
void start_gps_reset_timer(int reset_type) {
    int timer_duration = 0;
    switch (reset_type) {
        case GPS_COLD_RESET:
            timer_duration = TIMER_COLD_RESET;
            break;
        case GPS_WARM_RESET:
            timer_duration = TIMER_WARM_RESET;
            break;
        case GPS_HOT_RESET:
            timer_duration = TIMER_HOT_RESET;
            break;
    }
    printf("Temporizador iniciado para reinicio %d: %d s.\n", reset_type, timer_duration);
    init_timer(timer_duration);
    
}

// Ejecuta la lógica de la máquina de estados
void gps_state_machine_run(void) {
    switch (current_state) {
        case STATE_VERIFY_GPS:
            printf("ESTADO MÁQUINA GPS: Verificación de GPS------\n");
            gps_mode = sim808_gps_get_status();
            if (gps_mode==1) {
                current_state = STATE_ACQUIRE_DATA;
            } else {
                printf("GPS no está OK. Intentando reiniciar...\n");
                gps_retry_count++;
                if (gps_retry_count <= MAX_GPS_RETRIES) {
                    // Cambiar el tipo de reinicio en función del estado
                    if (gps_mode % 4 == 0) {
                        reset_type=GPS_COLD_RESET;
                    } else if (gps_mode % 3 == 0){
                        reset_type=GPS_WARM_RESET;
                    } else if (gps_mode % 2 == 0){
                        reset_type=GPS_HOT_RESET;
                    }
                    sim808_gps_reset_mode(reset_type);
                    start_gps_reset_timer(reset_type); // Inicia el temporizador
                    current_state=STATE_WAIT_TIMER;
                } else {
                    printf("Error crítico: no se pudo verificar el GPS tras %d intentos.\n", MAX_GPS_RETRIES);
                    current_state = STATE_ACQUIRE_DATA; //Ahora digo que pase a adquisicion de datos porque no tengo respuesta buena del estado del gps
                }
            }
            break;

        case STATE_ACQUIRE_DATA:
            printf("ESTADO MÁQUINA GPS: Adquisición de datos------\n");
            if (sim808_get_gps_data(&gps_data)&&sim808_get_battery_status(&gps_data)){
                printf("Datos adquiridos correctamente.\n");
                current_state = STATE_STORE_DATA;
            } else {
                acquisition_retry_count++;
                if (acquisition_retry_count > MAX_ACQUISITION_RETRIES) {
                    printf("Error: no se pudieron adquirir los datos tras %d intentos.\n", MAX_ACQUISITION_RETRIES);
                    current_state = STATE_ERROR;
                }
            }
            break;

        case STATE_STORE_DATA:
            printf("ESTADO MÁQUINA GPS: Almacenamiento de datos------\n");
            if (data_storage_save(&gps_data)) {
                printf("Total de entradas a la memoria: %d\n",  data_storage_get_count());
                printf("Datos almacenados con éxito. Reiniciando flujo.\n");
                current_state = STATE_VERIFY_GPS; // Reinicia el flujo
                gps_retry_count = 0;
                acquisition_retry_count = 0;
            } else {
                printf("Error al almacenar los datos.\n");
                current_state = STATE_ERROR;
            }
            break;

        case STATE_WAIT_TIMER:
           printf("ESTADO MÁQUINA GPS: Esperando reinicio del módulo GPS------\n");
           if (is_timer_finished()) {
              printf("Temporizador de reinicio completado.\n");
              current_state = STATE_VERIFY_GPS; // Regresa a verificar el GPS
            } else {
               vTaskDelay(10 / portTICK_PERIOD_MS); // Da tiempo al sistema operativo
            }
            break;

        case STATE_ERROR:
            printf("ESTADO MÁQUINA GPS: ERROR. Revise el sistema.------\n");
            // Aquí se puede añadir lógica para reiniciar la máquina o notificar el error
            current_state = STATE_VERIFY_GPS;
            break;

        default:
            printf("ESTADO MÁQUINA GPS: Desconocido. Reiniciando máquina de estados.------\n");
            current_state = STATE_VERIFY_GPS;
            break;
    }
}