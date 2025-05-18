idf_component_register(SRCS "task_svmqtt.c" "task_temperature.c" "task_blink.c" 
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES driver
                    REQUIRES mqtt_manager
                    )
