#include <stdio.h>
#include "esp_log.h"
#include "driver/i2c.h"
#include "ssd1306.h"
#include "UIT_Logo_OLED_Display.h"

#define I2C_MASTER_SDA_IO 5
#define I2C_MASTER_SCL_IO 4
#define I2C_MASTER_FREQ_HZ 100000
#define I2C_MASTER_RX_BUF_DISABLE 0
#define I2C_MASTER_TX_BUF_DISABLE 0

static const char *TAG = "TAG";

void ssd1306_init();
void ssd1306_display_clear();
void ssd1306_display_UIT_Logo();

void app_main(void)
{
	int i2c_master_port = 0;
	i2c_config_t conf = {
		.mode = I2C_MODE_MASTER,
		.sda_io_num = I2C_MASTER_SDA_IO, // select GPIO specific to your project
		.sda_pullup_en = GPIO_PULLUP_ENABLE,
		.scl_io_num = I2C_MASTER_SCL_IO, // select GPIO specific to your project
		.scl_pullup_en = GPIO_PULLUP_ENABLE,
		.master.clk_speed = I2C_MASTER_FREQ_HZ, // select frequency specific to your project
												//.clk_flags = 0,          /*!< Optional, you can use I2C_SCLK_SRC_FLAG_* flags to choose i2c source clock here. */
	};
	i2c_param_config(i2c_master_port, &conf);
	i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);

	ssd1306_init();
	ssd1306_display_clear();
	ssd1306_display_UIT_Logo();
}

void ssd1306_init()
{
	esp_err_t espRc;

	// Tạo một command handle để gửi các command tới OLED
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	// Bắt đầu truyền tín hiệu i2c đến OLED
	i2c_master_start(cmd);
	// Gửi địa chỉ I2C của OLED và bit R/W = 0 (WRITE)
	i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
	// Gửi byte điều khiển để đặt OLED vào chế độ command stream
	i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);
	// Đặt công suất cho OLED bằng lệnh SET_CHARGE_PUMP và giá trị 0x14
	i2c_master_write_byte(cmd, OLED_CMD_SET_CHARGE_PUMP, true);
	i2c_master_write_byte(cmd, 0x14, true);
	// Đặt chế độ đảo left-right mapping bằng lệnh SET_SEGMENT_REMAP
	i2c_master_write_byte(cmd, OLED_CMD_SET_SEGMENT_REMAP, true);
	// Đặt chế độ đảo up-bottom mapping bằng lệnh SET_COM_SCAN_MODE
	i2c_master_write_byte(cmd, OLED_CMD_SET_COM_SCAN_MODE, true);
	// Đặt chế độ hiển thị bằng lệnh DISPLAY_NORMAL, sau đó tắt và bật hiển thị với lệnh DISPLAY_OFF và DISPLAY_ON
	i2c_master_write_byte(cmd, OLED_CMD_DISPLAY_NORMAL, true);
	i2c_master_write_byte(cmd, OLED_CMD_DISPLAY_OFF, true);
	i2c_master_write_byte(cmd, OLED_CMD_DISPLAY_ON, true);
	// Kết thúc truyền tín hiệu i2c
	i2c_master_stop(cmd);
	// Gửi command tới OLED và chờ phản hồi
	espRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS);
	// Kiểm tra kết quả phản hồi từ OLED và log thông tin
	if (espRc == ESP_OK)
	{
		ESP_LOGI(TAG, "OLED configured successfully");
	}
	else
	{
		ESP_LOGE(TAG, "OLED configuration failed. code: 0x%.2X", espRc);
	}
	// Xóa command handle sau khi hoàn thành
	i2c_cmd_link_delete(cmd);
}
void ssd1306_display_clear()
{
	i2c_cmd_handle_t cmd;

	uint8_t clear[128];
	for (uint8_t i = 0; i < 128; i++)
	{
		clear[i] = 0;
	}
	for (uint8_t i = 0; i < 8; i++)
	{
		cmd = i2c_cmd_link_create();
		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
		i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_SINGLE, true);
		i2c_master_write_byte(cmd, 0xB0 | i, true);

		i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);
		i2c_master_write(cmd, clear, 128, true);
		i2c_master_stop(cmd);
		i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS);
		i2c_cmd_link_delete(cmd);
	}
}
void ssd1306_display_UIT_Logo()
{
	uint8_t cur_page = 0;
	i2c_cmd_handle_t cmd;
	int i;
	for (i = 0; i < 8; i++)
	{
		cmd = i2c_cmd_link_create();
		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
		i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);
		i2c_master_write_byte(cmd, 0x00, true); // reset column
		i2c_master_write_byte(cmd, 0x10, true);
		i2c_master_write_byte(cmd, 0xB0 | cur_page++, true); // increment page
		i2c_master_stop(cmd);
		i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS);
		i2c_cmd_link_delete(cmd);

		cmd = i2c_cmd_link_create();
		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
		i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);
		i2c_master_write(cmd, UIT_Logo_Page[i], 128, true);
		i2c_master_stop(cmd);
		i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS);
		i2c_cmd_link_delete(cmd);
	}
}