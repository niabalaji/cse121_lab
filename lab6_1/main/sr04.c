#include "sr04.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_rom_sys.h"

#define TRIG_PIN GPIO_NUM_3
#define ECHO_PIN GPIO_NUM_4

void sr04_init() {
    gpio_set_direction(TRIG_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(ECHO_PIN, GPIO_MODE_INPUT);
}

esp_err_t sr04_get_distance(float *distance_cm) {
    int64_t start_time, end_time;

    gpio_set_level(TRIG_PIN, 0);
    esp_rom_delay_us(2);
    gpio_set_level(TRIG_PIN, 1);
    esp_rom_delay_us(10);
    gpio_set_level(TRIG_PIN, 0);

    while (gpio_get_level(ECHO_PIN) == 0);
    start_time = esp_timer_get_time();

    while (gpio_get_level(ECHO_PIN) == 1);
    end_time = esp_timer_get_time();

    float duration_us = (float)(end_time - start_time);
    float distance = (duration_us * 0.0343) / 2.0;  // Assuming default speed of sound

    *distance_cm = distance;
    return ESP_OK;
}
