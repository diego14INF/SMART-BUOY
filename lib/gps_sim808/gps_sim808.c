#include "gps_sim808.h"
#include "driver/uart.h"
#include <string.h>

#define UART_NUM UART_NUM_1  // UART 1 para comunicación con SIM808
#define TXD_PIN (17)
#define RXD_PIN (16)
#define BAUD_RATE 115200

void sim808_init() {
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
    sim808_send_command("AT+CGNSPWR=1\r\n"); // Activa GPS en el SIM808
}

void sim808_send_command(const char *command) {
    uart_write_bytes(UART_NUM, command, strlen(command));
}

void sim808_read_response(char *buffer, size_t buffer_size) {
    int length = uart_read_bytes(UART_NUM, (uint8_t *)buffer, buffer_size - 1, 100 / portTICK_PERIOD_MS);
    if (length > 0) {
        buffer[length] = '\0';  // Termina el string
    } else {
        buffer[0] = '\0';  // No hay datos
    }
}