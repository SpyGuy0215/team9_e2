zephyr_include_directories(../include)

if(CONFIG_CUSTOM_RGB_LED)
    add_subdirectory(../drivers/rgb_led drivers_rgb_led)
endif()