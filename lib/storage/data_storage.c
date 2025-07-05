#include "data_storage.h"
#include <time.h>
#include "esp_heap_caps.h"

static DataEntry storage[STORAGE_SIZE];
static int storage_count = 0;

static DataEntry storage_paired[STORAGE_ENLAZADOS];
static int storage_count_paired = 0;

static DataEntry storage_sos[1];
bool storage_count_sos = false;

void data_storage_init() {
    storage_count = 0;
}

int data_storage_save(GPSData *data) {
        storage_sos[0].mmsi = MMSI_UNICO;
        storage_sos[0].gps_data = *data;
        storage_count_sos =true;
     if (storage_count < STORAGE_SIZE) {
        // Si hay espacio, añade directamente
        storage[storage_count].mmsi = MMSI_UNICO;
        storage[storage_count].gps_data = *data;
        storage_count++;
    } else {
        // Si está lleno, elimina el más antiguo (desplaza todos los datos)
        for (int i = 1; i < STORAGE_SIZE; i++) {
            storage[i - 1] = storage[i];
        }
        // Añade el nuevo dato al final
        storage[STORAGE_SIZE - 1].mmsi = MMSI_UNICO;
        storage[STORAGE_SIZE - 1].gps_data = *data;
    }
    return 1;
}

int data_storage_paired_save(long long int mmsi, GPSData *data) {
    // Buscar si ya existe el MMSI
    for (int i = 0; i < storage_count_paired; i++) {
        if (storage_paired[i].mmsi == mmsi) {
            // Ya existe → actualizar datos
            storage_paired[i].gps_data = *data;
            printf("🔁 Datos actualizados para MMSI enlazado: %lli\n", mmsi);
            return 1;
        }
    }

    // Si no existe y hay espacio → añadir nuevo
    if (storage_count_paired < STORAGE_ENLAZADOS) {
        storage_paired[storage_count_paired].mmsi = mmsi;
        storage_paired[storage_count_paired].gps_data = *data;
        storage_count_paired++;
        printf("✅ MMSI enlazado nuevo almacenado: %lli\n", mmsi);
        return 1;
    }

    // Si está lleno → reemplazar el más antiguo (FIFO)
    for (int i = 1; i < STORAGE_ENLAZADOS; i++) {
        storage_paired[i - 1] = storage_paired[i];
    }
    storage_paired[STORAGE_ENLAZADOS - 1].mmsi = mmsi;
    storage_paired[STORAGE_ENLAZADOS - 1].gps_data = *data;
    printf("⚠️ Almacenamiento enlazado lleno. Reemplazado más antiguo con MMSI: %lld\n", mmsi);
    return 1;
}


DataEntry* data_storage_get_all() {
    return storage;
}
GPSData* data_storage_get_sos(){
    return &storage_sos->gps_data;
}

int data_storage_get_count() {

    return storage_count;
}

bool data_storage_get_count_sos() {

    return storage_count_sos;
}