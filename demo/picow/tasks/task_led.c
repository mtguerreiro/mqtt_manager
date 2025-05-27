//=============================================================================
/*-------------------------------- Includes ---------------------------------*/
//=============================================================================
#include "task_led.h"

/* Kernel */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* Device and drivers */
#include "stdio.h"
#include "pico/stdlib.h"

/* Led module */
#include "mdrivers/led/led.h"
#include "mhw/pico/led/ledws2812.h"
#include "mhw/pico/led/ledpwm.h"

#include "mqttmng.h"
#include "mqttmngConfig.h"
//=============================================================================

//=============================================================================
/*--------------------------------- Defines ---------------------------------*/
//=============================================================================
#define LED_CFG_MQTT_COMP_NAME  "led223"
#define LED_CFG_MQTT_COMP_TYPE  "led"

#define LED_CFG_MQTT_COMP_ID    MQTT_MNG_CONFIG_DEV_ID "/" LED_CFG_MQTT_COMP_NAME
//=============================================================================

//=============================================================================
/*-------------------------------- Prototypes -------------------------------*/
//=============================================================================
static void taskLedInitialize(void);
static void taskLedInitializeHwWs2812(void);
static void taskLedInitializeHwPwm(void);
static void taskLedMqttUpdateState(MQTTContext_t *pContext, MQTTPublishInfo_t *pPublishInfo);
static void taskLedMqttUpdateRgb(MQTTContext_t *pContext, MQTTPublishInfo_t *pPublishInfo);
static void taskLedMqttUpdateIntensity(MQTTContext_t *pContext, MQTTPublishInfo_t *pPublishInfo);
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

    //taskLedInitializeHwWs2812();
    taskLedInitializeHwPwm();
}
//-----------------------------------------------------------------------------
static void taskLedInitializeHwWs2812(void){

    ledws2812Initialize();

    ledConfig_t ledConfig;

    ledConfig.lock = 0;
    ledConfig.unlock = 0;
    ledConfig.hwSetColor = ledws2812SetColor;
    ledConfig.hwSetIntensity = ledws2812SetIntensity;
    ledConfig.hwGetNumberLeds = ledws2812GetNumberLeds;

    ledInitialize(&ledConfig);

    while( mqttmngInitDone() != 0 );
    mqttmngPublishComponent(
        LED_CFG_MQTT_COMP_NAME,
        LED_CFG_MQTT_COMP_TYPE,
        "ri"
    );

    mqttmngSubscribe(LED_CFG_MQTT_COMP_ID "/state", taskLedMqttUpdateState);
    mqttmngSubscribe(LED_CFG_MQTT_COMP_ID "/rgb", taskLedMqttUpdateRgb);
    mqttmngSubscribe(LED_CFG_MQTT_COMP_ID "/intensity", taskLedMqttUpdateIntensity);
}
//-----------------------------------------------------------------------------
static void taskLedInitializeHwPwm(void){

    ledpwmInitialize();

    ledConfig_t ledConfig;

    ledConfig.lock = 0;
    ledConfig.unlock = 0;
    ledConfig.hwSetColor = 0;
    ledConfig.hwSetIntensity = ledpwmSetIntensity;
    ledConfig.hwGetNumberLeds = ledpwmGetNumberLeds;

    ledInitialize(&ledConfig);

    while( mqttmngInitDone() != 0 );
    mqttmngPublishComponent(
        LED_CFG_MQTT_COMP_NAME,
        LED_CFG_MQTT_COMP_TYPE,
        "i"
    );

    mqttmngSubscribe(LED_CFG_MQTT_COMP_ID "/state", taskLedMqttUpdateState);
    mqttmngSubscribe(LED_CFG_MQTT_COMP_ID "/intensity", taskLedMqttUpdateIntensity);
}
//-----------------------------------------------------------------------------
static void taskLedMqttUpdateState(MQTTContext_t *pContext, MQTTPublishInfo_t *pPublishInfo){

    ( void ) pContext;
    uint8_t state;
    
    state = *( (uint8_t *) pPublishInfo->pPayload );

    LogInfo( ("Invoked led state callback with state %d.", state) );
    
    if( state )
        ledSetIntensity(0, 4, 1000);
    else
        ledSetIntensity(0, 0, 1000);
}
//-----------------------------------------------------------------------------
static void taskLedMqttUpdateRgb(MQTTContext_t *pContext, MQTTPublishInfo_t *pPublishInfo){

    ( void ) pContext;
    uint8_t *p;
    
    p = (uint8_t *) pPublishInfo->pPayload;

    LogInfo( ("Invoked led rgb callback with RGB: %d %d %d.", p[0], p[1], p[2]) );

    ledSetColor(0, p[0], p[1], p[2], 1000);
}
//-----------------------------------------------------------------------------
static void taskLedMqttUpdateIntensity(MQTTContext_t *pContext, MQTTPublishInfo_t *pPublishInfo){

    ( void ) pContext;
    uint8_t intensity;
    
    intensity = *( (uint8_t *) pPublishInfo->pPayload );

    LogInfo( ("Invoked led intensity callback with intensity %d.", intensity) );

    ledSetIntensity(0, intensity, 0);
}
//-----------------------------------------------------------------------------
//=============================================================================
