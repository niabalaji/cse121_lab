#include "shtc3.h"
#include "driver/i2c.h"

#define I2C_MASTER_SCL_IO 0
#define I2C_MASTER_SDA_IO 1
#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_FREQ_HZ 100000
#define SHTC3_ADDR 0x70

void shtc3_init() {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ
    };
    i2c_param_config(I2C_MASTER_NUM, &conf);
    i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);

    uint8_t wake_cmd[] = { 0x35, 0x17 };
    i2c_master_write_to_device(I2C_MASTER_NUM, SHTC3_ADDR, wake_cmd, 2, 1000 / portTICK_PERIOD_MS);
}

esp_err_t shtc3_measure(float *temperature_c) {
    uint8_t cmd[] = { 0x7C, 0xA2 };
    uint8_t data[6];

    if (i2c_master_write_to_device(I2C_MASTER_NUM, SHTC3_ADDR, cmd, 2, 1000 / portTICK_PERIOD_MS) != ESP_OK)
        return ESP_FAIL;

    vTaskDelay(pdMS_TO_TICKS(20));

    if (i2c_master_read_from_device(I2C_MASTER_NUM, SHTC3_ADDR, data, 6, 1000 / portTICK_PERIOD_MS) != ESP_OK)
        return ESP_FAIL;

    uint16_t raw_temp = (data[0] << 8) | data[1];
    *temperature_c = -45 + 175 * ((float)raw_temp / 65535.0);

    return ESP_OK;
}
