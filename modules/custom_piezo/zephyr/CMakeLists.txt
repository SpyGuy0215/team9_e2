zephyr_include_directories(../include)

if(CONFIG_CUSTOM_PIEZO)
  add_subdirectory(../drivers/piezo custom_piezo_driver)
endif()