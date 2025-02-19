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
    if (err != ESP_OK) return err;
    return i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
}

int16_t ina219_read_register(uint8_t reg) {
    // uint8_t data[2];
    // i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    // i2c_master_start(cmd);
    // i2c_master_write_byte(cmd, (INA219_ADDRESS << 1) | I2C_MASTER_WRITE, true);
    // i2c_master_write_byte(cmd, reg, true);
    // i2c_master_start(cmd);
    // i2c_master_write_byte(cmd, (INA219_ADDRESS << 1) | I2C_MASTER_READ, true);
    // i2c_master_read(cmd, data, 2, I2C_MASTER_LAST_NACK);
    // i2c_master_stop(cmd);
    // i2c_cmd_link_delete(cmd);
    // //return (data[0] << 8) | data[1];

    // int16_t combined_data = (data[0] << 8) | data[1];

    // // Imprimir los bytes combinados en el terminal (usando printf)
    // printf("Registro 0x%02X: 0x%04X\n", reg, combined_data);

    // return combined_data;
    uint8_t data[2];
    esp_err_t err;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    err = i2c_master_start(cmd);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error en i2c_master_start: %s", esp_err_to_name(err));
        i2c_cmd_link_delete(cmd);
        return 0; // O un valor de error apropiado
    }

    err = i2c_master_write_byte(cmd, (INA219_ADDRESS << 1) | I2C_MASTER_WRITE, true);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error en i2c_master_write_byte (dirección): %s", esp_err_to_name(err));
        i2c_cmd_link_delete(cmd);
        return 0;
    }

    err = i2c_master_write_byte(cmd, reg, true);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error en i2c_master_write_byte (registro): %s", esp_err_to_name(err));
        i2c_cmd_link_delete(cmd);
        return 0;
    }

    err = i2c_master_start(cmd); // RESTART
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error en i2c_master_start (restart): %s", esp_err_to_name(err));
        i2c_cmd_link_delete(cmd);
        return 0;
    }

    err = i2c_master_write_byte(cmd, (INA219_ADDRESS << 1) | I2C_MASTER_READ, true);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error en i2c_master_write_byte (lectura): %s", esp_err_to_name(err));
        i2c_cmd_link_delete(cmd);
        return 0;
    }

    err = i2c_master_read(cmd, data, 2, I2C_MASTER_LAST_NACK);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error en i2c_master_read: %s", esp_err_to_name(err));
        i2c_cmd_link_delete(cmd);
        return 0;
    }

    err = i2c_master_stop(cmd);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error en i2c_master_stop: %s", esp_err_to_name(err));
        i2c_cmd_link_delete(cmd);
        return 0;
    }

    i2c_cmd_link_delete(cmd);

    int16_t combined_data = (data[0] << 8) | data[1];
    printf("Registro 0x%02X: 0x%04X\n", reg, combined_data);
    return combined_data;

}

float ina219_get_shunt_voltage() {
    int16_t value = ina219_read_register(INA219_REG_SHUNT_VOLTAGE);
    return (float)value * 0.01f; // Conversión a float y escalado

    //return ina219_read_register(INA219_REG_SHUNT_VOLTAGE) * 0.01;
}

float ina219_get_bus_voltage() {
    return (ina219_read_register(INA219_REG_BUS_VOLTAGE) >> 3) * 4.0;
}

float ina219_get_current() {
    return ina219_read_register(INA219_REG_CURRENT) * 0.001;
}

float ina219_get_power(){
    return ina219_read_register(INA219_REG_POWER);
}

void ina219_log_data() {
    float shunt_voltage = ina219_get_shunt_voltage();
    float bus_voltage = ina219_get_bus_voltage();
    float current = ina219_get_current();
    float power = ina219_get_power();

    ESP_LOGI(TAG, "Shunt Voltage: %.2f mV", shunt_voltage);
    ESP_LOGI(TAG, "Bus Voltage: %.2f V", bus_voltage);
    ESP_LOGI(TAG, "Current: %.3f A", current);
    ESP_LOGI(TAG, "Power: %.3f W", power);

    FILE *file = fopen("/spiffs/ina219_log.txt", "a");
    if (file) {
        fprintf(file, "Shunt Voltage: %.2f mV\n", shunt_voltage);
        fprintf(file, "Bus Voltage: %.2f V\n", bus_voltage);
        fprintf(file, "Current: %.3f A\n", current);
        fprintf(file, "Power: %.3f W\n\n", power);
        fclose(file);
    } else {
        ESP_LOGE(TAG, "Error al abrir el archivo de log");
    }
}
