
#include "freertos/idf_additions.h"
#include "lcd.h"
#include "network.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "definitions.h"
#include "esp_heap_caps.h"
#include "esp_system.h"
#include "lwip/sockets.h"
#include "portmacro.h"
#include <stdio.h>
#include <stdlib.h>

void wait_for_transfer_finish()
{
	while(transfer_done_semaphore)
	{
    	vTaskDelay(100 / portTICK_PERIOD_MS);
	}
}

void convertRecvBuffToFrameBuff(uint8_t* recv_buff, uint16_t* frame_buff, int recv_buff_size);

void app_main(void)
{
	prepare_wifi_modules();
	setup_wifi();

   	esp_lcd_panel_io_handle_t io_handle;
	setup_lcd(&io_handle);
	printf("Finished lcd setup\n");
    //lights up
	int listen_sock_fd = setup_socket();
	printf("bound socket\n");

    uint16_t* frame_buffer;
	int frame_buffer_size = SIZEV * SIZEH * sizeof(uint16_t);
    frame_buffer = heap_caps_malloc(frame_buffer_size, MALLOC_CAP_DMA | MALLOC_CAP_8BIT);

	uint8_t* recv_buffer;
	int recv_buffer_size = frame_buffer_size/16;
	recv_buffer = malloc(recv_buffer_size);

	if(frame_buffer == NULL)
	{
		printf("malloc failed!!!!!!!!!!!!!!\n");
		esp_restart();
	}

	printf("allocated buffer size: %d\n", frame_buffer_size);

	draw_frame_buffer(io_handle, frame_buffer);

	printf("Listening for tcp traffic\n");



	while (1) {
		int sock = accept(listen_sock_fd, NULL, NULL);
		bool closed = false;

		while(!closed)
		{
			int i = 0;
			while (i < recv_buffer_size)
			{
				int read = recv(sock, recv_buffer + i, recv_buffer_size - i , 0);

				if(read <= 0) {
					//printf("Read: %d\n",read);
					closed = true;
					break;
				}

				i += read;
				//printf("read: %d\n",i);
			}

			if(closed) {break;}
		
			//printf("drawing frame buffer?\n");
			
			convertRecvBuffToFrameBuff(recv_buffer, frame_buffer, recv_buffer_size);
			draw_frame_buffer(io_handle, frame_buffer);

			write(sock, "hi", 2); // send something to the client so it doesnt spam

			//wait_for_transfer_finish();

		}

		shutdown(sock, 0);
		close(sock);
	}

    esp_restart();
}

uint16_t convertToColor(uint8_t byte)
{
	if(byte>0)
	{
		return 65535;
	}
	else
	{
		return 0;
	}
}

void convertRecvBuffToFrameBuff(uint8_t* recv_buff, uint16_t* frame_buff, int recv_buff_size)
{
	for(int i = 0; i < recv_buff_size; i++)
	{
		uint8_t currentByte = recv_buff[i];
		for(int bit = 0; bit < 8; bit++)
		{
			frame_buff[i*8+bit] = convertToColor((currentByte >> (7-bit)) & 0x01);
		}
	}
}
