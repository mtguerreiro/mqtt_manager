idf_component_register(SRCS "station_example_main.c" "task_svmqtt.c" "task_temperature.c" "task_blink.c" 
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES esp_wifi nvs_flash driver
                    REQUIRES mqtt_manager
                    )
