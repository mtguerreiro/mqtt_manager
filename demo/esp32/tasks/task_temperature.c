//=============================================================================
/*-------------------------------- Includes ---------------------------------*/
//=============================================================================
#include "task_temperature.h"

/* Kernel */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

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

    int32_t status = 0;
    int32_t temp;

    taskTemperatureInitialize();

    while(1){

        vTaskDelay(3000 / portTICK_PERIOD_MS);

        temp = 23;
        if( status == 0 ) taskTemperatureUpdateMqtt( (uint16_t)temp );

        LogInfo( ("Temperature %d.", (int)temp) );
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
