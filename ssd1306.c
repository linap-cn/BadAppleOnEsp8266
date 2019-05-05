#include "ssd1306.h"
#include <string.h>

#define OLED_I2C_ADDRESS   0x3C

// Control byte
#define OLED_CONTROL_BYTE_CMD_SINGLE    0x80
#define OLED_CONTROL_BYTE_CMD_STREAM    0x00
#define OLED_CONTROL_BYTE_DATA_STREAM   0x40

// Fundamental commands (pg.28)
#define OLED_CMD_SET_CONTRAST           0x81    // follow with 0x7F
#define OLED_CMD_DISPLAY_RAM            0xA4
#define OLED_CMD_DISPLAY_ALLON          0xA5
#define OLED_CMD_DISPLAY_NORMAL         0xA6
#define OLED_CMD_DISPLAY_INVERTED       0xA7
#define OLED_CMD_DISPLAY_OFF            0xAE
#define OLED_CMD_DISPLAY_ON             0xAF

// Addressing Command Table (pg.30)
#define OLED_CMD_SET_MEMORY_ADDR_MODE   0x20    // follow with 0x00 = HORZ mode = Behave like a KS108 graphic LCD
#define OLED_CMD_SET_COLUMN_RANGE       0x21    // can be used only in HORZ/VERT mode - follow with 0x00 and 0x7F = COL127
#define OLED_CMD_SET_PAGE_RANGE         0x22    // can be used only in HORZ/VERT mode - follow with 0x00 and 0x07 = PAGE7

// Hardware Config (pg.31)
#define OLED_CMD_SET_DISPLAY_START_LINE 0x40
#define OLED_CMD_SET_SEGMENT_REMAP      0xA1
#define OLED_CMD_SET_MUX_RATIO          0xA8    // follow with 0x3F = 64 MUX
#define OLED_CMD_SET_COM_SCAN_MODE      0xC8
#define OLED_CMD_SET_DISPLAY_OFFSET     0xD3    // follow with 0x00
#define OLED_CMD_SET_COM_PIN_MAP        0xDA    // follow with 0x12
#define OLED_CMD_NOP                    0xE3    // NOP

// Timing and Driving Scheme (pg.32)
#define OLED_CMD_SET_DISPLAY_CLK_DIV    0xD5    // follow with 0x80
#define OLED_CMD_SET_PRECHARGE          0xD9    // follow with 0xF1
#define OLED_CMD_SET_VCOMH_DESELCT      0xDB    // follow with 0x30

// Charge Pump (pg.62)
#define OLED_CMD_SET_CHARGE_PUMP        0x8D    // follow with 0x14


uint8_t invert = OLED_CMD_DISPLAY_NORMAL;//0xA6 //OLED_CMD_DISPLAY_INVERTED;//0xA7

//******************************************************************************************

void i2c_ssd1306_init()
{
	i2c_config_t i2c_config = {
	.mode = I2C_MODE_MASTER,
	.sda_io_num = SDA_PIN,
	.scl_io_num = SCL_PIN,
	.sda_pullup_en = GPIO_PULLUP_ENABLE,
	.scl_pullup_en = GPIO_PULLUP_ENABLE
	};
	i2c_driver_install(SSD1306_PORT, I2C_MODE_MASTER);
	i2c_param_config(SSD1306_PORT, &i2c_config);
}
//-----------------------------------------------------------------------------------------
esp_err_t ssd1306_on(bool flag)
{
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	uint8_t bt;

	if (flag) bt = OLED_CMD_DISPLAY_ON; else bt = OLED_CMD_DISPLAY_OFF;

	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
	i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_SINGLE, true);
	i2c_master_write_byte(cmd, bt, true);
	i2c_master_stop(cmd);
	esp_err_t ret = i2c_master_cmd_begin(SSD1306_PORT, cmd, 20/portTICK_PERIOD_MS);
	if (ret != ESP_OK) {
	if (flag) {
	   printf("[%s] Display ON ERROR ! (0x%.2X)", __func__, ret);
	} else {
	   printf("[%s] Display OFF ERROR ! (0x%.2X)", __func__, ret);
	}
	}
	i2c_cmd_link_delete(cmd);

	return ret;
}
//-----------------------------------------------------------------------------------------
esp_err_t ssd1306_init()
{
	i2c_ssd1306_init();
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();

	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
	i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);

	i2c_master_write_byte(cmd, OLED_CMD_SET_CHARGE_PUMP, true);
	i2c_master_write_byte(cmd, 0x14, true);

	i2c_master_write_byte(cmd, OLED_CMD_SET_SEGMENT_REMAP, true); // reverse left-right mapping
	i2c_master_write_byte(cmd, OLED_CMD_SET_COM_SCAN_MODE, true); // reverse up-bottom mapping

	i2c_master_write_byte(cmd, OLED_CMD_SET_COLUMN_RANGE, true);
	i2c_master_write_byte(cmd, 0x00, true);
	i2c_master_write_byte(cmd, 0x7F, true);
	i2c_master_write_byte(cmd, OLED_CMD_SET_PAGE_RANGE, true);
	i2c_master_write_byte(cmd, 0x00, true);
	i2c_master_write_byte(cmd, 0x07, true);

	i2c_master_write_byte(cmd, OLED_CMD_DISPLAY_ON, true);

	i2c_master_write_byte(cmd, invert, true);//OLED_CMD_DISPLAY_INVERTED	   0xA7

	i2c_master_stop(cmd);

	esp_err_t ret = i2c_master_cmd_begin(SSD1306_PORT, cmd, 20/portTICK_PERIOD_MS);
	if (ret != ESP_OK) {
		printf("[%s] Display configuration failed. code: 0x%.2X", __func__, ret);
	}
	i2c_cmd_link_delete(cmd);

	return ret;
}

//******************************************************************************************

uint8_t buffer[128 * 64 / 8];
void ssd1306_clear()
{
	memset(buffer,0,sizeof(buffer));
	i2c_cmd_handle_t cmd;
	uint8_t i;
	cmd = i2c_cmd_link_create();
	for (i = 0; i < 8; i++) {
		i2c_master_start(cmd);

		i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
		i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_SINGLE, true);
		i2c_master_write_byte(cmd, 0xB0 | i, true);
		i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);
		i2c_master_write(cmd, buffer+i*128, 128,true);
		i2c_master_stop(cmd);
	}
	i2c_master_cmd_begin(SSD1306_PORT, cmd, 0);
	i2c_cmd_link_delete(cmd);
}
void ssd1306_draw()
{
	i2c_cmd_handle_t cmd;
	uint8_t i;
	cmd = i2c_cmd_link_create();
	for (i = 2; i < 8; i++) {
		i2c_master_start(cmd);

		i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
		i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_SINGLE, true);
		i2c_master_write_byte(cmd, 0xB0 | i, true);
		i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);
		i2c_master_write(cmd, buffer+i*128, 128,true);
		i2c_master_stop(cmd);
	}
	i2c_master_cmd_begin(SSD1306_PORT, cmd, 0);
	i2c_cmd_link_delete(cmd);
}

