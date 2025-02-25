#include "buzzer_rgb.h"


void baliza_zumbador_init(void) {
    gpio_set_direction(RGB_RED_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(RGB_GREEN_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(RGB_BLUE_GPIO, GPIO_MODE_OUTPUT);

    ledc_timer_config_t ledc_timer = {
        .speed_mode = BUZZER_LEDC_MODE,
        .timer_num = BUZZER_LEDC_TIMER,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .freq_hz = BUZZER_FREQUENCY,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = {
        .speed_mode = BUZZER_LEDC_MODE,
        .channel = BUZZER_LEDC_CHANNEL,
        .timer_sel = BUZZER_LEDC_TIMER,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = BUZZER_GPIO,
        .duty = 0,
        .hpoint = 0
    };
    ledc_channel_config(&ledc_channel);

    set_baliza_color(0, 0, 0);
}

void activar_zumbador(int frecuencia) {
    ledc_set_freq(BUZZER_LEDC_MODE, BUZZER_LEDC_TIMER, frecuencia);
    ledc_set_duty(BUZZER_LEDC_MODE, BUZZER_LEDC_CHANNEL, 512); // 50% duty cycle
    ledc_update_duty(BUZZER_LEDC_MODE, BUZZER_LEDC_CHANNEL);
}

void desactivar_zumbador(void) {
    ledc_set_duty(BUZZER_LEDC_MODE, BUZZER_LEDC_CHANNEL, 0);
    ledc_update_duty(BUZZER_LEDC_MODE, BUZZER_LEDC_CHANNEL);
}

void activar_zumbador_tono(int frecuencia, int duracion_ms) {
    activar_zumbador(frecuencia);
    vTaskDelay(pdMS_TO_TICKS(duracion_ms));
    desactivar_zumbador();
}

void set_baliza_color(uint8_t red, uint8_t green, uint8_t blue) {
    gpio_set_level(RGB_RED_GPIO, red);
    gpio_set_level(RGB_GREEN_GPIO, green);
    gpio_set_level(RGB_BLUE_GPIO, blue);
}

void baliza_zumbador_socorro(void) {
    for (int i = 0; i < 3; i++) {
        set_baliza_color(1, 0, 0);  // Rojo
        activar_zumbador_tono(1000, 5000);
        set_baliza_color(0, 0, 0);
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void baliza_zumbador_estado_ok(void) {
    for (int i = 0; i < 5; i++) {
        set_baliza_color(0, 1, 0);  // Verde
        vTaskDelay(pdMS_TO_TICKS(500));
        set_baliza_color(0, 0, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void baliza_zumbador_fuera_limites(void) {
    for (int i = 0; i < 5; i++) {
        set_baliza_color(1, 0.5, 0);  // Naranja
        vTaskDelay(pdMS_TO_TICKS(500));
        set_baliza_color(0, 0, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void baliza_zumbador_bajo_bateria(void) {
    for (int i = 0; i < 10; i++) {
        set_baliza_color(1, 1, 0);  // Amarillo
        activar_zumbador_tono(1500, 300);
        set_baliza_color(0, 0, 0);
        vTaskDelay(pdMS_TO_TICKS(300));
    }
}

void baliza_zumbador_error_sistema(void) {
    for (int i = 0; i < 5; i++) {
        set_baliza_color(1, 0, 1);  // Magenta
        activar_zumbador_tono(500, 1000);
        set_baliza_color(0, 0, 0);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void baliza_zumbador_transmision_datos(void) {
    for (int i = 0; i < 3; i++) {
        set_baliza_color(0, 0, 1);  // Azul
        vTaskDelay(pdMS_TO_TICKS(200));
        set_baliza_color(0, 0, 0);
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}
