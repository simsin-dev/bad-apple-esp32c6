
#ifndef __NETWORK_H__
#define __NETWORK_H__
#include <sys/types.h>
#include "esp_netif_types.h"

void print_ip(char* custom_msg, uint32_t addr);

void get_addr_info(esp_netif_t* netif);

void prepare_wifi_modules();
void setup_wifi();

int setup_socket();
#endif

