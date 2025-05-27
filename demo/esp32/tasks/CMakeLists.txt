idf_component_register(SRCS "task_mqtt_mng.c" "task_temperature.c" "task_blink.c" "task_led.c" 
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES driver led_strip
                    REQUIRES mqtt_manager
                    )
