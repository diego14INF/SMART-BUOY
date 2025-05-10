#include "siren_rgb.h"

void baliza_sirena_init(void) {
    gpio_set_direction(RGB_RED_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(RGB_GREEN_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(RGB_BLUE_GPIO, GPIO_MODE_OUTPUT);

    gpio_set_direction(RELE_SIRENA_GPIO, GPIO_MODE_OUTPUT);  // Nuevo: salida para relé
    gpio_set_level(RELE_SIRENA_GPIO, 0);  // Asegurarse que arranca apagado

    set_baliza_color(0, 0, 0);
}

void activar_sirena(void) {
    gpio_set_level(RELE_SIRENA_GPIO, 1);  // Enciende la sirena (relé activo)
}

void desactivar_sirena(void) {
    gpio_set_level(RELE_SIRENA_GPIO, 0);  // Apaga la sirena
}

void activar_sirena_duracion(int duracion_ms) {
    activar_sirena();
    vTaskDelay(pdMS_TO_TICKS(duracion_ms));
    desactivar_sirena();
}

void set_baliza_color(uint8_t red, uint8_t green, uint8_t blue) {
    gpio_set_level(RGB_RED_GPIO, red);
    gpio_set_level(RGB_GREEN_GPIO, green);
    gpio_set_level(RGB_BLUE_GPIO, blue);
}

// Las siguientes funciones usan la sirena en lugar del zumbador

void baliza_sirena_socorro(void) {
    // Tres puntos "S" (...)
    for (int i = 0; i < 3; i++) {
        set_baliza_color(1, 0, 0);  // Rojo
        activar_sirena_duracion(300); // Punto (~300ms)
        set_baliza_color(0, 0, 0);
        vTaskDelay(pdMS_TO_TICKS(300));
    }

    vTaskDelay(pdMS_TO_TICKS(600)); // Espacio entre letras

    // Tres rayas "O" (---)
    for (int i = 0; i < 3; i++) {
        set_baliza_color(1, 0, 0);  // Rojo
        activar_sirena_duracion(900); // Raya (~900ms)
        set_baliza_color(0, 0, 0);
        vTaskDelay(pdMS_TO_TICKS(300));
    }

    vTaskDelay(pdMS_TO_TICKS(600)); // Espacio entre letras

    // Tres puntos "S" (...)
    for (int i = 0; i < 3; i++) {
        set_baliza_color(1, 0, 0);  // Rojo
        activar_sirena_duracion(300); // Punto (~300ms)
        set_baliza_color(0, 0, 0);
        vTaskDelay(pdMS_TO_TICKS(300));
    }

    vTaskDelay(pdMS_TO_TICKS(1000));

    desactivar_sirena();

}

void baliza_sirena_estado_ok(void) {
    for (int i = 0; i < 2 ; i++) {
        set_baliza_color(0, 1, 0);  // Verde
        vTaskDelay(pdMS_TO_TICKS(500));
        set_baliza_color(0, 0, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void baliza_sirena_bajo_bateria(void) {
    for (int i = 0; i < 2; i++) {
        set_baliza_color(1, 1, 0);  // Amarillo
        activar_sirena_duracion(500);
        set_baliza_color(0, 0, 0);
        vTaskDelay(pdMS_TO_TICKS(300));
    }
}

void baliza_sirena_transmision_datos(void) {
    for (int i = 0; i < 2; i++) {
        set_baliza_color(0, 0, 1);  // Azul
        vTaskDelay(pdMS_TO_TICKS(500));
        set_baliza_color(0, 0, 0);
        vTaskDelay(pdMS_TO_TICKS(1500));
    }
}

void baliza_sirena_emparejamiento(void) {
    for (int i = 0; i < 5; i++) {
        set_baliza_color(1, 0, 1);  // ???
        vTaskDelay(pdMS_TO_TICKS(500));
        set_baliza_color(0, 0, 0);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}


void baliza_sirena_error_sistema(void) {
    for (int i = 0; i < 5; i++) {
        set_baliza_color(1, 0, 1);  // Magenta
        activar_sirena_duracion(1000);
        set_baliza_color(0, 0, 0);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// void init_led_ring(void) {
//     led_strip_config_t strip_config = {
//         .strip_gpio_num = LED_STRIP_GPIO,
//         .max_leds = LED_STRIP_NUM_LEDS,
//         .led_pixel_format = LED_PIXEL_FORMAT_GRB,
//         .led_model = LED_MODEL_WS2812,
//         .flags.invert_out = false
//     };
//     led_strip_rmt_config_t rmt_config = {
//         .clk_src = RMT_CLK_SRC_DEFAULT,
//         .resolution_hz = 10 * 1000 * 1000,
//         .mem_block_symbols = 64,
//         .flags.with_dma = false
//     };
//     led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip);
//     led_strip_clear(led_strip);
// }

// void mostrar_direccion_led(float angulo) {
//     // Normalizar el ángulo a [0, 360)
//     while (angulo < 0) angulo += 360;
//     while (angulo >= 360) angulo -= 360;

//     int led_central = (int)((angulo / 360.0f) * LED_STRIP_NUM_LEDS + 0.5f) % LED_STRIP_NUM_LEDS;
//     int led_izquierdo = (led_central - 1 + LED_STRIP_NUM_LEDS) % LED_STRIP_NUM_LEDS;
//     int led_derecho = (led_central + 1) % LED_STRIP_NUM_LEDS;

//     led_strip_clear(led_strip);  // Apagar todos primero

//     // Encender el LED central en rojo
//     led_strip_set_pixel(led_strip, led_central, 255, 0, 0);

//     // Encender los laterales en naranja (255, 100, 0)
//     led_strip_set_pixel(led_strip, led_izquierdo, 255, 100, 0);
//     led_strip_set_pixel(led_strip, led_derecho, 255, 100, 0);

//     led_strip_refresh(led_strip);
// }