#include "gsm_comm.h"
#include <stdio.h>

int gsm_prepare_data(char *buffer, size_t buffer_size) {
    DataEntry *entries = data_storage_get_all();
    int count = data_storage_get_count();
    int offset = 0;

    for (int i = 0; i < count; i++) {
        offset += snprintf(buffer + offset, buffer_size - offset,
                           "Time: %ld, Lat: %.6f, Lon: %.6f, Alt: %.2f, Speed: %.2f, Course: %.2f, Battery: %d\n",
                           entries[i].timestamp, entries[i].gps_data.latitude, 
                           entries[i].gps_data.longitude, entries[i].gps_data.altitude,
                           entries[i].gps_data.speed, entries[i].gps_data.course,
                           entries[i].gps_data.battery_voltage);
        if (offset >= buffer_size) {
            return 0;
        }
    }
    return 1;
}

int gsm_send_data(const char *data) {
    printf("Sending data over GSM:\n%s\n", data);
    return 1;
}