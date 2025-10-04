//=============================================================================
/*-------------------------------- Includes ---------------------------------*/
//=============================================================================
#include "task_led.h"

#include "stdio.h"

#include "mqttmng.h"
#include "mqttConfig.h"
#include "loggingConfig.h"
//=============================================================================

//=============================================================================
/*------------------------------- Definitions -------------------------------*/
//=============================================================================
#define LED_CFG_MQTT_COMP_NAME  "led223"
#define LED_CFG_MQTT_COMP_TYPE  "led"
#define LED_CFG_MQTT_COMP_FLAGS "ri"

//=============================================================================

//=============================================================================
/*-------------------------------- Prototypes -------------------------------*/
//=============================================================================
static void taskLedInitialize(void);
static void taskLedMqttUpdateState(MQTTContext_t *pContext, MQTTPublishInfo_t *pPublishInfo);
static void taskLedMqttUpdateRgb(MQTTContext_t *pContext, MQTTPublishInfo_t *pPublishInfo);
static void taskLedMqttUpdateIntensity(MQTTContext_t *pContext, MQTTPublishInfo_t *pPublishInfo);
//=============================================================================

//=============================================================================
/*---------------------------------- Task -----------------------------------*/
//=============================================================================
//-----------------------------------------------------------------------------
void* taskLed(void *param){

    (void)param;

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

    mqttmngAddComponent(
        LED_CFG_MQTT_COMP_NAME,
        LED_CFG_MQTT_COMP_TYPE,
        LED_CFG_MQTT_COMP_FLAGS
    );

    mqttmngSubscribeWithId(LED_CFG_MQTT_COMP_NAME "/state", taskLedMqttUpdateState);
    mqttmngSubscribeWithId(LED_CFG_MQTT_COMP_NAME "/rgb", taskLedMqttUpdateRgb);
    mqttmngSubscribeWithId(LED_CFG_MQTT_COMP_NAME "/intensity", taskLedMqttUpdateIntensity);
}
//-----------------------------------------------------------------------------
static void taskLedMqttUpdateState(MQTTContext_t *pContext, MQTTPublishInfo_t *pPublishInfo){
    
    ( void ) pContext;
    uint8_t state;
    
    state = *( (uint8_t *) pPublishInfo->pPayload );

    LogInfo( ("Invoked led state callback with state %d.", state) );
}
//-----------------------------------------------------------------------------
static void taskLedMqttUpdateRgb(MQTTContext_t *pContext, MQTTPublishInfo_t *pPublishInfo){

    ( void ) pContext;
    uint8_t *p;
    
    p = (uint8_t *) pPublishInfo->pPayload;

    LogInfo( ("Invoked led rgb callback with RGB: %d %d %d.", p[0], p[1], p[2]) );
}
//-----------------------------------------------------------------------------
static void taskLedMqttUpdateIntensity(MQTTContext_t *pContext, MQTTPublishInfo_t *pPublishInfo){

    ( void ) pContext;
    uint8_t intensity;
    
    intensity = *( (uint8_t *) pPublishInfo->pPayload );

    LogInfo( ("Invoked led intensity callback with intensity %d.", intensity) );
}
//-----------------------------------------------------------------------------
//=============================================================================
