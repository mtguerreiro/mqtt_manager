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

/* Led module */
#include "tif/c/modules/led/led.h"
#include "tif/c/hw/pico/ledHw.h"

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
mqttmngSubscrConfig_t mqttsubscr[2];
mqttmngSubscrConfig_t *mqttsubscrptr[2];
mqttmngConfig_t mqttconfig;
//=============================================================================

//=============================================================================
/*-------------------------------- Prototypes -------------------------------*/
//=============================================================================
static void taskLedInitialize(void);
static void taskLedUpdateStateMqtt(MQTTContext_t *pContext, MQTTPublishInfo_t *pPublishInfo);
static void taskLedUpdateState(uint8_t state);
static void taskLedUpdateRgbMqtt(MQTTContext_t *pContext, MQTTPublishInfo_t *pPublishInfo);
static void taskLedUpdateRgb(uint8_t *data);
static void taskLedUpdateIntensityMqtt(MQTTContext_t *pContext, MQTTPublishInfo_t *pPublishInfo);
// static void taskLedUpdateRgb(uint8_t *data);
//=============================================================================

//=============================================================================
/*---------------------------------- Task -----------------------------------*/
//=============================================================================
//-----------------------------------------------------------------------------
void taskLed(void *param){

    taskLedInitialize();

    vTaskDelete(NULL);
}
//-----------------------------------------------------------------------------
//=============================================================================

//=============================================================================
/*---------------------------- Static functions -----------------------------*/
//=============================================================================
//-----------------------------------------------------------------------------
static void taskLedInitialize(void){

    ledHwInitialize();

    ledConfig_t ledConfig;

    ledConfig.lock = 0;
    ledConfig.unlock = 0;
    ledConfig.hwSetColor = ledHwSetColor;
    ledConfig.hwSetIntensity = ledHwSetIntensity;
    ledConfig.hwGetNumberLeds = ledHwGetNumberLeds;
    ledConfig.hwIf = ledHwInterface;

    ledInitialize(&ledConfig);

    mqttsubscr[0].topic = "state";
    mqttsubscr[0].callback = taskLedUpdateStateMqtt;

    mqttsubscr[1].topic = "rgb";
    mqttsubscr[1].callback = taskLedUpdateRgbMqtt;

    mqttsubscrptr[0] = &mqttsubscr[0];
    mqttsubscrptr[1] = &mqttsubscr[1];

    mqttconfig.subscriptions = mqttsubscrptr;
    mqttconfig.nSubscriptions = 2;

    mqttconfig.name = "led233";
    mqttconfig.type = "led";
    mqttconfig.flags = "ri";

    mqttmngAddComponent(MQTT_MNG_COMP_2, &mqttconfig);
    while( mqttmngInitDone() != 0 );
}
//-----------------------------------------------------------------------------
static void taskLedUpdateStateMqtt(MQTTContext_t *pContext, MQTTPublishInfo_t *pPublishInfo){

    uint8_t state;

    assert( pPublishInfo != NULL );
    assert( pContext != NULL );

    /* Suppress unused parameter warning when asserts are disabled in build. */
    ( void ) pContext;

    LogInfo( ("Invoked led state callback.") );
    taskLedUpdateState( *( (uint8_t *) pPublishInfo->pPayload ) );
}
//-----------------------------------------------------------------------------
static void taskLedUpdateState(uint8_t state){

    LogInfo( ("Setting LED state to %d", state) );

    if( state )
        ledSetIntensity(0, 4, 1000);
    else
        ledSetIntensity(0, 0, 1000);
    
}
//-----------------------------------------------------------------------------
static void taskLedUpdateRgbMqtt(MQTTContext_t *pContext, MQTTPublishInfo_t *pPublishInfo){

    assert( pPublishInfo != NULL );
    assert( pContext != NULL );

    /* Suppress unused parameter warning when asserts are disabled in build. */
    ( void ) pContext;

    LogInfo( ("Invoked led rgb callback.") );
    
    taskLedUpdateRgb( (uint8_t *) pPublishInfo->pPayload );

}
//-----------------------------------------------------------------------------
static void taskLedUpdateRgb(uint8_t *data){

    mqttmngPayload_t payload;

    LogInfo( ("Setting LED color to %d %d %d", data[0], data[1], data[2]) );

    ledSetColor(0, data[0], data[1], data[2], 1000);
}
//-----------------------------------------------------------------------------
static void taskLedUpdateIntensityMqtt(MQTTContext_t *pContext, MQTTPublishInfo_t *pPublishInfo){

    assert( pPublishInfo != NULL );
    assert( pContext != NULL );

    /* Suppress unused parameter warning when asserts are disabled in build. */
    ( void ) pContext;

    LogInfo( ("Invoked led intensity callback.") );

    float duty = *((float *) pPublishInfo->pPayload);
    
    LogInfo( ("Duty: %.4f", duty) );
}
//-----------------------------------------------------------------------------
//=============================================================================
