#include "i2c_com.h"
#include "esp_log.h"

static const char *TAG = "I2C_COM";

esp_err_t i2c_master_init() {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
  
    esp_err_t err = i2c_param_config(I2C_MASTER_NUM, &conf);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Error configuring I2C parameters");
      return err;
    }
  
    return i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
  }

  esp_err_t ina219_leer_registro(uint8_t ina219_adrress, uint8_t reg, uint16_t *data) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    esp_err_t ret;

    // Escribir dirección y registro
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ina219_adrress << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);

    // Reiniciar para lectura
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ina219_adrress << 1) | I2C_MASTER_READ, true);

    uint8_t buffer[2];
    i2c_master_read(cmd, buffer, 2, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);

    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(I2C_TIMEOUT_MS));
    i2c_cmd_link_delete(cmd);

    if (ret == ESP_OK) {
        *data = (buffer[0] << 8) | buffer[1]; // Combina MSB y LSB
    }

    return ret;
}

esp_err_t ina219_calibracion(float shunt_value, float max_expected_current) {
  uint16_t calibration_value;
  float current_lsb;

  // Calcular el valor de calibración
  current_lsb = max_expected_current / 32768.0f; // 32768 = 2^15
 
  calibration_value = (uint16_t)(0.04096f / (current_lsb * shunt_value));

  // Escribir el valor de calibración en el registro de calibración
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  esp_err_t ret;

  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (INA219_ADDRESS_SIM808 << 1) | I2C_MASTER_WRITE, true);
  i2c_master_write_byte(cmd, INA219_REG_CALIBRATION, true);
  i2c_master_write_byte(cmd, (calibration_value >> 8) & 0xFF, true); // MSB
  i2c_master_write_byte(cmd, calibration_value & 0xFF, true); // LSB
  i2c_master_stop(cmd);

  ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(I2C_TIMEOUT_MS));
  i2c_cmd_link_delete(cmd);

  if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Error writing calibration register");
  } else {
      ESP_LOGI(TAG, "INA219 calibrated successfully");
  }

  return ret;
}


float ina219_leer_voltaje_shunt(uint8_t ina219_adrress) {
  uint16_t raw = 0;
  if (ina219_leer_registro(ina219_adrress, INA219_REG_SHUNT_VOLTAGE, &raw) == ESP_OK) {
      return (float)((int16_t)raw) * 0.01f;  // mV
  }
  return 0.0f;
}

float ina219_leer_voltaje_bus(uint8_t ina219_adrress) {
  uint16_t raw = 0;
  if (ina219_leer_registro(ina219_adrress, INA219_REG_BUS_VOLTAGE, &raw) == ESP_OK) {
      return ((raw >> 3) * 4.0f) / 1000.0f;  // V
  }
  return 0.0f;
}

float ina219_leer_corriente(uint8_t ina219_adrress) {
  uint16_t raw = 0;
  if (ina219_leer_registro(ina219_adrress, INA219_REG_CURRENT, &raw) == ESP_OK) {
      return (float)((int16_t)raw) * 0.001f; // A
  }
  return 0.0f;
}

float ina219_leer_potencia(uint8_t ina219_adrress) {
  uint16_t raw = 0;
  if (ina219_leer_registro(ina219_adrress, INA219_REG_POWER, &raw) == ESP_OK) {
      return (float)raw * 0.02f; // W
  }
  return 0.0f;
}

void ina219_log_data() {
    printf("\n--- INA219 SENSOR READINGS ---\n");

    // Lista de direcciones de los sensores
    uint8_t ina219_addresses[] = {INA219_ADDRESS_SIM808, INA219_ADDRESS_BATERIA, INA219_ADDRESS_SIRENA, INA219_ADDRESS_LEDS};

    // Iterar sobre cada sensor INA219 y obtener sus lecturas
    for (int i = 0; i < 4; i++) {
        uint8_t addr = ina219_addresses[i];

        float shunt_voltage = ina219_leer_voltaje_shunt(addr);
        float bus_voltage = ina219_leer_voltaje_bus(addr);
        float current = ina219_leer_corriente(addr);
        float power = ina219_leer_potencia(addr);

        // printf("INA219 (Addr 0x%X)\n", addr);
        // printf("  - Shunt Voltage: %.2f mV\n", shunt_voltage);
        // printf("  - Bus Voltage: %.2f V\n", bus_voltage);
        // printf("  - Current: %.3f A\n", current);
        // printf("  - Power: %.3f W\n", power);
        // printf("----------------------------\n");
    }
}

