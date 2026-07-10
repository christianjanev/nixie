#ifndef SERVER_H
#define SERVER_H

// Enter AP credentials
#define SSID ""
#define PSK ""

#include <esp_http_server.h>
#include <esp_err.h>
#include <esp_wifi.h>
#include <nvs_flash.h>

#define WIFI_CONNECTED_BIT BIT0

esp_err_t initialize_wifi(esp_netif_t** netif);

httpd_handle_t start_server();

#endif // SERVER_H