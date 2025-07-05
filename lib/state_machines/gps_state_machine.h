#ifndef GPS_STATE_MACHINE_H
#define GPS_STATE_MACHINE_H

#include <stdbool.h>
#include "sim808_gps.h"

// Estados de la máquina de estados
typedef enum {
    STATE_VERIFY_GPS,
    STATE_ACQUIRE_DATA,
    STATE_STORE_DATA,
    STATE_WAIT_TIMER,
    STATE_ERROR
} GPSState;

// Inicialización de la máquina de estados
void gps_state_machine_init(void);

void start_gps_reset_timer(int reset_type);

bool gps_data_is_valid(const GPSData *d);

// Ejecutar la máquina de estados
void gps_state_machine_run(void);


#endif // GPS_STATE_MACHINE_H