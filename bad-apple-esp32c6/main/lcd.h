
#ifndef __LCD_H__
#define __LCD_H__
#include "esp_lcd_types.h"
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

void setup_lcd(esp_lcd_panel_io_handle_t* io_handle);

void draw_frame_buffer(esp_lcd_panel_io_handle_t io_handle, void* frame_buffer);

extern bool transfer_done_semaphore;
#endif
