zephyr_include_directories(../include)

if(CONFIG_CUSTOM_USART)
  add_subdirectory(../drivers/usart custom_usart_driver)
endif()