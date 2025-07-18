
#include "lcd.h"
#include "definitions.h"
#include "driver/spi_master.h"
#include "esp_lcd_types.h"
#include "freertos/idf_additions.h"
#include "hal/spi_types.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_io.h"
#include "driver/spi_common.h"
#include "esp_lcd_io_spi.h"
#include "esp_lcd_panel_st7789.h"
#include "portmacro.h"

bool transfer_done_semaphore = false;

bool transfer_done_callback(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
	transfer_done_semaphore = false;
	return false;
}

void setup_lcd(esp_lcd_panel_io_handle_t* io_handle)
{
	spi_bus_config_t spi_bus_conf = {
        .sclk_io_num = SCLK_GPIO,
        .mosi_io_num = MOSI_GPIO,
        .miso_io_num = MISO_GPIO,
        .quadhd_io_num = -1,
        .quadwp_io_num = -1,
        .max_transfer_sz = SIZEH * SIZEV * 2 +8
    };

    spi_bus_initialize(SPI2_HOST,&spi_bus_conf, SPI_DMA_CH_AUTO);

    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = DC_GPIO,
        .cs_gpio_num = -1,
        .pclk_hz = CLK_SPEED,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .spi_mode = 3,
        .trans_queue_depth = 10,
    };

    esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SPI2_HOST, &io_config, io_handle);

    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = RST_GPIO,
        .bits_per_pixel = 16,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
    };

    esp_lcd_panel_handle_t panel_handle;
    esp_lcd_new_panel_st7789(*io_handle, &panel_config, &panel_handle);

    esp_lcd_panel_reset(panel_handle);
    esp_lcd_panel_init(panel_handle);
    esp_lcd_panel_set_gap(panel_handle, 0, 0);
    esp_lcd_panel_invert_color(panel_handle, true);
    esp_lcd_panel_disp_on_off(panel_handle, true);
    esp_lcd_panel_mirror(panel_handle, false, false);

    esp_lcd_panel_io_tx_param(*io_handle, 0x2A, (uint8_t[]){0x00,0x00,0x00,0xEF}, 4);
    esp_lcd_panel_io_tx_param(*io_handle, 0x2B, (uint8_t[]){0x00,0x00,0x01,0x3F}, 4);
    vTaskDelay(200 / portTICK_PERIOD_MS);

	esp_lcd_panel_io_callbacks_t cbs = {
		.on_color_trans_done = transfer_done_callback
	};
	esp_lcd_panel_io_register_event_callbacks(*io_handle, &cbs, NULL);

    vTaskDelay(200 / portTICK_PERIOD_MS);
}

void draw_frame_buffer(esp_lcd_panel_io_handle_t io_handle, void* frame_buffer)
{
    esp_lcd_panel_io_tx_param(io_handle, 0x2A, (uint8_t[]){0x00,0x00,0x00,0xEF}, 4);
    esp_lcd_panel_io_tx_param(io_handle, 0x2B, (uint8_t[]){0x00,0x00,0x01,0x3F}, 4);
    esp_lcd_panel_io_tx_color(io_handle, 0x2C, frame_buffer,SIZEV * SIZEH * sizeof(uint16_t));

	transfer_done_semaphore = true;
}
