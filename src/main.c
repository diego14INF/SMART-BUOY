
#include <stdio.h>
#include "sim808_gps.h" // Archivo de encabezado que contiene las declaraciones de las funciones y estructuras
#include <esp_rom_sys.h>  // Necesario para `esp_rom_delay_us()`
#include "data_storage.h"
#include "sim808_gprs.h"

void app_main(void) {

    //Inicialización de variables de estado:
    int gps_state = 0;
    int retry_count = 3;
    int gps_data = 0;
    int storage_status= 0;
 
    //Inicialización almacenamiento datos GPS
    printf("Iniciando almacenamiento memoria GPS...\n");
     GPSData gps_data;
     data_storage_init();
    //----------------------------------------  

    // Inicialización del módulo GPS SIM808
    printf("Iniciando programa con SIM808...\n");
    if (sim808_init()==0){
        printf("Error al inicializar el GPS del SIM808.\n");
    }
    //----------------------------------------


     //Inicialización del módulo GPRS SIM808
    if (sim808_gprs_connect()==0) {
        printf("Error al inicializar el GPRS del SIM808.\n");
    }
    //---------------------------------------- 

    // Reiniciar el GPS en modo "Cold Start" (por ejemplo)
    sim808_gps_reset_mode(2); // 3 representa "Cold Start" en nuestro programa

    // Obtener estado actual del GPS
    sim808_gps_get_status();


    // Ciclo principal
    while (1) {

        // Obtener datos de geolocalización
        sim808_get_gps_data(&gps_data);

        // Monitorear la batería
        sim808_get_battery_status(&gps_data);

        data_storage_save(&gps_data);

        // Obtener estado actual del GPS
        //sim808_gps_get_status();

        // Enviar datos a través de GSM
        //printf("Enviando datos mediante GSM...\n");
        //sim808_send_data_over_gsm(&gps_data);

        // Retraso de 1 segundo
        esp_rom_delay_us(2000000);   // 1000 ms = 1000000 us

        int num_storage;
        num_storage=data_storage_get_count();
        printf("Total de entradas a la memoria: %d\n", num_storage);

    }
}