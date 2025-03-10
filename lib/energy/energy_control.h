#ifndef ENERGY_CONTROL_H
#include <stdint.h>
#include <stdbool.h>

// Inicialización del sistema de control de energía
void energia_init(void);

// Actualiza el estado energético según el consumo y carga de batería
void energia_actualizar(void);

// Obtiene el nivel de batería (%)
uint8_t energia_get_bateria(void);

// Verifica si el sistema está en modo de bajo consumo
bool energia_bajo_consumo(void);

// Lee la tensión de la batería mediante ADC
float energia_leer_tension_bateria(void);

void sim808_control_energia(int a);

#endif // CONTROL_ENERGIA_H