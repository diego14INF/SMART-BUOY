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


//FUNCIONES DE CONTROL DE ESTADO----------------------------------------------------------------

int sim808_check_signal_strength() {
    char response[254];
    int signal_strength = 0;

    // Enviar comando AT+CSQ
    sim808_send_command("AT+CSQ\r\n");
    sim808_wait_for_response(response, sizeof(response), 10000); // Espera hasta 5s

    // Extraer valor de la señal GSM
    if (sscanf(response, "+CSQ: %d,", &signal_strength) == 1) {
        if (signal_strength >= 10 && signal_strength <= 30) {
            return 1; // Señal adecuada
        }
    }

    return 0;
}

int sim808_check_registration_status() {
    char response[254];
    int registration_status = 0;

    // Enviar comando AT+CREG?
    sim808_send_command("AT+CREG?\r\n");
    sim808_wait_for_response(response, sizeof(response), 10000); // Espera hasta 5s

    // Extraer estado de registro
    if (sscanf(response, "+CREG: %*d,%d", &registration_status) == 1) {
        if (registration_status == 1 || registration_status == 5) {
            return 1; // Registro exitoso
        }
    }

    return 0; // No registrado
}

int sim808_check_functionality_status() {
    char response[254];
    int cfun_status = 0;

    // Enviar comando AT+CFUN?
    sim808_send_command("AT+CFUN?\r\n");
    sim808_wait_for_response(response, sizeof(response), 10000); // Espera hasta 5s

    // Extraer el estado funcional
    if (sscanf(response, "+CFUN: %d", &cfun_status) == 1) {
        if (cfun_status == 1) {
            return 1; // Módulo en estado funcional completo
        }
    }

    return 0; // Módulo no funcional
}

int sim808_check_gprs_attachment(void) {
    char response[256];
    int gprs_att=0;

    sim808_send_command(AT_GPRS_CTRL_GPRS_ATTACHMENT);
    sim808_wait_for_response(response, sizeof(response), 10000); // Espera hasta 5s
    // Extrae si está adherido a la red
    if (sscanf(strstr(response, "+CGATT: 1"), "+CGATT: %d", &gprs_att) == 1) {
        if (gprs_att == 1) {
            return 1;
        }
    }

    return 0;
}

