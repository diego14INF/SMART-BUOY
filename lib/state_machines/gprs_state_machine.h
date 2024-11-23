#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <stdbool.h>
#include <stdio.h>


// Definición de estados
typedef enum {
    ESPERA,
    PREPARAR_BUFFER,
    ENVIAR_DATOS,
    CONFIRMAR_ENVIO,
    ERROR
} State;

void gprs_state_machine_init(void);

// Función principal de la máquina de estados
void gprs_state_machine_run(void);

#endif // STATE_MACHINE_H