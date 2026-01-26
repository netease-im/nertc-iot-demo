idf_component_register(
    INCLUDE_DIRS "include"
)

add_prebuilt_library(nertc_wake_up "${CMAKE_CURRENT_SOURCE_DIR}/libs/esp32s3/libnertc_wake_up.a"
                    REQUIRES espressif__esp-sr)
target_link_libraries(${COMPONENT_LIB} INTERFACE nertc_wake_up)
