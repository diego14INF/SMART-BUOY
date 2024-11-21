#include "gsm_data.h"
#include <stdio.h>

#define GSM_BATCH_SIZE 20
#define GSM_MAX_RETRIES 5

// Prepara una trama con hasta 20 entradas de la memoria para enviar por GSM
int gsm_prepare_batch(char *buffer, size_t buffer_size, int start_index) {
    DataEntry *entries = data_storage_get_all();
    int count = data_storage_get_count();
    int offset = 0;

    for (int i = start_index; i < start_index + GSM_BATCH_SIZE && i < count; i++) {
        offset += snprintf(buffer + offset, buffer_size - offset,
                           "Time ESP32: %lld,Time SIM808: %lf Lat: %.6f, Lon: %.6f, Alt: %.2f, Speed: %.2f, Course: %.2f, Battery: %d\n",
                           entries[i].timestamp,entries[i].gps_data.time, entries[i].gps_data.latitude,
                           entries[i].gps_data.longitude, entries[i].gps_data.altitude,
                           entries[i].gps_data.speed, entries[i].gps_data.course,
                           entries[i].gps_data.battery_voltage);
        if (offset >= buffer_size) {
            printf("Buffer overflow while preparing batch.\n");
            return -1; // Error por desbordamiento del buffer
        }
    }
    return offset; // Devuelve el tamaño de los datos preparados
}


// Gestiona el envío de tramas de 20 entradas de la memoria
int gsm_send_data_in_batches() {
    char buffer[1024]; // Buffer para las tramas GSM
    int start_index = 0; // Índice inicial de las entradas a enviar
    int retries = 0;     // Número de reintentos de envío

    while (1) {
        // Prepara la siguiente tanda de datos
        int prepared_size = gsm_prepare_batch(buffer, sizeof(buffer), start_index);
        if (prepared_size <= 0) {
            printf("No more data to send or buffer error.\n");
            break;
        }

        // Intenta enviar los datos, con hasta GSM_MAX_RETRIES reintentos
        int sent_successfully = 0;
        for (retries = 0; retries < GSM_MAX_RETRIES; retries++) {
            if (gsm_send_data(buffer)) {
                sent_successfully = 1;
                printf("Batch sent successfully!\n");
                break;
            } else {
                printf("Retrying (%d/%d)...\n", retries + 1, GSM_MAX_RETRIES);
            }
        }

        if (sent_successfully) {
            // Avanza al siguiente lote
            start_index += GSM_BATCH_SIZE;
            retries = 0;
            return 1;
        } else {
            printf("Failed to send batch after %d retries.\n", GSM_MAX_RETRIES);
            if (data_storage_get_count() > STORAGE_SIZE - GSM_BATCH_SIZE) {
                printf("Skipping current batch to avoid overwriting old data.\n");
                start_index += GSM_BATCH_SIZE; // Evita repetir valores si la memoria está llena
            } else {
                break; // Salir si no hay suficiente memoria nueva
            }
            return 0;
        }

        // Si no hay más datos en la memoria, salir del bucle
        if (start_index >= prepared_size) {
            printf("All data processed.\n");
            break;
        }
    }
}