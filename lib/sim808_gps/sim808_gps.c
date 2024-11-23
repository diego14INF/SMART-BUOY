#include "sim808_gps.h"
#include "driver/uart.h"
#include <string.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

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
    size_t total_bytes_read = 0; // Total de bytes leídos
    int bytes_read = 0;          // Bytes leídos en cada iteración
    TickType_t start_time = xTaskGetTickCount(); // Tiempo de inicio

    // Leer datos mientras no se excedan los 3 segundos y haya espacio en el buffer
    while ((xTaskGetTickCount() - start_time) < (5000 / portTICK_PERIOD_MS)) {
        if (total_bytes_read < buffer_size - 1) {
            bytes_read = uart_read_bytes(UART_NUM, (uint8_t *)buffer + total_bytes_read, 
                                         buffer_size - 1 - total_bytes_read, 100 / portTICK_PERIOD_MS);
            if (bytes_read > 0) {
                total_bytes_read += bytes_read;
            }
        } else {
            break; // Salir si el buffer está lleno
        }
    }

    // Asegurarse de terminar el buffer como cadena
    buffer[total_bytes_read] = '\0';

    // Verificar si se leyeron datos
    if (total_bytes_read > 0) {
        printf("Respuesta del SIM808: %s\n", buffer);
    } else {
        printf("No se recibieron datos del SIM808.\n");
    }
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

    printf("Respuesta a la inicialización: %s\n", response);

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
    sim808_read_response(command, sizeof(command));
    printf("Respuesta de GPS reset: %s\n", command);
}

int sim808_gps_get_status() {
    sim808_send_command(GPS_STATUS_COMMAND);
    char response[128];
    sim808_read_response(response, sizeof(response));
    printf("Estado actual del GPS: %s\n", response);
     // Verificar y mostrar el estado del GPS basado en la respuesta
    if (strstr(response, "Location 3D Fix") != NULL) {
        printf("GPS: Fijación de ubicación en 3D.\n");
        return 1;
    } else if (strstr(response, "Location 2D Fix") != NULL) {
        printf("GPS: Fijación de ubicación en 2D.\n");
        return 1;
    } else if (strstr(response, "Location Not Fix") != NULL) {
        printf("GPS: Ubicación conocida, pero sin fijación.\n");
        return 2;
    } else if (strstr(response, "Location Unknown") != NULL) {
        printf("GPS: Ubicación desconocida.\n");
        return 3;
    } else {
        printf("GPS: Estado desconocido o sin respuesta.\n");
        return 4;  //Parcheado mientras no cambie comando a enviar ya que parece que no lo soporta el firmware del sim808
    }

}

// Obtiene los datos GPS
int sim808_get_gps_data(GPSData *data) {
    sim808_send_command(GPS_INF_COMMAND);
    char response[BUF_SIZE];
    sim808_read_response(response, BUF_SIZE);

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
    sim808_read_response(response, BUF_SIZE);
    int level,vbat;
    
    if (sscanf(strstr(response, "+CBC: "), "+CBC: %*d,%d,%d", &level,&vbat)==2){ 
        data->battery_level= level;
        data->battery_voltage= vbat;
        printf("Nivel de la bateria: %d, Voltaje de la batería: %d mV\n", data->battery_level, data->battery_voltage); 
        return 1;
    }else {
        printf("No se pudo obtener el voltaje de la batería.\n");
    }
    return 0;
    
}

