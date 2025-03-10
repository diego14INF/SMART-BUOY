#include <stdio.h>
#include "sim808_gps.h" // Archivo de encabezado que contiene las declaraciones de las funciones y estructuras
#include <esp_rom_sys.h>  // Necesario para `esp_rom_delay_us()`
#include "data_storage.h"
#include "sim808_gprs.h"
#include "gps_state_machine.h"
#include "timers.h"
#include "gprs_state_machine.h"
#include "gsm_data.h"
#include "i2c_com.h"
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

QueueHandle_t uart_queue; // Declaración global
SemaphoreHandle_t sim808_semaphore; // Semáforo para sincronización entre máquinas de estado

void gps_task(void *pvParameters);
void gprs_task(void *pvParameters);

void app_main(void) {
    // Inicialización del módulo GPS SIM808
    printf("Iniciando programa con SIM808...\n");
    if (sim808_init() == 0) {
        printf("Error al inicializar el GPS del SIM808.\n");
    }

    // Inicialización de máquinas de estado:
    gps_state_machine_init();
    gprs_state_machine_init();
    i2c_master_init();
    ina219_calibrate(0.02, 3.2);

    // Crear el semáforo binario
    sim808_semaphore = xSemaphoreCreateBinary();
    if (sim808_semaphore == NULL) {
        printf("Error al crear el semáforo.\n");
        return;
    }

    // Inicializar el semáforo en estado disponible
    xSemaphoreGive(sim808_semaphore);

    // Crear las tareas para las máquinas de estado
    xTaskCreate(gps_task, "GPS Task", 4096, NULL, 1, NULL);
    xTaskCreate(gprs_task, "GPRS Task", 4096, NULL, 2, NULL);

    // Ya no se requiere un bucle en `app_main` porque las tareas se ejecutan en paralelo
}

// Implementación de la tarea para la máquina de estado GPS
void gps_task(void *pvParameters) {
    while (1) {
        // Intentar tomar el semáforo
        if (xSemaphoreTake(sim808_semaphore, portMAX_DELAY)) {
            printf("GPS Task: Accediendo al módulo SIM808.\n");

            // Ejecutar la máquina de estado GPS
            gps_state_machine_run();

            printf("GPS Task: Liberando el módulo SIM808.\n");
            // Esperar antes de liberar el semáforo
            vTaskDelay(pdMS_TO_TICKS(1000)); // Esperar 100ms antes de liberar
            xSemaphoreGive(sim808_semaphore); // Liberar el semáforo
        } else {
            printf("GPS Task: Esperando el semáforo.\n");
        }

        // Puede realizar otras actividades mientras espera el semáforo
        vTaskDelay(pdMS_TO_TICKS(1000)); // Esperar 1 segundo antes de reintentar
    }
}

// Implementación de la tarea para la máquina de estado GPRS
void gprs_task(void *pvParameters) {
    while (1) {
        // Intentar tomar el semáforo
        if (xSemaphoreTake(sim808_semaphore, portMAX_DELAY)) {
            printf("GPRS Task: Accediendo al módulo SIM808.\n");

            // Ejecutar la máquina de estado GPRS
            gprs_state_machine_run();

            printf("GPRS Task: Liberando el módulo SIM808.\n");
            vTaskDelay(pdMS_TO_TICKS(1000)); // Esperar 100ms antes de liberar
            xSemaphoreGive(sim808_semaphore); // Liberar el semáforo
        } else {
            printf("GPRS Task: Esperando el semáforo.\n");
        }

        // Puede realizar otras actividades mientras espera el semáforo
        vTaskDelay(pdMS_TO_TICKS(1000)); // Esperar 1 segundo antes de reintentar
    }
}


// void app_main(void) {

//     // Inicialización del módulo GPS SIM808
//     printf("Iniciando programa con SIM808...\n");
//     if (sim808_init()==0){
//         printf("Error al inicializar el GPS del SIM808.\n");
//     }
//     //Inicialización de máquinas de estado:
    
//     gps_state_machine_init();
//     gprs_state_machine_init();
//     i2c_master_init();

//     ina219_calibrate(0.02,3.2);

    
//     // Ciclo principal
//     while (1) {
//         ina219_log_data();
//         //Corriendo máquina de estados del GPS
//         gps_state_machine_run();
        
//         //Corriendo máquina de estados del módulo GSM/GPRS
//         gprs_state_machine_run();

//         vTaskDelay(pdMS_TO_TICKS(5000));  // 5 segundos
//     }
// }