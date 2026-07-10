idf_component_register(SRCS "server.c" "control.c" PRIV_REQUIRES esp_driver_gpio esp_wifi esp_http_server nvs_flash INCLUDE_DIRS ".")
