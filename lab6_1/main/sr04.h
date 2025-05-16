#ifndef SR04_H
#define SR04_H

#include "esp_err.h"

void sr04_init();
esp_err_t sr04_get_distance(float *distance_cm);

#endif
