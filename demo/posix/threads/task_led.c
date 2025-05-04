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

    mqttmngAddComponent(MQTT_MNG_COMP_2, (const char*)"led233", (const char*)"led", (const char*)"ri");
    while( mqttmngInitDone() != 0 );

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
