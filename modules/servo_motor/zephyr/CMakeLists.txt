zephyr_include_directories(../include)

if(CONFIG_SERVO_MOTOR)
    add_subdirectory(../drivers/servo_motor drivers_servo_motor)
endif()