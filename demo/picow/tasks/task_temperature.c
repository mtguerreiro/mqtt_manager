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
#include "mdrivers/temperature.h"
#include "temperatureDs18b20.h"

#include "mqttmng.h"
#include "mqttConfig.h"
#include "loggingConfig.h"
//=============================================================================

//=============================================================================
/*--------------------------------- Defines ---------------------------------*/
//=============================================================================
#define TEMP_CFG_MQTT_COMP_NAME     "temp1"
#define TEMP_CFG_MQTT_COMP_TYPE     "temperature"
#define TEMP_CFG_MQTT_COMP_FLAGS    NULL

#define TEMP_CFG_MQTT_COMP_ID    MQTT_CONFIG_DEV_ID "/" TEMP_CFG_MQTT_COMP_NAME

#define TASK_TEMPERATURE_CFG_PERIOD_MS      3000
//=============================================================================

//=============================================================================
/*--------------------------------- Globals ---------------------------------*/
//=============================================================================
static SemaphoreHandle_t lock;
static int32_t tempidx = -1;
//=============================================================================

//=============================================================================
/*-------------------------------- Prototypes -------------------------------*/
//=============================================================================
static void taskTemperatureInitialize(void);
static void taskTemperatureInitializeLock(void);
static int32_t taskTemperatureLock(uint32_t to);
static int32_t taskTemperatureUnlock(void);
static void taskTemperatureMqttUpdate(uint16_t temp);
//=============================================================================

//=============================================================================
/*---------------------------------- Task -----------------------------------*/
//=============================================================================
//-----------------------------------------------------------------------------
void taskTemperature(void *param){

    (void) param;

    int32_t status;
    int32_t temp;
    taskTemperatureInitialize();

    temperatureUpdate(tempidx, 0, 1000);

    while(1){
        vTaskDelay(3000 / portTICK_PERIOD_MS);

        status = temperatureRead(tempidx, 0, &temp, 1000);
        LogInfo( ("Temperature status %d, reading %d.", (int)status, (int)temp) );

        if( status == 0 ) taskTemperatureMqttUpdate( (uint16_t)temp );

        temperatureUpdate(tempidx, 0, 1000);
    }
}
//-----------------------------------------------------------------------------
//=============================================================================

//=============================================================================
/*---------------------------- Static functions -----------------------------*/
//=============================================================================
//-----------------------------------------------------------------------------
static void taskTemperatureInitialize(void){

    temperatureConfig_t config = {0};
    temperatureDriver_t driver = {0};

    temperatureDs18b20Initialize();
    taskTemperatureInitializeLock();

    config.lock = taskTemperatureLock;
    config.unlock = taskTemperatureUnlock;
    temperatureInitialize(&config);

    driver.read = temperatureDs18b20Read;
    driver.update = temperatureDs18b20Update;
    tempidx = temperatureRegister(&driver, 1000);

    while( mqttmngInitDone() != 0 ) vTaskDelay(1000 / portTICK_PERIOD_MS);

    mqttmngAddComponent(
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
static int32_t taskTemperatureUnlock(void){

    xSemaphoreGive( lock );

    return 0;
}
//-----------------------------------------------------------------------------
static void taskTemperatureMqttUpdate(uint16_t temp){

    mqttPayload_t payload;

    payload.data = (void *)&temp;
    payload.size = 2;
    payload.dup = 0;
    payload.retain = 0;

    mqttmngPublish(TEMP_CFG_MQTT_COMP_ID "/temperature", &payload);
}
//-----------------------------------------------------------------------------
//=============================================================================
