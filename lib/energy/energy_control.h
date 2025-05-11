#ifndef ENERGY_CONTROL_H
#include <stdint.h>
#include <stdbool.h>

bool verificacion_nivel_bateria();
bool verificacion_estado_modulo(uint8_t ina219_addr);
void sim808_bateria_baja();


#endif // CONTROL_ENERGIA_H