#include "sr04.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_rom_sys.h"
#include "driver/i2c.h"
#define TIMEOUT_US 30000 //30 ms timeout

#define TRIG_PIN GPIO_NUM_3
#define ECHO_PIN GPIO_NUM_4

void sr04_init() {
    gpio_set_direction(TRIG_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(ECHO_PIN, GPIO_MODE_INPUT);
}

esp_err_t sr04_get_pulse_duration(float *duration_us) {
    int64_t start_time, end_time;

    gpio_set_level(TRIG_PIN, 0);
    esp_rom_delay_us(2);
    gpio_set_level(TRIG_PIN, 1); // send pulse
    esp_rom_delay_us(10); // pulse is high for 10 microseconds
    gpio_set_level(TRIG_PIN, 0);

    int64_t timeout = esp_timer_get_time() + TIMEOUT_US;
    while (gpio_get_level(ECHO_PIN) == 0) {
        if (esp_timer_get_time() > timeout) {
            return ESP_ERR_TIMEOUT;
        }
    }
    start_time = esp_timer_get_time();

    timeout = esp_timer_get_time() + TIMEOUT_US;
    while (gpio_get_level(ECHO_PIN) == 1) {
        if (esp_timer_get_time() > timeout) {
            return ESP_ERR_TIMEOUT;
        }
    }
    end_time = esp_timer_get_time();

    *duration_us = (float)(end_time - start_time);
    return ESP_OK;
}

esp_err_t sr04_get_average_pulse_duration(float *avg_duration_us, int samples) {
    float total = 0;
    int count = 0;
    for (int i = 0; i < samples; i++) {
        float dur;
        if (sr04_get_pulse_duration(&dur) == ESP_OK) {
            total += dur;
            count++;
        }
        vTaskDelay(pdMS_TO_TICKS(50));  // short delay between pulses
    }

    if (count == 0) return ESP_ERR_TIMEOUT;

    *avg_duration_us = total / count;
    return ESP_OK;
}
