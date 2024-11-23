#include "gsm_data.h"
#include <stdio.h>

#define GSM_BATCH_SIZE 1
#define GSM_MAX_RETRIES 5

// Prepara una trama con hasta 20 entradas de la memoria para enviar por GSM
int gsm_prepare_batch(char *buffer, size_t buffer_size, int start_index) {
    DataEntry *shipping_buffer = data_storage_get_all();
    int count = data_storage_get_count();
    int offset = 0;
    printf("Tamaño del buffer de salida= %d \n", buffer_size);

    for (int i = start_index; i < start_index + GSM_BATCH_SIZE && i < count; i++) {
        offset += snprintf(buffer + offset, buffer_size - offset,
                           "Time ESP32: %lld,Time SIM808: %lf Lat: %.6f, Lon: %.6f, Alt: %.2f, Speed: %.2f, Course: %.2f, Battery: %d\n",
                           shipping_buffer[i].timestamp,shipping_buffer[i].gps_data.time, shipping_buffer[i].gps_data.latitude,
                           shipping_buffer[i].gps_data.longitude, shipping_buffer[i].gps_data.altitude,
                           shipping_buffer[i].gps_data.speed, shipping_buffer[i].gps_data.course,
                           shipping_buffer[i].gps_data.battery_voltage);
        if (offset >= buffer_size) {
            printf("Buffer overflow while preparing batch.\n");
            return -1; // Error por desbordamiento del buffer
        }
    }
    return offset; // Devuelve el tamaño de los datos preparados
}
