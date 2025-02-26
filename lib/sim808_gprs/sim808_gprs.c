#include "sim808_gprs.h"
#include "sim808_gps.h"
#include "gsm_data.h"
#include <stdio.h>
#include <string.h>
#include "driver/uart.h"

#define UART_NUM UART_NUM_1
// Comandos AT para conexión GPRS
#define AT_GPRS_INIT "AT+CGATT=1\r\n"       // Conectar a la red GPRS
#define AT_GPRS_CTRL_SIGNAL "AT+CSQ\r\n"    //Para comprobar valores de la señal gsm, los valores deben entre 10 y 30 son buenos
#define AT_GPRS_APN "AT+CGDCONT=1,\"IP\",\"" APN "\"\r\n" // Configuración APN
#define AT_GPRS_STATUS "AT+CGACT?\r\n"      // Comprobar estado de la conexión GPRS
#define AT_TCP_CONNECT "AT+CIICR\r\n"       // Activar GPRS
#define AT_TCP_SOCKET "AT+CIPSTART=\"TCP\",\"" GPRS_SERVER "\",\"" #define GPRS_PORT "\"\r\n" // Conectar a servidor TCP


//Preparar tarjeta SIM
int sim808_config_sim(void){
    char response[254];
    //Paso 0.0: Activar todas las funcionalidades
    sim808_send_command("AT+CFUN=1\r\n");
    sim808_read_response(response, sizeof(response));

     // Paso 0.1: Verificar el estado de la tarjeta SIM
    sim808_send_command("AT+CPIN?\r\n");
    sim808_read_response(response, sizeof(response));
    if (!strstr(response, "+CPIN: READY")) {
        // Introducir el PIN si la tarjeta lo requiere
        sim808_send_command("AT+CPIN=\"8495\"\r\n"); 
        sim808_read_response(response, sizeof(response));
        if (!strstr(response, "OK")) {
            printf("*******Error: No se pudo desbloquear la tarjeta SIM.*******\n");
            return 0;
        }
    } 
    return 1;
}

int sim808_check_network_status(){
    char response[254];
    int signal_strength = 0;
    int registration_status = 0;
    int network_ok = 0;
    int cfun_ok = 0;
    int cfun_status=0;

    printf("\n----------------------------------------\n");
    printf("| Parámetro     | Valor               |\n");
    printf("----------------------------------------\n");

    // Comprobar la señal GSM (AT+CSQ)
    sim808_send_command("AT+CSQ\r\n");
    sim808_read_response(response, sizeof(response));
    printf("| Señal GSM     | %s\n", response);

    // Extraer el valor de la señal GSM
    if (sscanf(response, "+CSQ: %d,", &signal_strength) == 1) {
        if (signal_strength >= 10 && signal_strength <= 30) {
            network_ok++;
        }
    }

    // Comprobar el estado del registro GSM (AT+CREG?)
    sim808_send_command("AT+CREG?\r\n");
    sim808_read_response(response, sizeof(response));
    printf("| Registro GSM  | %s\n", response);

    // Extraer el estado de registro
    if (sscanf(response, "+CREG: %*d,%d", &registration_status) == 1) {
        if (registration_status == 1 || registration_status == 5) {
            network_ok++;
        }
    }

    // Comprobar si la SIM está lista (AT+CPIN?)
    sim808_send_command("AT+CPIN?\r\n");
    sim808_read_response(response, sizeof(response));
    printf("| Estado SIM    | %s\n", response);

    // Verificar si la respuesta contiene "READY"
    if (strstr(response, "READY") != NULL) {
        network_ok++;
    }

    // Asegurarse de que el módulo está en modo funcional completo (AT+CFUN?)
    sim808_send_command("AT+CFUN?\r\n");
    sim808_read_response(response, sizeof(response));
    printf("Estado del módulo: %s\n", response);

   // Extraer el estado de CFUN
   if (sscanf(response, "+CFUN: %d", &cfun_status) == 1) {
     if (cfun_status == 1) {
        cfun_ok = 1;
      }
    }
 
    printf("----------------------------------------\n");

     // Retornar 1 solo si todas las condiciones se cumplen
     return (network_ok == 3 && cfun_ok == 1) ? 1 : 0;
}


// Conectar a la red GPRS
int sim808_gprs_connect(void) {
    char response[128];
    // Enviar comandos para activar GPRS

    // Paso 1: Conectar a GPRS
    sim808_send_command(AT_GPRS_INIT);
    sim808_read_response(response, sizeof(response));
      if (!strstr(response, "OK")) {
        printf("*******Error: No se pudo conectar a la red GPRS.*******\n");
        return 0;
    }

    // Paso 2: Configurar APN
    sim808_send_command(AT_GPRS_APN);
    sim808_read_response(response, sizeof(response));
       if (!strstr(response, "OK")) {
        printf("*******Error: Configuración APN fallida.*******\n");
        return 0;
    }

    
    // Paso 3: Activar conexión de datos
    sim808_send_command("AT+CGACT=1,1\r\n");
    sim808_read_response(response, sizeof(response));
    if (!strstr(response, "OK")) {
        printf("*******Error: Activación de conexión de datos fallida.*******\n");
        return 0;
    }


    // Paso 4: Establecer enlace PPP
    sim808_send_command("AT+CIICR\r\n");
    sim808_read_response(response, sizeof(response));
    if (!strstr(response, "OK")) {
        printf("*******Error: Enlace PPP fallido.*******\n");
        return 0;
    }

    // Paso 5: Obtener dirección IP
    sim808_send_command("AT+CIFSR\r\n");
    sim808_read_response(response, sizeof(response));
    if (strstr(response, "ERROR")) {
        printf("*******Error: No se pudo obtener dirección IP.*******\n");
        return 0;
    }

    //sim808_send_command(AT_TCP_CONNECT);

    printf("Conexión GPRS establecida correctamente. IP: %s\n", response);
    return 1;
}

// Enviar los datos por GPRS
void sim808_gprs_send_data(char *shipping_buffer) {
    char response[256];
    //char http_request[512];
    // Preparar solicitud HTTP
    // snprintf(http_request, sizeof(http_request),
    //          "POST /upload-data HTTP/1.1\r\n"
    //          "Host: <IP_DEL_SERVIDOR>:3000\r\n"
    //          "Content-Type: application/json\r\n"
    //          "Content-Length: %d\r\n"
    //          "\r\n"
    //          "%s",
    //          strlen(shipping_buffer), shipping_buffer);

    // Iniciar la transmisión de datos
    sim808_send_command("AT+CIPSEND\r\n");
    //sim808_send_command(http_request);
    //Recuerda que tengo preparar el shipping_buffer en formato json!!!!!
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