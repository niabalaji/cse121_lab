#include <stdio.h>
#include "shtc3.h"
#include "sr04.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"

#define I2C_MASTER_SCL_IO 0
#define I2C_MASTER_SDA_IO 1

void i2c_scan() {
    printf("Scanning I2C...\n");
    for (uint8_t addr = 1; addr < 127; addr++) {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, addr << 1, true);
        i2c_master_stop(cmd);
        esp_err_t err = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd);
        if (err == ESP_OK) {
            printf("Found device at 0x%02X\n", addr);
        }
    }
}

void app_main() {
    i2c_master_init();   // I2C bus setup
    i2c_scan();          // Confirm devices
    shtc3_init();        // Wake up temp sensor
    sr04_init();         // Set up ultrasonic GPIOs

    while (1) {
        float tempC, duration_us;
        esp_err_t temp_status = shtc3_measure(&tempC);
        esp_err_t dist_status = sr04_get_pulse_duration(&duration_us);

        if (temp_status == ESP_OK && dist_status == ESP_OK) {
            //if (sr04_get_average_pulse_duration(&duration_us, 5) == ESP_OK) {
                float speed = 331.3 + 0.6 * tempC;             // m/s
                float speed_cm_per_us = speed / 10000.0f;      // convert to cm/μs
                float distance_cm = (duration_us * speed_cm_per_us) / 2.0f;

                printf("Distance: %.2f cm at %.1f°C\n", distance_cm, tempC);
            //}    
        }  
        else {
            if (temp_status != ESP_OK) {
                printf("SHTC3 read failed\n");
            }
            if (dist_status != ESP_OK) {
                printf("SR04 timeout or read error\n");
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1000)); // every second
    }   
}

