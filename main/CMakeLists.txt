idf_component_register(SRCS 
    mqtt/MqttClient.cxx
    uart/UartEvent.cxx
    wifi/Wireless.cxx

    Application.cxx

    REQUIRES
    driver
    esp_app_format
    esp_wifi
    mqtt
    nvs_flash

    INCLUDE_DIRS ".")

add_subdirectory(components/util)

target_link_libraries(
    ${COMPONENT_LIB} PUBLIC 
    util
)
