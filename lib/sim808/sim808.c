#include "sim808.h"
#include "driver/uart.h"
#include <string.h>
#include <stdio.h>

#define UART_NUM UART_NUM_1
#define TXD_PIN (17)
#define RXD_PIN (16)
#define BAUD_RATE 115200
#define GPS_PWR_ON_COMMAND "AT+CGNSPWR=1\r\n"
#define GPS_INF_COMMAND "AT+CGNSINF\r\n"
#define BATTERY_COMMAND "AT+CBC\r\n"
#define GPS_RESET_MODE_COMMAND "AT+CGPSRST=%d\r\n" //0:hot reset, 1:warm reset, 2:cold reset
#define GPS_STATUS_COMMAND "AT+CGPSSTATUS?\r\n" 
#define BUF_SIZE 1024
#define SMS_CONFIG_COMAND "AT+CMGF=1\r\n" // Configura para enviar SMS
#define TLF_CONFIG_COMMAND "AT+CMGS=\"+123456789\"\r\n" //Configura el número de destino


void sim808_send_command(const char *command) {
    uart_write_bytes(UART_NUM, command, strlen(command));
    printf("Enviando comando al SIM808: %s\n", command);
}

void sim808_read_response(char *buffer, size_t buffer_size) {
    int length = uart_read_bytes(UART_NUM, (uint8_t *)buffer, buffer_size - 1, 100 / portTICK_PERIOD_MS);
    buffer[length] = '\0';
}

// Inicializa el módulo SIM808 y UART
int sim808_init() {
    const uart_config_t uart_config = {
        .baud_rate = BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM, &uart_config);
    uart_set_pin(UART_NUM, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_NUM, BUF_SIZE * 2, 0, 0, NULL, 0);

    // Enciende el GPS
    sim808_send_command(GPS_PWR_ON_COMMAND);
    char response[64];
    sim808_read_response(response, sizeof(response));
    if (strstr(response, "OK") != NULL) {
        
        return 1;  // Inicialización exitosa
    } else {
        return 0;  // Error en la inicialización
    }
    printf("Respuesta a la inicialización: %s\n", response);
}

void sim808_gps_reset_mode(int mode) {
    char command[16];
    //EScribo los datos formateados (mode) en la cadena de caracteres sin exceder el tamaño del buffer (command)
    snprintf(command, sizeof(command), GPS_RESET_MODE_COMMAND, mode); 
    sim808_send_command(command);
    sim808_read_response(command, sizeof(command));
    printf("Respuesta de GPS reset: %s\n", command);
}

void sim808_gps_get_status() {
    sim808_send_command(GPS_STATUS_COMMAND);
    char response[128];
    sim808_read_response(response, sizeof(response));
    printf("Estado actual del GPS: %s\n", response);
}

// Obtiene los datos GPS
int sim808_get_gps_data(GPSData *data) {
    sim808_send_command(GPS_INF_COMMAND);
    char response[BUF_SIZE];
    sim808_read_response(response, BUF_SIZE);

    if (strstr(response, "+CGNSINF: ")) {
        sscanf(response, "+CGNSINF: %*d,%*d,%*f,%f,%f,%f,%f,%f", 
               &data->latitude, &data->longitude, &data->altitude, 
               &data->speed, &data->course);
        return 1;
    }
    return 0;
}

// Obtiene el voltaje de la batería
int sim808_get_battery_status(int *battery_voltage) {
    sim808_send_command(BATTERY_COMMAND);
    char response[BUF_SIZE];
    sim808_read_response(response, BUF_SIZE);

    if (strstr(response, "+CBC: ")) {
        sscanf(response, "+CBC: %*d,%*d,%d", battery_voltage); //Pendiente de corregir
        return 1;
        printf("Voltaje de la batería: %d mV\n", battery_voltage);
    }else {
        printf("No se pudo obtener el voltaje de la batería.\n");
    }
    return 0;
    
     
}

void sim808_send_data_over_gsm(const GPSData *gps_data) {
    char message[256];
    snprintf(message, sizeof(message),
             "Location: lat=%.6f, lon=%.6f, alt=%.2f, speed=%.2f, course=%.2f, battery=%d mV",
             gps_data->latitude, gps_data->longitude, gps_data->altitude,
             gps_data->speed, gps_data->course);

    // Enviar mensaje de datos por GSM
    sim808_send_command(SMS_CONFIG_COMAND); 
    sim808_send_command(TLF_CONFIG_COMMAND);  // Número de destino
    sim808_send_command(message);
    uart_write_bytes(UART_NUM, "\x1A", 1);  // Enviar CTRL+Z para enviar SMS
}