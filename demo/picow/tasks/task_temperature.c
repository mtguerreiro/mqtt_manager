//=============================================================================
/*-------------------------------- Includes ---------------------------------*/
//=============================================================================
#include "task_temperature.h"

/* Kernel */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* Device and drivers */
#include "stdio.h"
#include "pico/stdlib.h"

/* Temperature module */
#include "tif/c/modules/temperature/temperature.h"
#include "tif/c/hw/pico/temperatureHw.h"

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
static SemaphoreHandle_t lock;

static mqttmngConfig_t mqttconfig;
//=============================================================================

//=============================================================================
/*-------------------------------- Prototypes -------------------------------*/
//=============================================================================
static void taskTemperatureInitialize(void);
static void taskTemperatureInitializeLock(void);
static int32_t taskTemperatureLock(uint32_t to);
static void taskTemperatureUnlock(void);
static void taskTemperatureUpdateMqtt(uint16_t temp);
//=============================================================================

//=============================================================================
/*---------------------------------- Task -----------------------------------*/
//=============================================================================
//-----------------------------------------------------------------------------
void taskTemperature(void *param){

    int32_t status;
    int32_t temp;
    taskTemperatureInitialize();

    temperatureUpdate(0, 1000);

    while(1){
        vTaskDelay(3000);

        status = temperatureGet(0, &temp, 1000);
        LogInfo( ("Temperature %d.", temp) );

        if( status == 0 ) taskTemperatureUpdateMqtt( (uint16_t)temp );

        temperatureUpdate(0, 1000);
    }
}
//-----------------------------------------------------------------------------
//=============================================================================

//=============================================================================
/*---------------------------- Static functions -----------------------------*/
//=============================================================================
//-----------------------------------------------------------------------------
static void taskTemperatureInitialize(void){

    taskTemperatureInitializeLock();

    temperatureHwInitialize();

    temperatureConfig_t config;
    config.hwTempUpdate = temperatureHwUpdate;
    config.hwTempGet = temperatureHwGet;
    config.hwGetNumberSensors = temperatureHwGetNumberSensors;
    config.lock = taskTemperatureLock;
    config.unlock = taskTemperatureUnlock;
    config.hwIf = 0;

    temperatureInitialize(&config);

    mqttconfig.name = (const char*)"temp1";
    mqttconfig.type = (const char*)"temperature";
    mqttconfig.flags = NULL;
    mqttconfig.subscriptions = NULL;
    mqttconfig.nSubscriptions = 0;

    mqttmngAddComponent(MQTT_MNG_COMP_1, &mqttconfig);
    while( mqttmngInitDone() != 0 );
}
//-----------------------------------------------------------------------------
static void taskTemperatureInitializeLock(void){

    lock = xSemaphoreCreateMutex();
}
//-----------------------------------------------------------------------------
static int32_t taskTemperatureLock(uint32_t to){

    if( xSemaphoreTake(lock, to) != pdTRUE ) return -1;

    return 0;
}
//-----------------------------------------------------------------------------
static void taskTemperatureUnlock(void){

    xSemaphoreGive( lock );
}
//-----------------------------------------------------------------------------
static void taskTemperatureUpdateMqtt(uint16_t temp){

    mqttmngPayload_t payload;

    payload.data = (void *)&temp;
    payload.size = 2;
    payload.dup = 0;
    payload.retain = 0;

    mqttmngPublish(MQTT_MNG_COMP_1, "temperature", &payload);
}
//-----------------------------------------------------------------------------
//=============================================================================
