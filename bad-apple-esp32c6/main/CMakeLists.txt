idf_component_register(SRCS "program.c" "network.c" "network.h" "lcd.h" "lcd.c" "definitions.h"
                    PRIV_REQUIRES spi_flash
                    REQUIRES esp_lcd
                    REQUIRES driver
                    PRIV_REQUIRES esp_wifi nvs_flash
                    INCLUDE_DIRS "")
