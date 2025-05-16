#ifndef SR04_H
#define SR04_H

#include "esp_err.h"

void sr04_init();
esp_err_t sr04_get_pulse_duration(float *duration_us);
esp_err_t sr04_get_average_pulse_duration(float *avg_duration_us, int samples);

#endif
