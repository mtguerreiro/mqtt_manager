//=============================================================================
/*-------------------------------- Includes ---------------------------------*/
//=============================================================================
#include "task_temperature.h"

/* Kernel */

/* Device and drivers */
#include "stdio.h"


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
//static SemaphoreHandle_t lock;
//=============================================================================

//=============================================================================
/*-------------------------------- Prototypes -------------------------------*/
//=============================================================================
static void taskLedInitialize(void);
static void taskLedUpdateStateMqtt(MQTTContext_t *pContext, MQTTPublishInfo_t *pPublishInfo);
static void taskLedUpdateState(uint8_t state);
static void taskLedUpdateRgbMqtt(MQTTContext_t *pContext, MQTTPublishInfo_t *pPublishInfo);
static void taskLedUpdateRgb(uint8_t *data);
//=============================================================================

//=============================================================================
/*---------------------------------- Task -----------------------------------*/
//=============================================================================
//-----------------------------------------------------------------------------
void* taskLed(void *param){

    taskLedInitialize();

    return NULL;

    //vTaskDelete(NULL);
}
//-----------------------------------------------------------------------------
//=============================================================================

//=============================================================================
/*---------------------------- Static functions -----------------------------*/
//=============================================================================
//-----------------------------------------------------------------------------
static void taskLedInitialize(void){

    // ledHwInitialize();

    // ledConfig_t ledConfig;

    // ledConfig.lock = 0;
    // ledConfig.unlock = 0;
    // ledConfig.hwSetColor = ledHwSetColor;
    // ledConfig.hwSetIntensity = ledHwSetIntensity;
    // ledConfig.hwGetNumberLeds = ledHwGetNumberLeds;
    // ledConfig.hwIf = ledHwInterface;

    // ledInitialize(&ledConfig);

    while( taskSvmqttStatus() != 0 );

    mqttmngSubscribe(MQTT_MNG_COMP_2, "state", taskLedUpdateStateMqtt);
    mqttmngSubscribe(MQTT_MNG_COMP_2, "rgb", taskLedUpdateRgbMqtt);
}
//-----------------------------------------------------------------------------
static void taskLedUpdateStateMqtt(MQTTContext_t *pContext, MQTTPublishInfo_t *pPublishInfo){

    uint8_t state;

    ( void ) pContext;

    LogInfo( ("Invoked led state callback.") );
    taskLedUpdateState( *( (uint8_t *) pPublishInfo->pPayload ) );
}
//-----------------------------------------------------------------------------
static void taskLedUpdateState(uint8_t state){

    LogInfo( ("Setting LED state to %d", state) );
    // if( state )
    //     ledSetIntensity(0, 4, 1000);
    // else
    //     ledSetIntensity(0, 0, 1000);
    
}
//-----------------------------------------------------------------------------
static void taskLedUpdateRgbMqtt(MQTTContext_t *pContext, MQTTPublishInfo_t *pPublishInfo){

    ( void ) pContext;

    LogInfo( ("Invoked led rgb callback.") );
    
    taskLedUpdateRgb( (uint8_t *) pPublishInfo->pPayload );
}
//-----------------------------------------------------------------------------
static void taskLedUpdateRgb(uint8_t *data){

    mqttmngPayload_t payload;

    LogInfo( ("Setting LED color to %d %d %d", data[0], data[1], data[2]) );

    //ledSetColor(0, data[0], data[1], data[2], 1000);
}
//-----------------------------------------------------------------------------
//=============================================================================
