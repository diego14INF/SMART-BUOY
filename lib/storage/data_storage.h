#ifndef DATA_STORAGE_H
#define DATA_STORAGE_H

#include "sim808_gps.h"

#define STORAGE_SIZE 100

typedef struct {
    time_t timestamp;
    GPSData gps_data;
} DataEntry;

// Funciones de almacenamiento
void data_storage_init();
int data_storage_save(GPSData *data);
DataEntry* data_storage_get_all();
int data_storage_get_count();

#endif // DATA_STORAGE_H