//=============================================================================
/*-------------------------------- Includes ---------------------------------*/
//=============================================================================
#include "task_led.h"

#include "stdio.h"

#include "mqttmng.h"
#include "mqttmngConfig.h"
#include "loggingConfig.h"
//=============================================================================

//=============================================================================
/*------------------------------- Definitions -------------------------------*/
//=============================================================================
#define LED_CFG_MQTT_COMP_NAME  "led223"
#define LED_CFG_MQTT_COMP_TYPE  "led"
#define LED_CFG_MQTT_COMP_FLAGS "ri"

#define LED_CFG_MQTT_COMP_ID    MQTT_MNG_CONFIG_DEV_ID "/" LED_CFG_MQTT_COMP_NAME
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
    mqttmngPublishComponent(
        LED_CFG_MQTT_COMP_NAME,
        LED_CFG_MQTT_COMP_TYPE,
        LED_CFG_MQTT_COMP_FLAGS
    );

    mqttmngSubscribe(LED_CFG_MQTT_COMP_ID "/state", taskLedMqttUpdateState);
    mqttmngSubscribe(LED_CFG_MQTT_COMP_ID "/rgb", taskLedMqttUpdateRgb);
    mqttmngSubscribe(LED_CFG_MQTT_COMP_ID "/intensity", taskLedMqttUpdateIntensity);
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
