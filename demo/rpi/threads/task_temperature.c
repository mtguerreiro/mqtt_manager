//=============================================================================
/*-------------------------------- Includes ---------------------------------*/
//=============================================================================
#include "task_temperature.h"

#include "time.h"
#include "stdio.h"

#include "mqttmng.h"
#include "mqttmngConfig.h"
#include "loggingConfig.h"

#include "temperatureDs18b20.h"
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
static struct timespec t;
//=============================================================================

//=============================================================================
/*-------------------------------- Prototypes -------------------------------*/
//=============================================================================
static void taskTemperatureInitialize(void);
static void taskTemperatureMqttUpdate(uint16_t temp);
//=============================================================================

//=============================================================================
/*---------------------------------- Task -----------------------------------*/
//=============================================================================
//-----------------------------------------------------------------------------
void* taskTemperature(void *param){

    (void)param;
    taskTemperatureInitialize();
    int32_t temp = 0;
    int32_t status = 0;

    while(1){

        nanosleep(&t, NULL);

        status = temperatureDs18b20Read(0, &temp);
        LogInfo( ("Temperature status %d, reading %d.", (int)status, (int)temp) );
        if( status != 0 ) continue;

        taskTemperatureMqttUpdate( temp );
    }
}
//-----------------------------------------------------------------------------
//=============================================================================

//=============================================================================
/*---------------------------- Static functions -----------------------------*/
//=============================================================================
//-----------------------------------------------------------------------------
static void taskTemperatureInitialize(void){

    while( mqttmngInitDone() != 0 );

    mqttmngPublishComponent(
        TEMP_CFG_MQTT_COMP_NAME,
        TEMP_CFG_MQTT_COMP_TYPE,
        TEMP_CFG_MQTT_COMP_FLAGS
    );

    t.tv_sec = (TASK_TEMPERATURE_CFG_PERIOD_MS) / 1000;
    t.tv_nsec = (TASK_TEMPERATURE_CFG_PERIOD_MS) * 1000000U - t.tv_sec * 1000000000U;

    LogInfo( ("Running temperature task with. Timespec: %lu (s), %lu (ns).", t.tv_sec, t.tv_nsec) );
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
