#ifndef I2C_COM_H
#define I2C_COM_H

#include <stdint.h>
#include "driver/i2c.h"

#define I2C_MASTER_SCL_IO    6  // Pin SCL
#define I2C_MASTER_SDA_IO    7  // Pin SDA
#define I2C_MASTER_FREQ_HZ   100000  // Frecuencia I2C 100kHz
#define I2C_MASTER_NUM       I2C_NUM_0  // Bus I2C
#define I2C_TIMEOUT_MS       1000  // Timeout en milisegundos

#define INA219_ADDRESS       0x40  // Dirección por defecto del INA219
#define INA219_REG_SHUNT_VOLTAGE 0x01
#define INA219_REG_BUS_VOLTAGE   0x02
#define INA219_REG_POWER       0x03
#define INA219_REG_CURRENT       0x04

esp_err_t i2c_master_init();
int16_t ina219_read_register(uint8_t reg);
float ina219_get_shunt_voltage();
float ina219_get_bus_voltage();
float ina219_get_current();
float ina219_get_power();
void ina219_log_data();

#endif
