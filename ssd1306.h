#ifndef __SSD1306_H__
#define __SSD1306_H__

#include <espos_types.h>
#include <driver/gpio.h>
#include <driver/i2c.h>

#define SDA_PIN GPIO_NUM_12
#define SCL_PIN GPIO_NUM_14
#define SSD1306_PORT I2C_NUM_0

esp_err_t ssd1306_on(bool flag);
esp_err_t ssd1306_init();
void ssd1306_draw();
void ssd1306_clear();
#endif


