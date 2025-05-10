#include "gsm_data.h"
#include <stdio.h>

#define GSM_BATCH_SIZE 1
#define GSM_MAX_RETRIES 1

int gsm_prepare_batch(char *buffer, size_t buffer_size, int start_index) {
    DataEntry *shipping_buffer = data_storage_get_all(); // Obtén los datos
    int count = data_storage_get_count();               // Total de registros disponibles
    int offset = 0;

    printf("Tamaño del buffer de salida = %d \n", buffer_size);

    // Inicia un array JSON
    offset += snprintf(buffer + offset, buffer_size - offset, "[");

    for (int i = start_index; i < start_index + GSM_BATCH_SIZE && i < count; i++) {
        // Generar un objeto JSON para cada entrada de datos
        offset += snprintf(buffer + offset, buffer_size - offset,
                           "{\"mmsi\": %lli, \"time_sim808\": %.6f, \"latitude\": %.6f, \"longitude\": %.6f, "
                           "\"altitude\": %.2f, \"speed\": %.2f, \"course\": %.2f, \"battery_voltage\": %d}",
                           shipping_buffer[i].mmsi, shipping_buffer[i].gps_data.time,
                           shipping_buffer[i].gps_data.latitude, shipping_buffer[i].gps_data.longitude,
                           shipping_buffer[i].gps_data.altitude, shipping_buffer[i].gps_data.speed,
                           shipping_buffer[i].gps_data.course, shipping_buffer[i].gps_data.battery_voltage);

        // Agregar coma después de cada objeto, excepto el último
        if (i < start_index + GSM_BATCH_SIZE - 1 && i < count - 1) {
            offset += snprintf(buffer + offset, buffer_size - offset, ",");
        }

        // Verificar desbordamiento
        if (offset >= buffer_size) {
            printf("Buffer overflow while preparing JSON batch.\n");
            return -1; // Error por desbordamiento del buffer
        }
    }

    // Cierra el array JSON
    offset += snprintf(buffer + offset, buffer_size - offset, "]");

    // Verificar nuevamente el desbordamiento después de cerrar el array
    if (offset >= buffer_size) {
        printf("Buffer overflow while preparing JSON batch (closing array).\n");
        return -1; // Error por desbordamiento
    }

    return offset; // Devuelve el tamaño del JSON preparado
}


// Prepara una trama con hasta 20 entradas de la memoria para enviar por GSM
// int gsm_prepare_batch(char *buffer, size_t buffer_size, int start_index) {
//     DataEntry *shipping_buffer = data_storage_get_all();
//     int count = data_storage_get_count();
//     int offset = 0;
//     printf("Tamaño del buffer de salida= %d \n", buffer_size);

//     for (int i = start_index; i < start_index + GSM_BATCH_SIZE && i < count; i++) {
//         offset += snprintf(buffer + offset, buffer_size - offset,
//                            "Time ESP32: %lld,Time SIM808: %lf Lat: %.6f, Lon: %.6f, Alt: %.2f, Speed: %.2f, Course: %.2f, Battery: %d\n",
//                            shipping_buffer[i].timestamp,shipping_buffer[i].gps_data.time, shipping_buffer[i].gps_data.latitude,
//                            shipping_buffer[i].gps_data.longitude, shipping_buffer[i].gps_data.altitude,
//                            shipping_buffer[i].gps_data.speed, shipping_buffer[i].gps_data.course,
//                            shipping_buffer[i].gps_data.battery_voltage);
//         if (offset >= buffer_size) {
//             printf("Buffer overflow while preparing batch.\n");
//             return -1; // Error por desbordamiento del buffer
//         }
//     }
//     return offset; // Devuelve el tamaño de los datos preparados
// }
