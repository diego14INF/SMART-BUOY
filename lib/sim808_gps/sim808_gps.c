#include "sim808_gps.h"
#include "driver/uart.h"
#include <string.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

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
static char sim808_response[BUF_SIZE]; // Buffer global para almacenar la respuesta
static bool response_ready = false; // Bandera para indicar que hay una respuesta lista
static SemaphoreHandle_t response_semaphore; // Semáforo para sincronización
static QueueHandle_t uart_queue;  // Cola de eventos de UART
static int response_index = 0;


static void uart_event_task(void *pvParameters) {
        uart_event_t event;
        char buffer[BUF_SIZE];
    
        while (1) {
            if (xQueueReceive(uart_queue, &event, portMAX_DELAY)) {
                switch (event.type) {
                    case UART_DATA: {
                        memset(buffer, 0, BUF_SIZE);
                        int len = uart_read_bytes(UART_NUM, (uint8_t *)buffer, event.size, pdMS_TO_TICKS(100));
                        if (len > 0) {
                            buffer[len] = '\0';
                            printf("Respuesta del SIM808:\n%s", buffer); // Datos recibidos
                            
                            // Copiar datos al buffer global
                            if (response_index + len < BUF_SIZE - 1) {
                                memcpy(&sim808_response[response_index], buffer, len);
                                response_index += len;
                                sim808_response[response_index] = '\0';
                            } else {
                                response_index = 0; // Reiniciar en caso de desbordamiento
                            }
    
                            // Verificar si la respuesta está completa
                            if (strstr(sim808_response, "\nERROR") ||
                                strstr(sim808_response, ">") || strstr(sim808_response, "\nSEND OK") ||
                                strstr(sim808_response, "DOWNLOAD") || strstr(sim808_response, "+CME ERROR") ||
                                strstr(sim808_response, "\r\n\r\n0\r\n\r\n") || strstr(sim808_response, ".")) {
                                response_ready = true;
                                response_index = 0;
                                xSemaphoreGive(response_semaphore);
                            }
                            else if (strstr(sim808_response, "\nOK")) {
                                response_ready = true;
                                response_index = 0;
                                xSemaphoreGive(response_semaphore);
                            }
                        }
                        break;
                    }
                    case UART_FIFO_OVF:
                    case UART_BUFFER_FULL:
                        printf("UART buffer overflow, flushing input.\n");
                        uart_flush_input(UART_NUM);
                        xQueueReset(uart_queue);
                        break;
                    case UART_FRAME_ERR:
                        printf("UART frame error detected.\n");
                        uart_flush_input(UART_NUM);
                        response_index = 0;
                        break;
                    default:
                        break;
                }
            }
        }
}

    
int sim808_wait_for_response(char *buffer, size_t buffer_size, uint32_t timeout_ms) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        if (xSemaphoreTake(response_semaphore, pdMS_TO_TICKS(timeout_ms)) == pdTRUE) {
            strncpy(buffer, sim808_response, buffer_size - 1);
            buffer[buffer_size - 1] = '\0';
            response_ready = false;
    
            if (strstr(buffer, "ERROR") || strstr(buffer, "+CME ERROR")) {
                printf("SIM808 Error: %s\n", buffer);
                return -1;
            }
            // Verificar si contiene una dirección IP
          if (strstr(buffer, ".") != NULL) {
            // Validación básica de formato de dirección IP
            int num_dots = 0;
            for (size_t i = 0; i < strlen(buffer); i++) {
                if (buffer[i] == '.') num_dots++;
            }
            if (num_dots == 3) { // Detectar formato típico de IP
                return 1; // Código de éxito para direcciones IP
            }
          }else if (strstr(buffer, "OK") || strstr(buffer, ">") || strstr(buffer, "DOWNLOAD")) {
                return 0;
            }
            return -3;
        } else {
            printf("Timeout waiting for SIM808 response.\n");
            buffer[0] = '\0';
            return -2;
        }
    }
    

void sim808_send_command(const char *command) {
    uart_write_bytes(UART_NUM, command, strlen(command));
    printf("Enviando comando al SIM808: %s\n", command);
}

// void sim808_read_response(char *buffer, size_t buffer_size) {
//     size_t total_bytes_read = 0; // Total de bytes leídos
//     int bytes_read = 0;          // Bytes leídos en cada iteración
//     TickType_t start_time = xTaskGetTickCount(); // Tiempo de inicio
//     // Leer datos mientras no se excedan los 3 segundos y haya espacio en el buffer
//     while ((xTaskGetTickCount() - start_time) < (5000 / portTICK_PERIOD_MS)) {
//         if (total_bytes_read < buffer_size - 1) {
//             bytes_read = uart_read_bytes(UART_NUM, (uint8_t *)buffer + total_bytes_read, 
//                                          buffer_size - 1 - total_bytes_read, 100 / portTICK_PERIOD_MS);
//             if (bytes_read > 0) {
//                 total_bytes_read += bytes_read;
//             }
//         } else {
//             break; // Salir si el buffer está lleno
//         }
//     }
//     // Asegurarse de terminar el buffer como cadena
//     buffer[total_bytes_read] = '\0';
//     // Verificar si se leyeron datos
//     if (total_bytes_read > 0) {
//         printf("Respuesta del SIM808: %s\n", buffer);
//     } else {
//         printf("No se recibieron datos del SIM808.\n");
//     }
// }


