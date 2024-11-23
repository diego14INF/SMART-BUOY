
#include <stdio.h>
#include "sim808_gps.h" // Archivo de encabezado que contiene las declaraciones de las funciones y estructuras
#include <esp_rom_sys.h>  // Necesario para `esp_rom_delay_us()`
#include "data_storage.h"
#include "sim808_gprs.h"
#include "gps_state_machine.h"
#include "timers.h"
#include "gprs_state_machine.h"
#include "gsm_data.h"

void app_main(void) {

    //Inicialización de máquinas de estado:
    gps_state_machine_init();
    gprs_state_machine_init();

    // Inicialización del módulo GPS SIM808
    printf("Iniciando programa con SIM808...\n");
    if (sim808_init()==0){
        printf("Error al inicializar el GPS del SIM808.\n");
    }
  
    // Ciclo principal
    while (1) {
        //Corriendo máquina de estados del GPS
        gps_state_machine_run();
        
        //Corriendo máquina de estados del módulo GSM/GPRS
        gprs_state_machine_run();

        // Retraso de 1 segundo
        esp_rom_delay_us(2000000);   // 1000 ms = 1000000 us
    }
}