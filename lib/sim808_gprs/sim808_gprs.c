#include "sim808_gprs.h"
#include "sim808_gps.h"
#include "gsm_data.h"
#include <stdio.h>
#include <string.h>
#include "driver/uart.h"

#define UART_NUM UART_NUM_1

int sim808_full_reset(void) {
    char response[256];
    // Enviar el comando AT+CFUN=1,1 para reiniciar el módulo
    sim808_send_command("AT+CFUN=1,1\r\n");
    sim808_wait_for_response(response, sizeof(response), 10000); // Espera hasta 5s
    if (strstr(response, "OK") != NULL) { 
        return 1;  // Reinicio exitoso
    } else {
        return 0;  // Error en el reinicio
    
    }
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
    sim808_wait_for_response(response, sizeof(response), 10000); // Espera hasta 5s
    printf("| Señal GSM     | %s\n", response);

    // Extraer el valor de la señal GSM
    if (sscanf(response, "+CSQ: %d,", &signal_strength) == 1) {
        if (signal_strength >= 10 && signal_strength <= 30) {
            network_ok++;
        }
    }

    // Comprobar el estado del registro GSM (AT+CREG?)
    sim808_send_command("AT+CREG?\r\n");
    sim808_wait_for_response(response, sizeof(response), 10000); // Espera hasta 5s
    printf("| Registro GSM  | %s\n", response);

    // Extraer el estado de registro
    if (sscanf(response, "+CREG: %*d,%d", &registration_status) == 1) {
        if (registration_status == 1 || registration_status == 5) {
            network_ok++;
        }
    }

    // Comprobar si la SIM está lista (AT+CPIN?)
    sim808_send_command("AT+CPIN?\r\n");
    sim808_wait_for_response(response, sizeof(response), 10000); // Espera hasta 5s
    printf("| Estado SIM    | %s\n", response);

    // Verificar si la respuesta contiene "READY"
    if (strstr(response, "READY") != NULL) {
        network_ok++;
    }

    // Asegurarse de que el módulo está en modo funcional completo (AT+CFUN?)
    sim808_send_command("AT+CFUN?\r\n");
    sim808_wait_for_response(response, sizeof(response), 10000); // Espera hasta 5s
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

// Función para comprobar si el APN está presente
int sim808_check_apn_present(void) {
    char response[256];
    sim808_send_command("AT+CGDCONT?\r\n");
    sim808_wait_for_response(response, sizeof(response), 5000); // Ajusta el tiempo de espera si es necesario

    if (strstr(response, APN)) { 
        printf("APN ya configurado.\n");
        return 1; // APN presente
    } else {
        printf("APN no configurado.\n");
        return 0; // APN no presente
    }
}

int sim808_check_ppp_status(void) {
    char response[256];

    // Enviar el comando AT+CIPSTATUS
    sim808_send_command("AT+CIPSTATUS\r\n");
    sim808_wait_for_response(response, sizeof(response), 5000); // Ajusta el tiempo de espera si es necesario

    if (strstr(response, "IP INITIAL") != NULL) {
        printf("Estado PPP: IP INITIAL - Servicios IP iniciados pero no configurados.\n");
        return 1;
    } else if (strstr(response, "IP START") != NULL) {
        printf("Estado PPP: IP START - Listo para configurar la conexión GPRS.\n");
        return 2;
    } else if (strstr(response, "IP CONFIG") != NULL) {
        printf("Estado PPP: IP CONFIG - Configuración de red en curso.\n");
        return 3;
    } else if (strstr(response, "IP GPRSACT") != NULL) {
        printf("Estado PPP: IP GPRSACT - Contexto PDP activado.\n");
        return 4;
    } else if (strstr(response, "IP STATUS") != NULL) {
        printf("Estado PPP: IP STATUS - Listo para iniciar una conexión TCP/UDP.\n");
        return 5;
    } else if (strstr(response, "CONNECT OK") != NULL) {
        printf("Estado PPP: CONNECT OK - Conexión TCP/UDP establecida.\n");
        return 6;
    } else if (strstr(response, "PDP DEACT") != NULL) {
        printf("Estado PPP: PDP DEACT - Contexto PDP desactivado.\n");
        return 7;
    } else if (strstr(response, "TCP CLOSED") != NULL) {
        printf("Estado PPP: TCP CLOSED - Conexión TCP cerrada.\n");
        return 8;
    } else if (strstr(response, "CONNECT FAIL") != NULL) {
        printf("Estado PPP: CONNECT FAIL - Error al intentar conectar.\n");
        return 9;
    } else if (strstr(response, "TCP CONNECTING") != NULL) {
        printf("Estado PPP: CONNECTING... - Estableciendo conexión.\n");
        return 10;
    } else {
        printf("Estado PPP: Desconocido - Respuesta: %s\n", response);
        return -1; // Código para estado desconocido o error
    }
}


//Paso 0: Preparar tarjeta SIM
int sim808_config_sim(void){
    char response[254];
    //Paso 0.0: Activar todas las funcionalidades
    sim808_send_command("AT+CFUN=1\r\n");
    sim808_wait_for_response(response, sizeof(response), 10000); // Espera hasta 5s

     // Paso 0.1: Verificar el estado de la tarjeta SIM
    sim808_send_command("AT+CPIN?\r\n");
    sim808_wait_for_response(response, sizeof(response), 10000); // Espera hasta 5s
    if (!strstr(response, "+CPIN: READY")) {
        // Introducir el PIN si la tarjeta lo requiere
        sim808_send_command("AT+CPIN=\"8495\"\r\n"); 
        sim808_wait_for_response(response, sizeof(response), 10000); // Espera hasta 5s
        if (!strstr(response, "OK")) {
            printf("*******Error: No se pudo desbloquear la tarjeta SIM.*******\n");
            return 0;
        }
    } 
    return 1;
}

//Paso 1: Conectar a GPRS//nO SIRVE DE NADA ESTO CON ESE COMANDO, ES INCOMPLETO
int sim808_gprs_connect_init(void) {
    char response[128];
    sim808_send_command(AT_GPRS_INIT);
    sim808_wait_for_response(response, sizeof(response), 10000);
    if (!strstr(response, "OK")) {
        printf("*******Error: No se pudo conectar a la red GPRS.*******\n");
        return 0;
    }
    return 1;
}

// Paso 2: Configurar APN
int sim808_gprs_connect_apn(void) {
    char response[128];
    sim808_send_command(AT_GPRS_APN);
    sim808_wait_for_response(response, sizeof(response), 10000);
    if (!strstr(response, "OK")) {
        printf("*******Error: Configuración APN fallida.*******\n");
        return 0;
    }
    return 1;
}

// Paso 3: Activar conexión de datos
int sim808_gprs_activate_data(void) {
    char response[128];
    sim808_send_command("AT+CGACT=1,1\r\n");
    sim808_wait_for_response(response, sizeof(response), 10000);
    if (!strstr(response, "OK")) {
        printf("*******Error: Activación de conexión de datos fallida.*******\n");
        return 0;
    }
    return 1;
}

// Paso 4: Establecer enlace PPP
int sim808_gprs_establish_ppp(void) {
    char response[128];
    sim808_send_command(AT_PDP_CONNECT);
    sim808_wait_for_response(response, sizeof(response), 10000);
    if (!strstr(response, "OK")) {
        printf("*******Error: Enlace PPP fallido.*******\n");
        return 0;
    }
    return 1;
}

// Paso 5: Obtener dirección IP
int sim808_gprs_get_ip(void) {
    char response[128];
    sim808_send_command("AT+CIFSR\r\n");
    sim808_wait_for_response(response, sizeof(response), 10000);
    if (strstr(response, "ERROR")) {
        printf("*******Error: No se pudo obtener dirección IP.*******\n");
        return 0;
    }
    printf("Dirección IP obtenida: %s\n", response); // Imprimir IP
    return 1;
}

// Paso 6: Conexión TCP
int sim808_gprs_tcp_connect(void) {
    char response[128];
    sim808_send_command(AT_TCP_SOCKET);
    sim808_wait_for_response(response, sizeof(response), 10000);
    if (strstr(response, "ERROR")) {
        printf("*******Error: No se pudo realizar la conexión TCP.*******\n");
        return 0;
    }
    printf("Conexion TCP: %s\n", response); // Tengo que ver con que responde el modulo
    return 1;
}

void sim808_gprs_send_data(char *shipping_buffer) {
    char response[256];
    char http_request[512];

    // Preparar la solicitud HTTP
    snprintf(http_request, sizeof(http_request),
             "POST /upload-data HTTP/1.1\r\n"
             "Host: \"" GPRS_SERVER "\":8080\r\n"
             "Content-Type: application/json\r\n"
             "Content-Length: %d\r\n"
             "\r\n"
             "%s",
             strlen(shipping_buffer), shipping_buffer);

    // Iniciar la transmisión de datos
    sim808_send_command("AT+CIPSEND\r\n");

    // Esperar el símbolo '>' indicando que el módulo está listo para enviar datos
    sim808_wait_for_response(response, sizeof(response), 10000);
    if (strstr(response, ">")) {
        printf("El módulo está listo para enviar datos.\n");

        // Enviar los datos
        sim808_send_command(http_request);
        uart_write_bytes(UART_NUM, "\x1A", 1); // CTRL+Z para enviar y finalizar

        // Verificar el resultado del envío
        sim808_wait_for_response(response, sizeof(response), 20000); // Esperar hasta 10s
        if (strstr(response, "SEND OK")) {
            printf("Datos enviados correctamente: %s\n", shipping_buffer);
            return;
        } else if (strstr(response, "SEND FAIL")) {
            printf("Error: El envío de datos falló. Respuesta del módulo: %s\n", response);
        } else {
            printf("Error desconocido durante el envío de datos. Respuesta del módulo: %s\n", response);
        }
    } else {
        printf("Error: El módulo no está listo para enviar datos. Respuesta: %s\n", response);
    }
}

// Implementación de la función principal para solicitud HTTPS
int sim808_gprs_https_request(const char *url, const char *data) {
    char command[256];
    char response[RESPONSE_BUFFER_SIZE];

    // 1. Inicializar HTTP
    sim808_send_command(CMD_HTTPINIT);
    if (sim808_wait_for_response(response, sizeof(response), RESPONSE_TIMEOUT_MS) != 0) {
        printf("Error al inicializar HTTP: %s\n", response);
        return -1;
    }

    // 2. Habilitar HTTPS (SSL)
    sim808_send_command(CMD_HTTPSSL);
    if (sim808_wait_for_response(response, sizeof(response), RESPONSE_TIMEOUT_MS) != 0) {
        printf("Error al habilitar HTTPS: %s\n", response);
        return -2;
    }

    // 3. Configurar la URL
    snprintf(command, sizeof(command), CMD_HTTPPARA_URL, url);
    sim808_send_command(command);
    if (sim808_wait_for_response(response, sizeof(response), RESPONSE_TIMEOUT_MS) != 0) {
        printf("Error al configurar la URL: %s\n", response);
        return -3;
    }

    // 4. Configurar el contenido como JSON
    sim808_send_command(CMD_HTTPPARA_CONTENT);
    if (sim808_wait_for_response(response, sizeof(response), RESPONSE_TIMEOUT_MS) != 0) {
        printf("Error al configurar el tipo de contenido: %s\n", response);
        return -4;
    }

    // 5. Preparar los datos para el envío
    snprintf(command, sizeof(command), CMD_HTTPDATA, strlen(data));
    sim808_send_command(command);
    if (sim808_wait_for_response(response, sizeof(response), RESPONSE_TIMEOUT_MS) != 0) {
        if (strstr(response, "DOWNLOAD") == NULL) {
            printf("Error al preparar los datos: %s\n", response);
            return -5;
        }
    }

    // 6. Enviar los datos
    sim808_send_command(data);
    if (sim808_wait_for_response(response, sizeof(response), RESPONSE_TIMEOUT_MS) != 0) {
        printf("Error al enviar los datos: %s\n", response);
        return -6;
    }

    // 7. Ejecutar la solicitud POST
    sim808_send_command(CMD_HTTPACTION);
    if (sim808_wait_for_response(response, sizeof(response), RESPONSE_TIMEOUT_MS) != 0) {
        if (strstr(response, "+HTTPACTION:") == NULL) {
            printf("Error al ejecutar la solicitud POST: %s\n", response);
            return -7;
        }
    }

    // 8. Leer la respuesta del servidor
    sim808_send_command(CMD_HTTPREAD);
    if (sim808_wait_for_response(response, sizeof(response), RESPONSE_TIMEOUT_MS) != 0) {
        printf("Error al leer la respuesta: %s\n", response);
        return -8;
    }

    // Imprimir la respuesta del servidor
    printf("Respuesta del servidor: %s\n", response);

    // 9. Finalizar la conexión HTTP
    sim808_send_command(CMD_HTTPTERM);
    if (sim808_wait_for_response(response, sizeof(response), RESPONSE_TIMEOUT_MS) != 0) {
        printf("Error al finalizar HTTP: %s\n", response);
        return -9;
    }

    printf("Solicitud HTTPS enviada con éxito.\n");
    return 0;
}

// Desconectar la sesión GPRS
int sim808_gprs_disconnect(void) {
    char response[256];
    sim808_send_command("AT+CIPSHUT\r\n");  // Cerrar la conexión GPRS
    sim808_wait_for_response(response, sizeof(response), 10000); // Espera hasta 5s
    return 1;
}