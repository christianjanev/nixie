idf_component_register(SRCS "server.c" "control.c" PRIV_REQUIRES esp_driver_gpio esp_wifi esp_http_server nvs_flash littlefs INCLUDE_DIRS ".")

littlefs_create_partition_image("webserver" "server" FLASH_IN_PROJECT)
