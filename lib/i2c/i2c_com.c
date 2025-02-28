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

  esp_err_t ina219_read_register(uint8_t reg, uint16_t *data) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    esp_err_t ret;

    // Escribir dirección y registro
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (INA219_ADDRESS << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);

    // Reiniciar para lectura
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (INA219_ADDRESS << 1) | I2C_MASTER_READ, true);

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

esp_err_t ina219_calibrate(float shunt_value, float max_expected_current) {
  uint16_t calibration_value;
  float current_lsb;
  float power_lsb;

  // Calcular el valor de calibración
  current_lsb = max_expected_current / 32768.0f; // 32768 = 2^15
  power_lsb = 20 * current_lsb; // power_lsb = current_lsb * 20

  calibration_value = (uint16_t)(0.04096f / (current_lsb * shunt_value));

  // Escribir el valor de calibración en el registro de calibración
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  esp_err_t ret;

  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (INA219_ADDRESS << 1) | I2C_MASTER_WRITE, true);
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


float ina219_get_shunt_voltage() {
  uint16_t raw = 0;
  if (ina219_read_register(INA219_REG_SHUNT_VOLTAGE, &raw) == ESP_OK) {
      return (float)((int16_t)raw) * 0.01f;  // mV
  }
  return 0.0f;
}

float ina219_get_bus_voltage() {
  uint16_t raw = 0;
  if (ina219_read_register(INA219_REG_BUS_VOLTAGE, &raw) == ESP_OK) {
      return ((raw >> 3) * 4.0f) / 1000.0f;  // V
  }
  return 0.0f;
}

float ina219_get_current() {
  uint16_t raw = 0;
  if (ina219_read_register(INA219_REG_CURRENT, &raw) == ESP_OK) {
      return (float)((int16_t)raw) * 0.001f; // A
  }
  return 0.0f;
}

float ina219_get_power() {
  uint16_t raw = 0;
  if (ina219_read_register(INA219_REG_POWER, &raw) == ESP_OK) {
      return (float)raw * 0.02f; // W
  }
  return 0.0f;
}

void ina219_log_data() {
    float shunt_voltage = ina219_get_shunt_voltage();
    //ESP_LOGI(TAG, "Shunt Voltage: %.2f mV", shunt_voltage);
    

    float bus_voltage = ina219_get_bus_voltage();
    //ESP_LOGI(TAG, "Bus Voltage: %.2f V", bus_voltage);
 

    float current = ina219_get_current();
    //ESP_LOGI(TAG, "Current: %.3f A", current);
   
    float power = ina219_get_power();
    //ESP_LOGI(TAG, "Power: %.3f W", power);

    //vTaskDelay(pdMS_TO_TICKS(2000));  // Espera innecesaria ya la tengo en el main// 2 segundos


    //  FILE *file = fopen("/spiffs/ina219_log.txt", "a");
    //  if (file) {
    //      fprintf(file, "Shunt Voltage: %.2f mV\n", shunt_voltage);
    //      fprintf(file, "Bus Voltage: %.2f V\n", bus_voltage);
    //      fprintf(file, "Current: %.3f A\n", current);
    //      fprintf(file, "Power: %.3f W\n\n", power);
    //      fclose(file);
    //  } else {
    //      ESP_LOGE(TAG, "Error al abrir el archivo de log");
    //  }
}
