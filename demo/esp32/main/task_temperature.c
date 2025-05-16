//=============================================================================
/*-------------------------------- Includes ---------------------------------*/
//=============================================================================
#include "task_temperature.h"

/* Kernel */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

/* Device and drivers */
//#include "stdio.h"
//#include "pico/stdlib.h"

/* Temperature module */
//#include "tif/c/modules/temperature/temperature.h"
//#include "tif/c/hw/pico/temperatureHw.h"

// #include "tif/c/drivers/modbus/modbushandle.h"

#include "mqttmng.h"
#include "mqttmngConfig.h"

#include "task_svmqtt.h"
//=============================================================================

//=============================================================================
/*--------------------------------- Defines ---------------------------------*/
//=============================================================================

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
static void taskTemperatureUpdateMqtt(uint16_t temp);
//=============================================================================

//=============================================================================
/*---------------------------------- Task -----------------------------------*/
//=============================================================================
//-----------------------------------------------------------------------------
void taskTemperature(void *param){

    int32_t status = 0;
    int32_t temp;

    printf("\n\n ========xxxxxxxxxx Initializing tmep \n\n");
    vTaskDelay(300);

    taskTemperatureInitialize();

    while(1){

        //temperatureUpdate(0, 1000);
        vTaskDelay(3000 / portTICK_PERIOD_MS);
        temp = 23;
        //status = temperatureGet(0, &temp, 1000);
        if( status == 0 ) taskTemperatureUpdateMqtt( (uint16_t)temp );

        printf("Status: %d\tTemperature: %d\n\r", (int)status, (int)temp);
    }
}
//-----------------------------------------------------------------------------
//=============================================================================

//=============================================================================
/*---------------------------- Static functions -----------------------------*/
//=============================================================================
//-----------------------------------------------------------------------------
static void taskTemperatureInitialize(void){

    //taskTemperatureInitializeLock();

    mqttmngAddComponent(MQTT_MNG_COMP_1, (const char*)"temp1", (const char*)"temperature", (const char*)0);
    while( mqttmngInitDone() != 0 );

    //temperatureHwInitialize();

    //temperatureConfig_t config;
    //config.hwTempUpdate = temperatureHwUpdate;
    //config.hwTempGet = temperatureHwGet;
    //config.hwGetNumberSensors = temperatureHwGetNumberSensors;
    //config.lock = taskTemperatureLock;
    //config.unlock = taskTemperatureUnlock;
    //config.hwIf = 0;

    //temperatureInitialize(&config);
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

    if( taskSvmqttStatus() != 0 ) return;

    payload.data = (void *)&temp;
    payload.size = 2;
    payload.dup = 0;
    payload.retain = 0;

    mqttmngPublish(MQTT_MNG_COMP_1, "temperature", &payload);
}
//-----------------------------------------------------------------------------
//=============================================================================
