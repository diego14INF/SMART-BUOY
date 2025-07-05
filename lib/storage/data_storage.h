#ifndef DATA_STORAGE_H
#define DATA_STORAGE_H

#include "sim808_gps.h"
#include <stdbool.h>

#define STORAGE_SIZE 100
#define MMSI_UNICO 970000001

#define STORAGE_ENLAZADOS 5


typedef struct {
    long long int mmsi;
    GPSData gps_data;
} DataEntry;

// Funciones de almacenamiento
void data_storage_init();
int data_storage_save(GPSData *data);
int data_storage_paired_save(long long int a, GPSData *data);
DataEntry* data_storage_get_all();
int data_storage_get_count();
GPSData* data_storage_get_sos();
bool data_storage_get_count_sos();

#endif // DATA_STORAGE_H