
#include "definitions.h"
#include "network.h"

#include "esp_netif.h"
#include "esp_netif_types.h"
#include <stdio.h>
#include "esp_netif_ip_addr.h"
#include "esp_wifi_types_generic.h"
#include "esp_wifi.h"
#include <netdb.h>
#include "lwip/sockets.h"
#include "nvs_flash.h"

void print_ip(char* custom_msg, uint32_t addr)
{
	uint8_t octets[4];
	octets[0] = (addr << 24) >> 24;
	octets[1] = (addr << 16) >> 24;
	octets[2] = (addr << 8) >> 24;
	octets[3] = addr >> 24;

	char* msg = custom_msg != NULL ? custom_msg : "IP";
	printf("%s: %u.%u.%u.%u\n", msg, octets[0],octets[1],octets[2],octets[3]);
}

void get_addr_info(esp_netif_t* netif)
{
	esp_netif_ip_info_t ip_info;

	esp_netif_get_ip_info(netif, &ip_info);
	print_ip(NULL, ip_info.ip.addr);
	print_ip("Gateway", ip_info.gw.addr);
	print_ip("Netmask", ip_info.netmask.addr);
}

void setup_wifi()
{
	esp_netif_t* netif = esp_netif_create_default_wifi_sta();
    
    wifi_init_config_t wifi_conf_init = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_conf_init);

    esp_wifi_set_mode(WIFI_MODE_STA);
	wifi_config_t wifi_conf = {
		.sta = {
			.ssid = AP_SSID,
			.password = AP_PASSWD,
			.threshold.authmode = WIFI_AUTH_WPA2_PSK,
		}
	};

	esp_wifi_set_config(WIFI_IF_STA, &wifi_conf);

    esp_wifi_start();
	esp_wifi_connect();

	printf("We should be connected?...\n");

	esp_netif_dhcpc_stop(netif);

	esp_netif_ip_info_t ip_info;
	memset(&ip_info, 0, sizeof(esp_netif_ip_info_t));
	ip_info.ip.addr = ipaddr_addr(IP); 
	ip_info.gw.addr = ipaddr_addr(GW); 
	ip_info.netmask.addr = ipaddr_addr(NM);

	esp_netif_set_ip_info(netif, &ip_info);

    vTaskDelay(1000 / portTICK_PERIOD_MS);

	get_addr_info(netif);
}

void prepare_wifi_modules()
{
    nvs_flash_init();
    esp_netif_init(); 
    esp_event_loop_create_default();
}

int setup_socket()
{
	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

	struct timeval timeout;
	timeout.tv_sec = 100;
	timeout.tv_usec = 0;
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

	struct sockaddr_in sock_addr;
	sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_port = htons(PORT);
	sock_addr.sin_len = 16;

	int res = bind(sock, (struct sockaddr *)&sock_addr, sizeof(sock_addr));
	if(res < 0)
	{
		printf("Couldn't bind socket err: %d\n",res);
		esp_restart();
	}

	listen(sock, 0);

	return sock;
}
