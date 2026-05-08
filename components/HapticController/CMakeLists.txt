idf_component_register(
    SRCS "HapticController.cpp"
    INCLUDE_DIRS "include"
    REQUIRES nvs_flash bt
)
target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-switch)