// Inicializa el módulo SIM808 y UART
int sim808_init() {
    char response[254];
    
    const uart_config_t uart_config = {
        .baud_rate = BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM, &uart_config);
    uart_set_pin(UART_NUM, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_NUM, BUF_SIZE * 2, BUF_SIZE*2, 10, &uart_queue, 0);
    
    // Desactivar loopback interno
    uart_set_loop_back(UART_NUM_1, false);

    // Crear la tarea de manejo de eventos UART
    xTaskCreate(uart_event_task, "uart_event_task", 4096, NULL, 5, NULL);

    
    response_semaphore = xSemaphoreCreateBinary();
    if (response_semaphore == NULL) {
       printf("Error al crear el semáforo de respuesta.\n");
       return 0; // Error en la inicialización
    }

    sim808_send_command("AT\r\n");
    sim808_wait_for_response(response, sizeof(response), 10000); // Espera hasta 5s

    sim808_send_command("ATE0&W\r\n");
    sim808_wait_for_response(response, sizeof(response), 10000); // Espera hasta 5s

    //Paso 0.0: Activar todas las funcionalidades
    sim808_send_command("AT+CFUN=1\r\n");
    sim808_wait_for_response(response, sizeof(response), 10000); // Espera hasta 5s

    return 1;


}

int sim808_gps_power_on(){   // Enciende el GPS
    char response[64];

    sim808_send_command(GPS_PWR_ON_COMMAND);  
    sim808_wait_for_response(response, sizeof(response), 10000); // Espera hasta 5s
    if (strstr(response, "OK") != NULL) {
        
        return 1;  // Inicialización exitosa
    } else {
        return 0;  // Error en la inicialización
    }
}

void sim808_gps_reset_mode(int mode) {
    char command[16];
    //EScribo los datos formateados (mode) en la cadena de caracteres sin exceder el tamaño del buffer (command)
    snprintf(command, sizeof(command), GPS_RESET_MODE_COMMAND, mode); 
    sim808_send_command(command);
    sim808_wait_for_response(command, sizeof(command), 10000); // Espera hasta 5s
    printf("Respuesta de GPS reset: %s\n", command);
}

int sim808_gps_get_status() {
    sim808_send_command(GPS_INF_COMMAND);
    char response[128];
    sim808_wait_for_response(response, sizeof(response), 10000); // Espera hasta 5s
    int gps_status = -1; // Variable para almacenar el estado del GPS
    
    // Extraer el segundo campo de la respuesta
    if (sscanf(strstr(response, "+CGNSINF: "), "+CGNSINF: %*d,%d", &gps_status) == 1) {
        switch (gps_status) {
            case 1:
                printf("GPS: Fijación de ubicación en 3D.\n");
                return 1;
            case 2:
                printf("GPS: Fijación de ubicación en 2D.\n");
                return 1;
            case 3:
                printf("GPS: Ubicación conocida, pero sin fijación.\n");
                return 2;
            case 0:
                printf("GPS: Ubicación desconocida.\n");
                return 3;
            default:
                 printf("GPS: Estado desconocido o sin respuesta.\n");
                return 4; 
        }
 
    }else{
        printf("GPS: Estado desconocido o sin respuesta.\n");
        return 4; 
    }
 return 0;
}

// Obtiene los datos GPS
int sim808_get_gps_data(GPSData *data) {
    sim808_send_command(GPS_INF_COMMAND);
    char response[BUF_SIZE];
    sim808_wait_for_response(response, BUF_SIZE, 10000); // Espera hasta 5s

    if (strstr(response, "+CGNSINF: ")) {
        float lat,lon,alt,vel,curs;
        double time;
        if (sscanf(strstr(response, "+CGNSINF: "), "+CGNSINF: %*d,%*d,%lf,%f,%f,%f,%f,%f", 
               &time, &lat, &lon, &alt, &vel, &curs)==6){
                data->time=time; 
                data->latitude=lat;
                data->longitude=lon;
                data->altitude=alt;
                data->speed=vel;
                data->course=curs;
               }
               printf("Fecha&hora: %.0f, Latitud: %.6f, Longitud: %.6f, Altitud: %.2f m, Velocidad: %.2f km/h, Curso: %.2f°\n",
                   data->time, data->latitude, data->longitude, data->altitude, data->speed,data->course);

        return 1;
    }
    return 0;
}

//Obtiene el voltaje de la batería (Los datos vienen dados como: 0-1 entero(estado de carga), 0-100(% de carga), 0-5000 (Voltaje en mV))
int sim808_get_battery_status(GPSData *data) {
    sim808_send_command(BATTERY_COMMAND);
    char response[BUF_SIZE];
    sim808_wait_for_response(response, BUF_SIZE, 10000); // Espera hasta 5s
    int level,vbat;
    
    if (sscanf(strstr(response, "+CBC: "), "+CBC: %*d,%d,%d", &level,&vbat)==2){ 
        data->battery_level= level;
        data->battery_voltage= vbat;
        //printf("Nivel de la bateria: %d, Voltaje de la batería: %d mV\n", data->battery_level, data->battery_voltage); 
        return 1;
    }else {
        printf("No se pudo obtener el voltaje de la batería.\n");
    }
    return 0;
    
}

