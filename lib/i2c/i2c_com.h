#ifndef I2C_COM_H
#define I2C_COM_H

#include <stdint.h>
#include "driver/i2c.h"
#include "driver/i2c_master.h"

#define I2C_MASTER_SCL_IO    6  // Pin SCL
#define I2C_MASTER_SDA_IO    7  // Pin SDA
#define I2C_MASTER_FREQ_HZ   100000  // Frecuencia I2C 100kHz
#define I2C_MASTER_NUM       I2C_NUM_0  // Bus I2C
#define I2C_TIMEOUT_MS       1000  // Timeout en milisegundos

#define INA219_ADDRESS_SIM808       0x40  // SIM808
#define INA219_ADDRESS_BATERIA  0x41  // Bateria
#define INA219_ADDRESS_SIRENA  0x42  // Sirena
#define INA219_ADDRESS_LEDS  0x43  // Baliza y tira LED

#define INA219_REG_SHUNT_VOLTAGE 0x01
#define INA219_REG_BUS_VOLTAGE   0x02
#define INA219_REG_POWER       0x03
#define INA219_REG_CURRENT       0x04
#define INA219_REG_CALIBRATION 0X05

esp_err_t i2c_master_init();
esp_err_t ina219_leer_registro(uint8_t ina219_adrress, uint8_t reg, uint16_t *data);
esp_err_t ina219_calibracion(float shunt_value, float max_expected_current);
float ina219_leer_voltaje_shunt(uint8_t ina219_adrress);
float ina219_leer_voltaje_bus(uint8_t ina219_adrress);
float ina219_leer_corriente(uint8_t ina219_adrress);
float ina219_leer_potencia(uint8_t ina219_adrress);
void ina219_log_data();


#endif
