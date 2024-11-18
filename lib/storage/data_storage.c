#include "data_storage.h"
#include <time.h>

static DataEntry storage[STORAGE_SIZE];
static int storage_count = 0;

void data_storage_init() {
    storage_count = 0;
}

int data_storage_save(GPSData *data) {
    if (storage_count < STORAGE_SIZE) {
        storage[storage_count].timestamp = time(NULL);
        storage[storage_count].gps_data = *data;
        storage_count++;
        return 1;
    }
    return 0;
}

DataEntry* data_storage_get_all() {
    return storage;
}

int data_storage_get_count() {
    return storage_count;
}