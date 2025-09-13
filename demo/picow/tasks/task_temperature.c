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
#include "mdrivers/temperature/temperature.h"
#include "mhw/pico/temperatureHw.h"

#include "mqttmng.h"
#include "mqttmngConfig.h"
#include "loggingConfig.h"
//=============================================================================

//=============================================================================
/*--------------------------------- Defines ---------------------------------*/
//=============================================================================
#define TEMP_CFG_MQTT_COMP_NAME     "temp1"
#define TEMP_CFG_MQTT_COMP_TYPE     "temperature"
#define TEMP_CFG_MQTT_COMP_FLAGS    NULL

#define TEMP_CFG_MQTT_COMP_ID    MQTT_MNG_CONFIG_DEV_ID "/" TEMP_CFG_MQTT_COMP_NAME

#define TASK_TEMPERATURE_CFG_PERIOD_MS      3000
//=============================================================================

//=============================================================================
/*--------------------------------- Globals ---------------------------------*/
//=============================================================================
static SemaphoreHandle_t lock;
//=============================================================================

//=============================================================================
/*-------------------------------- Prototypes -------------------------------*/
//=============================================================================
static void taskTemperatureInitialize(void);
static void taskTemperatureInitializeLock(void);
static int32_t taskTemperatureLock(uint32_t to);
static void taskTemperatureUnlock(void);
static void taskTemperatureMqttUpdate(uint16_t temp);
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
        LogInfo( ("Temperature %d.", (int)temp) );

        if( status == 0 ) taskTemperatureMqttUpdate( (uint16_t)temp );

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

    temperatureInitialize(&config);

    while( mqttmngInitDone() != 0 );

    mqttmngPublishComponent(
        TEMP_CFG_MQTT_COMP_NAME,
        TEMP_CFG_MQTT_COMP_TYPE,
        TEMP_CFG_MQTT_COMP_FLAGS
    );
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
static void taskTemperatureMqttUpdate(uint16_t temp){

    mqttmngPayload_t payload;

    payload.data = (void *)&temp;
    payload.size = 2;
    payload.dup = 0;
    payload.retain = 0;

    mqttmngPublish(TEMP_CFG_MQTT_COMP_ID "/temperature", &payload);
}
//-----------------------------------------------------------------------------
//=============================================================================
