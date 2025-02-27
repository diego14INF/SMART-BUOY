#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <stdbool.h>
#include <stdio.h>


// Definición de estados
typedef enum {
    ESPERA,
    PREPARAR_BUFFER,
    PREPARAR_RED,
    COMPROBACION_RED,
    ENVIAR_DATOS,
    CONFIRMAR_ENVIO,
    ERROR
} State;

typedef enum {
    GPRS_SUBSTATE_SIM,
    GPRS_SUBSTATE_INIT,
    GPRS_SUBSTATE_APN,
    GPRS_SUBSTATE_ACTIVATE,
    GPRS_SUBSTATE_PPP,
    GPRS_SUBSTATE_IP,
    GPRS_SUBSTATE_TCP_CONNECT,
    GPRS_SUBSTATE_CONNECTED,
    GPRS_SUBSTATE_ERROR
} GPRSConnectionSubstate;

void gprs_state_machine_init(void);

// Función principal de la máquina de estados
void gprs_state_machine_run(void);

#endif // STATE_MACHINE_H