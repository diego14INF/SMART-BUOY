#include <stdio.h>
#include "gps_sim808.h"
#include "gps_data.h"
#include <esp_rom_sys.h>  // Necesario para `esp_rom_delay_us()`

void app_main(void) {
    // Inicializa el módulo SIM808
    sim808_init();    
    
    while (1) {
        // Obtiene la respuesta del módulo SIM808
        char buffer[256];
        sim808_read_response(buffer, sizeof(buffer));

        // Procesa los datos para obtener coordenadas u otra información
        GPSData gps_data;
        if (parse_gps_data(buffer, &gps_data)) {
            printf("Latitud: %f\n", gps_data.latitude);
            printf("Longitud: %f\n", gps_data.longitude);
        }
        
        monitorizar_vbat();
       
       // Retraso de 1 segundo
        esp_rom_delay_us(1000000);   // 1000 ms = 1000000 us

        read_gps_data();
    }
    
}