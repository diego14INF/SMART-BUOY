#ifndef PERIPHERALS_STATE_MACHINE_H
#define PERIPHERALS_STATE_MACHINE_H

#include <stdbool.h>

typedef enum {
    EVENTO_BALIZA_SOCORRO,
    EVENTO_BALIZA_ESTADO_OK,
    EVENTO_BALIZA_BAJA_BATERIA,
    EVENTO_BALIZA_EMPAREJAMIENTO,
    EVENTO_BALIZA_TRANSMISION,
    EVENTO_BALIZA_ERROR
} evento_baliza_t;


typedef enum {
    EVENT_AYUDA,
    EVENT_SOCORRO,
    EVENT_FUERA,
    EVENT_EMPAREJAR
} boton_evento_t;

// Estados principales
typedef enum {
    ESTADO_INICIAL,
    ESTADO_BAJO_BATERIA,
    ESTADO_SOCORRO,
    ESTADO_ENVIAR_MENSAJE,
    ESTADO_EMPAREJAMIENTO,
    ESTADO_ERROR
} estado_boya_t;

// Inicialización
void peripherals_state_machine_init(void);
void peripherals_state_machine_run(void);  // Se llama periódicamente

#endif 