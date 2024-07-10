idf_component_register(SRCS 
    Application.cxx
    sync.cxx

    REQUIRES
    driver
    esp_app_format
    esp_wifi

    INCLUDE_DIRS ".")

add_subdirectory(components/util)

target_link_libraries(
    ${COMPONENT_LIB} PUBLIC 
    util
)
