set(SOURCES main.c)
idf_component_register(SRC_DIRS "." "images" "sounds"
                    INCLUDE_DIRS "includes"
                    REQUIRES "core2forAWS" "esp-cryptoauthlib" "fft" "nvs_flash")
