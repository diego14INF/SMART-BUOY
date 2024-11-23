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
        // Si hay espacio, añade directamente
        storage[storage_count].timestamp = time(NULL);
        storage[storage_count].gps_data = *data;
        storage_count++;
    } else {
        // Si está lleno, elimina el más antiguo (desplaza todos los datos)
        for (int i = 1; i < STORAGE_SIZE; i++) {
            storage[i - 1] = storage[i];
        }
        // Añade el nuevo dato al final
        storage[STORAGE_SIZE - 1].timestamp = time(NULL);
        storage[STORAGE_SIZE - 1].gps_data = *data;
    }
    return 1;
}

DataEntry* data_storage_get_all() {
    return storage;
}

int data_storage_get_count() {

    return storage_count;
}