int sim808_check_apn_present(void) {
    char response[256];
    sim808_send_command("AT+CGDCONT?\r\n");
    sim808_wait_for_response(response, sizeof(response), 5000); 
    
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

//FUNCIONES DE CONEXION A RED GPRS-------------------------------------------------------------------

//Paso 0: Preparar tarjeta SIM
int sim808_config_sim(void){
    char response[254];

     // Paso 0.1: Verificar el estado de la tarjeta SIM
    sim808_send_command("AT+CPIN?\r\n");
    sim808_wait_for_response(response, sizeof(response), 10000); // Espera hasta 5s
    if (!strstr(response, "+CPIN: READY")) {
        // Introducir el PIN si la tarjeta lo requiere
        sim808_send_command("AT+CPIN=\"8495\"\r\n"); 
        sim808_wait_for_response(response, sizeof(response), 10000); // Espera hasta 5s
        if (!strstr(response, "+CPIN: READY")) {
            printf("*******Error: No se pudo desbloquear la tarjeta SIM.*******\n");
            return 0;
        }
    } 
    return 1;
}

//Paso 1: Conectar a la red GPRS
int sim808_gprs_connect_init(void) {
    char response[128];
    sim808_send_command(AT_GPRS_CTRL_GPRS_ATTACHMENT);
    sim808_wait_for_response(response, sizeof(response), 10000); 
    if (!strstr(response, "+CGATT: 1")) {
        sim808_send_command(AT_GPRS_INIT);
        sim808_wait_for_response(response, sizeof(response), 10000);
        if (!strstr(response, "OK")) {
            printf("*******Error: No se pudo conectar a la red GPRS.*******\n");
            return 0;
        }
    }
    return 1;    
}

// Paso 2: Configurar APN
int sim808_gprs_connect_apn(void) {
    if (!sim808_check_apn_present()){
      char response[128];
      sim808_send_command(AT_GPRS_APN);
      sim808_wait_for_response(response, sizeof(response), 10000);
      if (!strstr(response, "OK")) {
          printf("*******Error: Configuración APN fallida.*******\n");
          return 0;
      }
    
    } 
    return 1;
}

// Paso 3: iniciar conexion inalambrica
int sim808_gprs_wireless_activate(void) {
    char response[128];
    sim808_send_command(AT_GPRS_WIRELESS_CONNECT);
    vTaskDelay(pdMS_TO_TICKS(4000));
    sim808_wait_for_response(response, sizeof(response), 10000);
    if (!strstr(response, "OK")) {
        printf("*******Error: Activación de conexión de datos fallida.*******\n");
        return 0;
    }
    return 1;
}

// Paso 4: Obtener dirección IP
int sim808_gprs_get_ip(void) {
    char response[128];
    sim808_send_command(AT_GPRS_CTRL_IP);
    sim808_wait_for_response(response, sizeof(response), 10000);
    if (strstr(response, "ERROR")) {
        printf("*******Error: No se pudo obtener dirección IP.*******\n");
        return 0;
    }
    printf("Dirección IP obtenida: %s\n", response); // Imprimir IP
    return 1;
}

//------------------------------------------------------------------------------------------------------

// Paso 5: Conexión TCP
int sim808_gprs_tcp_connect(void) {
    char response[128];
    sim808_send_command(AT_TCP_SOCKET);
    vTaskDelay(pdMS_TO_TICKS(2000));
    sim808_wait_for_response(response, sizeof(response), 10000);
    
    if (strstr(response, "ALREADY CONNECT") || strstr(response, "CONNECT OK") ){
       printf("Conexion TCP: %s\n", response); // Tengo que ver con que responde el modulo
         return 1;
    }else {
        printf("*******Error: No se pudo realizar la conexión TCP.*******\n");
        return 0;
    }

    return 1;
}


//-----------------------------ENVIO TCP/IP SOCKETS (Version Http) ------------------------------------
int sim808_gprs_send_data(char *shipping_buffer) {
    char response[4096];
    char http_request[1024];

    // Preparar la solicitud HTTP
    snprintf(http_request, sizeof(http_request),
             "POST / HTTP/1.1\r\n"
             "Host:" GPRS_SERVER ":%d\r\n"
             "Content-Type: application/json\r\n"
             "Content-Length: %d\r\n"
             "User-Agent: SIM808\r\n"
             "Connection:close\r\n"
             "Authorization: Bearer TU_TOKEN\r\n"
             "\r\n"
             "%s"
             "\r\n",
             HTTP_PORT, strlen(shipping_buffer), shipping_buffer);

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
        sim808_wait_for_response(response, sizeof(response), 10000); // Esperar hasta 10s
         vTaskDelay(pdMS_TO_TICKS(3000));

        // Verificar el resultado del envío (esperar "SEND OK")
        if (strstr(response, "SEND OK")) {
            printf("SEND OK VÁLIDO. Datos enviados correctamente al servidor.\n");

                // Procesar el código de estado HTTP
                if (strstr(response, "HTTP/1.1 200 OK")) {
                    printf("Solicitud exitosa. Datos procesados correctamente por el servidor.\n");
                        return 1; // Éxito
                }else if(strstr(response, "HTTP/1.") != NULL) {

                    int http_code = 0;
                    sscanf(response, "HTTP/1.%*d %d", &http_code); // Extraer el código de estado HTTP
                    printf("Código de estado HTTP: %d\n", http_code);

                    if (http_code == 400) {
                        printf("Error: Solicitud incorrecta (400).\n");
                        return -1; // Error del cliente
                    } else if (http_code == 500) {
                        printf("Error: Error interno del servidor (500).\n");
                        return -2; // Error del servidor
                    } else {
                        printf("Código HTTP desconocido: %d. Respuesta: %s\n", http_code, response);
                        return -3; // Código desconocido
                    }
                } else {
                    printf("Respuesta inesperada del servidor: %s\n", response);
                    return -4; // Respuesta no válida
                }
        
        } else if (strstr(response, "SEND FAIL")) {
            printf("Error: El envío de datos falló. Respuesta del módulo: %s\n", response);
            return 0;
        } else {
            printf("Error desconocido durante el envío de datos. Respuesta del módulo: %s\n", response);
            return 0;
        }
    } else {
        printf("Error: El módulo no está listo para enviar datos. Respuesta: %s\n", response);
        return 0;
    }
}

