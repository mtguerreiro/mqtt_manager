//=============================================================================
/*-------------------------------- Includes ---------------------------------*/
//=============================================================================
#include "task_temperature.h"

/* Kernel */
#include "time.h"

/* Device and drivers */
#include "stdio.h"

#include "mqttmngConfig.h"
#include "mqttmng.h"

#include "task_svmqtt.h"
//=============================================================================

//=============================================================================
/*--------------------------------- Defines ---------------------------------*/
//=============================================================================
#define TASK_TEMPERATURE_CFG_PERIOD_MS      300
//=============================================================================

//=============================================================================
/*--------------------------------- Globals ---------------------------------*/
//=============================================================================
static struct timespec t;
// static SemaphoreHandle_t lock;
//=============================================================================

//=============================================================================
/*-------------------------------- Prototypes -------------------------------*/
//=============================================================================
static void taskTemperatureInitialize(void);
//static void taskTemperatureInitializeLock(void);
// static int32_t taskTemperatureLock(uint32_t to);
// static void taskTemperatureUnlock(void);
static void taskTemperatureUpdateMqtt(uint16_t temp);
//=============================================================================

//=============================================================================
/*---------------------------------- Task -----------------------------------*/
//=============================================================================
//-----------------------------------------------------------------------------
void* taskTemperature(void *param){

    int32_t status;
    int32_t temp;
    taskTemperatureInitialize();

    while(1){

        nanosleep(&t, NULL);
        taskTemperatureUpdateMqtt( 20 );

        //temperatureUpdate(0, 1000);
        // vTaskDelay(3000);
        // status = temperatureGet(0, &temp, 1000);
        // if( status == 0 ) taskTemperatureUpdateMqtt( (uint16_t)temp );

        // printf("Status: %d\tTemperature: %d\n\r", status, temp);
    }
}
//-----------------------------------------------------------------------------
//=============================================================================

//=============================================================================
/*---------------------------- Static functions -----------------------------*/
//=============================================================================
//-----------------------------------------------------------------------------
static void taskTemperatureInitialize(void){

    t.tv_sec = (TASK_TEMPERATURE_CFG_PERIOD_MS) / 1000;
    t.tv_nsec = (TASK_TEMPERATURE_CFG_PERIOD_MS) * 1000000U - t.tv_sec * 1000000000U;

    LogInfo( ("Running temperature task with. Timespec: %lu (s), %lu (ns).", t.tv_sec, t.tv_nsec) );

    // taskTemperatureInitializeLock();

    // temperatureHwInitialize();

    // temperatureConfig_t config;
    // config.hwTempUpdate = temperatureHwUpdate;
    // config.hwTempGet = temperatureHwGet;
    // config.hwGetNumberSensors = temperatureHwGetNumberSensors;
    // config.lock = taskTemperatureLock;
    // config.unlock = taskTemperatureUnlock;
    // config.hwIf = 0;

    // temperatureInitialize(&config);
}
//-----------------------------------------------------------------------------
// static void taskTemperatureInitializeLock(void){

//     lock = xSemaphoreCreateMutex();
// }
//-----------------------------------------------------------------------------
// static int32_t taskTemperatureLock(uint32_t to){

//     if( xSemaphoreTake(lock, to) != pdTRUE ) return -1;

//     return 0;
// }
//-----------------------------------------------------------------------------
// static void taskTemperatureUnlock(void){

//     xSemaphoreGive( lock );
// }
//-----------------------------------------------------------------------------
static void taskTemperatureUpdateMqtt(uint16_t temp){

    mqttmngPayload_t payload;

    if( taskSvmqttStatus() != 0 ) return;

    payload.data = (void *)&temp;
    payload.size = 2;
    payload.dup = 0;
    payload.retain = 0;

    mqttmngPublish(MQTT_MNG_COMP_1, "temperature", &payload);
}
//-----------------------------------------------------------------------------
//=============================================================================
