#include "gps_data.h"
#include <stdio.h>
#include <string.h>

int parse_gps_data(const char *response, GPSData *gps_data) {
     // Busca la cadena de inicio "+CGNSINF: "
    const char *lat_ptr = strstr(response, "+CGNSINF: ");
    if (lat_ptr) {
        float lat, lon;
        if (sscanf(lat_ptr, "+CGNSINF: %*d,%*d,%*f,%f,%f", &lat, &lon) == 2) {
            gps_data->latitude = lat;
            gps_data->longitude = lon;
            return 1;  // Éxito
        }
    }
    return 0;  // No se encontraron datos
}