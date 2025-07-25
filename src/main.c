#include <stdio.h>
#include "sim808_gps.h"
#include <esp_rom_sys.h> 
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
#include "peripherals_state_machine.h"

QueueHandle_t uart_queue; // Declaración global
SemaphoreHandle_t sim808_semaphore; // Semáforo para sincronización entre máquinas de estado

void gps_task(void *pvParameters);
void gprs_task(void *pvParameters);
void peripherals_task(void *pvParameters);

void app_main(void) {
    // Inicialización del módulo GPS SIM808
    printf("Iniciando programa con SIM808...\n");
    if (sim808_init() == 0) {
        printf("Error al inicializar el GPS del SIM808.\n");
    }

    // Inicialización de máquinas de estado:
    gps_state_machine_init();
    gprs_state_machine_init();
    peripherals_state_machine_init();
    i2c_master_init();
    ina219_calibracion(0.02, 3.2);

    // Crear el semáforo ternario
    sim808_semaphore = xSemaphoreCreateBinary();
    if (sim808_semaphore == NULL) {
        printf("Error al crear el semáforo.\n");
        return;
    }

    // Inicializar el semáforo en estado disponible
    xSemaphoreGive(sim808_semaphore);

    // Crear las tareas para las máquinas de estado
    xTaskCreate(gps_task, "GPS Task", 16384, NULL, 2, NULL);
    xTaskCreate(gprs_task, "GPRS Task", 32768, NULL, 2, NULL);
    xTaskCreate(peripherals_task, "Peripherals Task", 16384, NULL, 2, NULL);

}

// Implementación de la tarea para la máquina de estado GPS
void gps_task(void *pvParameters) {
    while (1) {
        // Intentar tomar el semáforo
        if (xSemaphoreTake(sim808_semaphore, portMAX_DELAY)) {
            printf("-------->>>>>>GPS>>>>>>--------\n");
            // Ejecutar la máquina de estado GPS
            gps_state_machine_run();
            printf("Memoria restante en la pila de GPS Task: %u bytes\n", uxTaskGetStackHighWaterMark(NULL));
            //printf("Memoria libre en el heap: %lu bytes\n", esp_get_free_heap_size());
            printf("--------<<<<<<GPS<<<<<<--------\n");
            // Esperar antes de liberar el semáforo
            vTaskDelay(pdMS_TO_TICKS(2000)); // Esperar 100ms antes de liberar
            uart_flush(UART_NUM_1);
            xSemaphoreGive(sim808_semaphore); // Liberar el semáforo
        } else {
            printf("GPS Task: Esperando el semáforo.\n");
        }
        
        // Puede realizar otras actividades mientras espera el semáforo
        vTaskDelay(pdMS_TO_TICKS(5000)); // Esperar 1 segundo antes de reintentar
    }
}

// Implementación de la tarea para la máquina de estado GPRS
void gprs_task(void *pvParameters) {
    while (1) {
        // Intentar tomar el semáforo
        if (xSemaphoreTake(sim808_semaphore, portMAX_DELAY)) {
            printf("-------->>>>>>GPR(S)>>>>>>--------\n");
            // Ejecutar la máquina de estado GPRS
            gprs_state_machine_run();
            printf("Memoria restante en la pila de GPRS Task: %u bytes\n", uxTaskGetStackHighWaterMark(NULL));
            //printf("Memoria libre en el heap: %lu bytes\n", esp_get_free_heap_size());
            printf("--------<<<<<<GPR(S)<<<<<<--------\n");
            vTaskDelay(pdMS_TO_TICKS(4000)); // Esperar 100ms antes de liberar
            uart_flush(UART_NUM_1);
            xSemaphoreGive(sim808_semaphore); // Liberar el semáforo
        } else {
            printf("GPRS Task: Esperando el semáforo.\n");
        }
        // Puede realizar otras actividades mientras espera el semáforo
        vTaskDelay(pdMS_TO_TICKS(1000)); // Esperar 1 segundo antes de reintentar
    }
}

// Implementación de la tarea para la máquina de estado GPRS
void peripherals_task(void *pvParameters) {
    while (1) {
        // Intentar tomar el semáforo
        if (xSemaphoreTake(sim808_semaphore, portMAX_DELAY)) {
            printf("-------->>>>>>Peripherals>>>>>>--------\n");
            // Ejecutar la máquina de estado GPRS
            peripherals_state_machine_run();
            printf("Memoria restante en la pila de Periphericals Task: %u bytes\n", uxTaskGetStackHighWaterMark(NULL));
            //printf("Memoria libre en el heap: %lu bytes\n", esp_get_free_heap_size());
            printf("--------<<<<<<Peripherals<<<<<<--------\n");
            vTaskDelay(pdMS_TO_TICKS(1000)); // Esperar 100ms antes de liberar
            xSemaphoreGive(sim808_semaphore); // Liberar el semáforo
        } else {
            printf("Peripherals Task: Esperando el semáforo.\n");
        }
        // Puede realizar otras actividades mientras espera el semáforo
        vTaskDelay(pdMS_TO_TICKS(2000)); // Esperar 1 segundo antes de reintentar
    }
}

