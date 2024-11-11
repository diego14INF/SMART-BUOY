#include "gps_sim808.h"
#include "driver/uart.h"
#include <string.h>
#include <stdio.h>
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define UART_NUM UART_NUM_1  // UART 1 para comunicación con SIM808
#define TXD_PIN (17)
#define RXD_PIN (16)
#define BAUD_RATE 9600
#define BUF_SIZE           1024
#define VBAT_COMMAND       "AT+CBC\r\n"

void print_command(const char *command) {
    printf("Se envió al SIM808: %s\n", command);
}

int sim808_init() {
    // Configura UART
    const uart_config_t uart_config = {
        .baud_rate = BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM, &uart_config);
    uart_set_pin(UART_NUM, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_NUM, 2048, 0, 0, NULL, 0);

    // Configuración inicial del SIM808
    sim808_send_command("AT\r\n");          // Prueba conexión

      // Lee la respuesta para ver si es "OK"
    char response[64];
    sim808_read_response(response, sizeof(response));
    if (strstr(response, "OK") != NULL) {
        return 1;  // Inicialización exitosa
    } else {
        return 0;  // Error en la inicialización
    }
    sim808_send_command("AT+CGNSPWR=1\r\n"); // Activa GPS en el SIM808
}

void sim808_send_command(const char *command) {
    uart_write_bytes(UART_NUM, command, strlen(command));
    print_command(command);
}

void sim808_read_response(char *buffer, size_t buffer_size) {
    int length = uart_read_bytes(UART_NUM, (uint8_t *)buffer, buffer_size - 1, 100 / portTICK_PERIOD_MS);
    if (length > 0) {
        buffer[length] = '\0';  // Termina el string
    } else {
        buffer[0] = '\0';  // No hay datos
        printf("No se recibió nada en el buffer de recepción.\n");
    }
}

// Función para enviar el comando AT+CBC y recibir el voltaje de la batería
void monitorizar_vbat() {
    // Enviar el comando AT+CBC
    uart_write_bytes(UART_NUM, (const char *)VBAT_COMMAND, strlen(VBAT_COMMAND));
    printf("Enviando comando: %s\n", VBAT_COMMAND);

    // Buffer para leer la respuesta
    char data[BUF_SIZE];
    int length = uart_read_bytes(UART_NUM, (uint8_t *)data, BUF_SIZE - 1, 100 / portTICK_PERIOD_MS);

    // Asegurarse de que haya datos leídos
    if (length > 0) {
        data[length] = '\0';  // Añadir el terminador nulo para convertir a cadena de caracteres
        printf("Respuesta del módulo: %s\n", data);

        // Buscar el valor del voltaje en la respuesta
        char *start = strstr(data, "+CBC: ");
        if (start) {
            start += 7;  // Desplazar el puntero después de "+CBC: "
            int battery_voltage = atoi(start);  // Convertir la parte de la respuesta que contiene el voltaje a entero
            printf("Voltaje de la batería: %d mV\n", battery_voltage);
        } else {
            printf("No se pudo obtener el voltaje de la batería.\n");
        }
    } else {
        printf("No se recibió respuesta o hubo un error al leer.\n");
    }
}
