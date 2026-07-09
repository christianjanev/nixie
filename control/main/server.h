#include <esp_http_server.h>
#include <esp_err.h>
#include <esp_wifi.h>

esp_err_t initialize_wifi();

httpd_handle_t start_server();