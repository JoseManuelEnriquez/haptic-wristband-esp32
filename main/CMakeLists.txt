idf_component_register(SRCS "main.cpp"
                    INCLUDE_DIRS "."
                    REQUIRES HapticController esp_driver_gpio GPtimer)
