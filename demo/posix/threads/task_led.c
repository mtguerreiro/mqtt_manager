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

    mqttmngPayload_t payload;

    LogInfo( ("Setting LED color to %d %d %d", data[0], data[1], data[2]) );
}
//-----------------------------------------------------------------------------
//=============================================================================
