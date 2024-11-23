#include "sim808_gprs.h"
#include "sim808_gps.h"
#include "gsm_data.h"
#include <stdio.h>
#include <string.h>
#include "driver/uart.h"

#define UART_NUM UART_NUM_1
// Comandos AT para conexión GPRS
#define AT_GPRS_INIT "AT+CGATT=1\r\n"       // Conectar a la red GPRS
#define AT_GPRS_APN "AT+CGDCONT=1,\"IP\",\"" APN "\"\r\n" // Configuración APN
#define AT_GPRS_STATUS "AT+CGACT?\r\n"      // Comprobar estado de la conexión GPRS
#define AT_TCP_CONNECT "AT+CIICR\r\n"       // Activar GPRS
#define AT_TCP_SOCKET "AT+CIPSTART=\"TCP\",\"" GPRS_SERVER "\",\"" #define GPRS_PORT "\"\r\n" // Conectar a servidor TCP

// Conectar a la red GPRS
int sim808_gprs_connect(void) {
    char response[64];
    // Enviar comandos para activar GPRS
    sim808_send_command(AT_GPRS_INIT);
    sim808_read_response(response, sizeof(response));

    sim808_send_command(AT_GPRS_APN);
    sim808_read_response(response, sizeof(response));

    sim808_send_command(AT_GPRS_STATUS);
    sim808_read_response(response, sizeof(response));

    sim808_send_command(AT_TCP_CONNECT);
    if (strstr(response, "OK")) {
        return 1;  // Conexión GPRS exitosa
    }
    return 0;  // Error al conectar
}

// Enviar los datos por GPRS
void sim808_gprs_send_data(char *shipping_buffer) {
    char response[256];
    // Iniciar la transmisión de datos
    sim808_send_command("AT+CIPSEND\r\n");
    sim808_send_command(shipping_buffer);
    uart_write_bytes(UART_NUM, "\x1A", 1);  // CTRL+Z para enviar

    // Verificar si el envío fue exitoso
    sim808_read_response(response, sizeof(response));
    printf("Datos enviados: %s\n", shipping_buffer);
}

// Desconectar la sesión GPRS
int sim808_gprs_disconnect(void) {
    sim808_send_command("AT+CIPSHUT\r\n");  // Cerrar la conexión GPRS
    sim808_read_response(NULL, 0);
    return 1;
}