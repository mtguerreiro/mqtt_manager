idf_component_register(SRCS "task_temperature.c" "station_example_main.c" "task_svmqtt.c"
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES esp_wifi nvs_flash
                    REQUIRES mqtt_manager
                    )