int sim808_gprs_get_data(char *mmsi_enlazado) {
    char response[4096];
    char http_request[1024];
    char ubicacion_buffer[1024];

    // Preparar la solicitud HTTP
    snprintf(http_request, sizeof(http_request),
             "GET /ubicacion/%s HTTP/1.1\r\n"
             "Host:" GPRS_SERVER ":%d\r\n"
             "Content-Type: application/json\r\n"
             "User-Agent: SIM808\r\n"
             "Connection:close\r\n"
             "Authorization: Bearer TU_TOKEN\r\n"
             "\r\n",
             mmsi_enlazado, HTTP_PORT);

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
        sim808_wait_for_response(response, sizeof(response), 10000); // Esperar hasta 10s
         vTaskDelay(pdMS_TO_TICKS(3000));

        // Verificar el resultado del envío (esperar "SEND OK")
        if (strstr(response, "SEND OK")) {
            printf("SEND OK VÁLIDO. Petición enviada correctamente.\n");

                // Procesar el código de estado HTTP
                if (strstr(response, "HTTP/1.1 200 OK")) {
                    // Buscar inicio del JSON (primera llave después del encabezado)
                    char *json_start = strchr(response, '{');
                    if (json_start) {
                       strncpy(ubicacion_buffer, json_start, sizeof(ubicacion_buffer) - 1);
                       printf("✅ JSON recibido y almacenado:\n%s\n", ubicacion_buffer);
                     // Extraer datos del JSON (suponiendo campos "latitude", "longitude", "timestamp")
                      GPSData data;
                      long long int mmsi_long;
                      long long int mmsi_enlazado_ll = atoll(mmsi_enlazado);  // Convertir cadena a long long int

                       if (sscanf(json_start,
                            "{\"mmsi\": %lli, \"time_sim808\": %lf, \"latitude\": %f, \"longitude\": %f, "
                           "\"altitude\": %f, \"speed\": %f, \"course\": %f, \"battery_voltage\": %d}",
                           &mmsi_long, &data.time, &data.latitude, &data.longitude,
                           &data.altitude, &data.speed, &data.course, &data.battery_voltage) == 8) {
                           if (mmsi_long == mmsi_enlazado_ll) {
                              data_storage_paired_save(mmsi_long, &data);
                              printf("✅ Datos válidos y almacenados para MMSI %lli\n", mmsi_long);
                            } else {
                                printf("⚠ MMSI en JSON (%lli) no coincide con el esperado (%lli). No se almacenó.\n",
                                 mmsi_long, mmsi_enlazado_ll);
                            }
                        } else {
                            printf("⚠ Error al parsear el JSON en GPSData.\n");
                            return -6;
                        }
                    } else {
                        printf("⚠ No se encontró JSON válido en la respuesta.\n");
                        return -5;
                    }
                }else if(strstr(response, "HTTP/1.") != NULL) {

                    int http_code = 0;
                    sscanf(response, "HTTP/1.%*d %d", &http_code); // Extraer el código de estado HTTP
                    printf("Código de estado HTTP: %d\n", http_code);

                    if (http_code == 400) {
                        printf("Error: Solicitud incorrecta (400).\n");
                        return -1; // Error del cliente
                    } else if (http_code == 500) {
                        printf("Error: Error interno del servidor (500).\n");
                        return -2; // Error del servidor
                    } else {
                        printf("Código HTTP desconocido: %d. Respuesta: %s\n", http_code, response);
                        return -3; // Código desconocido
                    }
                } else {
                    printf("Respuesta inesperada del servidor: %s\n", response);
                    return -4; // Respuesta no válida
                }
        
        } else if (strstr(response, "SEND FAIL")) {
            printf("Error: El envío de datos falló. Respuesta del módulo: %s\n", response);
            return 0;
        } else {
            printf("Error desconocido durante el envío de datos. Respuesta del módulo: %s\n", response);
            return 0;
        }
    } else {
        printf("Error: El módulo no está listo para enviar datos. Respuesta: %s\n", response);
        return 0;
    }
    return 1;
}

// Desconectar la sesión GPRS
int sim808_gprs_disconnect(void) {
    char response[256];
    sim808_send_command("AT+CIPSHUT\r\n");  // Cerrar la conexión GPRS
    sim808_wait_for_response(response, sizeof(response), 10000); // Espera hasta 5s
    return 1;
}

//--------MENSAJE SMS SOS--------------------
int sim808_send_sos_sms(GPSData *data)
{   char response [256];
    //char cmd[128];
    char payload[256];

 char time_str[20]; // Buffer para almacenar el tiempo como cadena

// Convertir el valor double a una cadena
snprintf(time_str, sizeof(time_str), "%.6f", data->time);

// Extraer los valores de fecha y hora
int anio, mes, dia, hora, minuto, segundo;
sscanf(time_str, "%4d%2d%2d%2d%2d%2d", &anio, &mes, &dia, &hora, &minuto, &segundo);


    // 1) Poner SIM808 en modo texto
    sim808_send_command(SMS_CONFIG_COMAND);
    vTaskDelay(pdMS_TO_TICKS(500));

    // 2) Preparar el mensaje SMS con los datos GPS
    // Formato: SOS! MMSI:xxxxx Lat:xx.xxxx Lon:yy.yyyy Time:tt.tttttt
    snprintf(payload, sizeof(payload),
             "SOS!!! Necesito rescate! Dia&Hora:%s Lat:%.6f Lon:%.6f ",
             time_str,
             data->latitude,
             data->longitude
             );

    // 3) Indicar número de destino
    sim808_send_command(SMS_PHONE_NUMBER_COMAND);

    sim808_wait_for_response(response, sizeof(response), 10);
    if (strstr(response, ">")) {
    // 4) Enviar el cuerpo del SMS y CTRL+Z (0x1A)
       sim808_send_command(payload);
        uart_write_bytes(UART_NUM, "\x1A", 1); // CTRL+Z para enviar y finalizar

        // Verificar el resultado del envío
        sim808_wait_for_response(response, sizeof(response), 10000); // Esperar hasta 10s
         vTaskDelay(pdMS_TO_TICKS(3000));
    }else{
        return 0;
    }

    //sim808_wait_for_response(response, sizeof(response), 10);
        if (strstr(response, "OK")) {
            printf("✅ SMS de socorro enviado: %s\n", payload);
            return 1;
        } else {
            printf("❌ Error al enviar SMS: %s\n", response);
            return -1;
        }
    

    printf("❌ Timeout esperando confirmación de SMS.\n");
    return 0;
}