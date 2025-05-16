#include <stdio.h>
#include "shtc3.h"
#include "sr04.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main() {
    shtc3_init();
    sr04_init();

    while (1) {
        float tempC = 0;
        float distance = 0;

        if (shtc3_measure(&tempC) == ESP_OK && sr04_get_distance(&distance) == ESP_OK) {
            float speed = 331.3 + 0.6 * tempC;
            printf("Distance: %.2f cm at %.1f°C\n", distance, tempC);
        } else {
            printf("Sensor read error\n");
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
