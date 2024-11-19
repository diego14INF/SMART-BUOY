#include "data_storage.h"
#include <time.h>
#include "esp_heap_caps.h"

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
    size_t free_dram = heap_caps_get_free_size(MALLOC_CAP_8BIT);
    size_t free_psram = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    printf("DRAM libre: %d bytes\n", free_dram);
    printf("PSRAM libre: %d bytes\n", free_psram);    
    return storage_count;
}