
//=============================================================================
/*-------------------------------- Includes ---------------------------------*/
//=============================================================================
#include "task_wifi_init.h"

/* Kernel */
#include "FreeRTOS.h"
#include "task.h"

/* Device and drivers */
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

/* Tasks */
#include "task_svmqtt.h"
#include "task_blink.h"
#include "task_temperature.h"
#include "task_led.h"

/* SVMQTT */
#include "mqttmng.h"
#include "mqttmngConfig.h"
//=============================================================================

//=============================================================================
/*--------------------------------- Defines ---------------------------------*/
//=============================================================================

//=============================================================================

//=============================================================================
/*--------------------------------- Globals ---------------------------------*/
//=============================================================================

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

    printf("Initializing WiFi...\n\r");

    if( taskWifiInitHwInit() != 0 ) exit (-1);

    printf("Wifi initialized.\n\r");

    xTaskCreate(
        taskSvmqtt,
        "svmqtt",
        TASKS_SVMQTT_CONFIG_TASK_STACK_SIZE,
        NULL,
        TASKS_SVMQTT_CONFIG_TASK_PRIO,
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

    vTaskDelete(NULL);
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
        printf("Failed to initialize cyw43 arch \n");
        return -1;
    }
 
    cyw43_arch_enable_sta_mode();
 
    printf("Connecting to WiFi...\n\r");
    
    status = cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000);
    if ( status != 0 )
    {   
        printf("Failed to connect. Status %d\n\r", status);
        while(1);
        return -1;
    }

    printf("Connected.\n\r");

    return 0;
}
//-----------------------------------------------------------------------------
//=============================================================================
