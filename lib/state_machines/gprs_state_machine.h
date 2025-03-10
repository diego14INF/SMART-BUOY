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
    GPRS_SUBSTATE_ACTIVATE, //Activate y ppp lo mismo
    GPRS_SUBSTATE_PPP,
    GPRS_SUBSTATE_IP,
    GPRS_SUBSTATE_TCP_CONNECT,
    GPRS_SUBSTATE_CONNECTED,
    GPRS_SUBSTATE_ERROR,
    GPRS_SUBSTATE_RESET_WAIT,
} GPRSConnectionSubstate;

typedef enum {
    HTTP_STATE_INIT,
    HTTP_STATE_ENABLE,
    HTTP_STATE_URL_SETUP,
    HTTP_STATE_CONTENT_TYPE,
    HTTP_STATE_PREPARE_REQUEST,
    HTTP_STATE_SEND_DATA,
    HTTP_STATE_POST_ACTION,
    HTTP_STATE_READ_RESPONSE,
    HTTP_STATE_HTTP_TERMINATE,
    HTTP_STATE_FINISHED,
    HTTP_STATE_ERROR
} HTTPState;

void gprs_state_machine_init(void);

int retry_function(int (*func)(void), const char *func_name);

// Función principal de la máquina de estados
void gprs_state_machine_run(void);

#endif // STATE_MACHINE_H