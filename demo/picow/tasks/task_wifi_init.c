
//=============================================================================
/*-------------------------------- Includes ---------------------------------*/
//=============================================================================
#include "task_wifi_init.h"

/* Kernel */
#include "FreeRTOS.h"
#include "task.h"

/* Device and drivers */
#include "cyw43.h"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

/* Tasks */
#include "task_mqtt_mng.h"
#include "task_blink.h"
#include "task_temperature.h"
#include "task_led.h"

/* SVMQTT */
#include "mqttmng.h"
#include "mqttmngConfig.h"
#include "loggingConfig.h"
//=============================================================================

//=============================================================================
/*--------------------------------- Defines ---------------------------------*/
//=============================================================================

//=============================================================================

//=============================================================================
/*--------------------------------- Globals ---------------------------------*/
//=============================================================================
static volatile int wifistatus = 1;
//=============================================================================

//=============================================================================
/*-------------------------------- Prototypes -------------------------------*/
//=============================================================================
static int taskWifiInitHwInit(void);
//=============================================================================

//=============================================================================
/*---------------------------------- Task -----------------------------------*/
//=============================================================================
//-----------------------------------------------------------------------------
void taskWifiInit(void *param){

    (void) param;

    LogInfo(( "Initializing WiFi..." ));

    if( taskWifiInitHwInit() != 0 ) exit (-1);

    LogInfo(( "Wifi initialized." ));

    xTaskCreate(
        taskMqttmng,
        "mqttmng",
        TASKS_MQTT_MNG_CONFIG_TASK_STACK_SIZE,
        NULL,
        TASKS_MQTT_MNG_CONFIG_TASK_PRIO,
        NULL );

    xTaskCreate(
        taskBlink,
        "blink",
        TASK_BLINK_CONFIG_TASK_STACK_SIZE,
        NULL,
        TASK_BLINK_CONFIG_TASK_PRIO,
        NULL );

    xTaskCreate(
        taskTemperature,
        "temperature",
        TASK_TEMPERATURE_CONFIG_TASK_STACK_SIZE,
        NULL,
        TASK_TEMPERATURE_CONFIG_TASK_PRIO,
        NULL );

    xTaskCreate(
        taskLed,
        "led",
        TASK_LED_CONFIG_TASK_STACK_SIZE,
        NULL,
        TASK_LED_CONFIG_TASK_PRIO,
        NULL );

    vTaskDelete( NULL );

}
//-----------------------------------------------------------------------------
//=============================================================================

//=============================================================================
/*---------------------------- Static functions -----------------------------*/
//=============================================================================
//-----------------------------------------------------------------------------
static int taskWifiInitHwInit(void){
    
    int status;

    if (cyw43_arch_init())
    {
        LogError(( "Failed to initialize cyw43 arch" ));
        return -1;
    }
 
    cyw43_arch_enable_sta_mode();
 
    LogInfo(( "Connecting to WiFi..." ));
    
    status = cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000);
    if ( status != 0 )
    {   
        LogError(( "Failed to connect. Status %d", status ));
        while(1);
        return -1;
    }

    LogInfo(( "Connected. "));

    return 0;
}
//-----------------------------------------------------------------------------
//=============================================================================
