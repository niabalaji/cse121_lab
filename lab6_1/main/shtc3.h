#ifndef SHTC3_H
#define SHTC3_H

#include "esp_err.h"

void shtc3_init();
esp_err_t shtc3_measure(float *temperature_c);

#endif
