//=============================================================================
/*-------------------------------- Includes ---------------------------------*/
//=============================================================================
#include "task_temperature.h"

#include "stdio.h"

#include "mqttmng.h"
#include "mqttmngConfig.h"
//=============================================================================

//=============================================================================
/*--------------------------------- Globals ---------------------------------*/
//=============================================================================

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
}
//-----------------------------------------------------------------------------
//=============================================================================

//=============================================================================
/*---------------------------- Static functions -----------------------------*/
//=============================================================================
//-----------------------------------------------------------------------------
static void taskLedInitialize(void){

    while( mqttmngInitDone() != 0 );
    mqttmngPublishComponent("led223", "led", "ri");
    mqttmngSubscribe(MQTT_MNG_CONFIG_DEV_ID "/led223/state", taskLedUpdateStateMqtt);
    mqttmngSubscribe(MQTT_MNG_CONFIG_DEV_ID "/led223/rgb", taskLedUpdateRgbMqtt);
}
//-----------------------------------------------------------------------------
static void taskLedUpdateStateMqtt(MQTTContext_t *pContext, MQTTPublishInfo_t *pPublishInfo){

    ( void ) pContext;

    LogInfo( ("Invoked led state callback.") );
    taskLedUpdateState( *( (uint8_t *) pPublishInfo->pPayload ) );
}
//-----------------------------------------------------------------------------
static void taskLedUpdateState(uint8_t state){

    LogInfo( ("Setting LED state to %d", state) );
}
//-----------------------------------------------------------------------------
static void taskLedUpdateRgbMqtt(MQTTContext_t *pContext, MQTTPublishInfo_t *pPublishInfo){

    ( void ) pContext;

    LogInfo( ("Invoked led rgb callback.") );
    
    taskLedUpdateRgb( (uint8_t *) pPublishInfo->pPayload );
}
//-----------------------------------------------------------------------------
static void taskLedUpdateRgb(uint8_t *data){

    LogInfo( ("Setting LED color to %d %d %d", data[0], data[1], data[2]) );
}
//-----------------------------------------------------------------------------
//=============================================================================
