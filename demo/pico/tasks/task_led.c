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
#include "mdrivers/led.h"
#include "ledws2812.h"
#include "ledpwm.h"

#include "mqttmng.h"
#include "mqttConfig.h"
#include "loggingConfig.h"
//=============================================================================

//=============================================================================
/*--------------------------------- Defines ---------------------------------*/
//=============================================================================
#define LED_CFG_MQTT_COMP_NAME  "led223"
#define LED_CFG_MQTT_COMP_TYPE  "led"

#define LED_CFG_MQTT_COMP_ID    MQTT_CONFIG_DEV_ID "/" LED_CFG_MQTT_COMP_NAME

static int32_t ledPwmIdx = -1;
static int32_t ledWsIdx = -1;
//=============================================================================

//=============================================================================
/*-------------------------------- Prototypes -------------------------------*/
//=============================================================================
static void taskLedInitialize(void);
static void taskLedInitializeLed(void);
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

    (void)param;

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

    taskLedInitializeLed();
    taskLedInitializeHwWs2812();
    taskLedInitializeHwPwm();

    while( mqttmngInitDone() != 0 ) vTaskDelay(1000 / portTICK_PERIOD_MS);
    mqttmngAddComponent(
        LED_CFG_MQTT_COMP_NAME,
        LED_CFG_MQTT_COMP_TYPE,
        "ri"
    );
    mqttmngSubscribe(LED_CFG_MQTT_COMP_ID "/state", taskLedMqttUpdateState);
    mqttmngSubscribe(LED_CFG_MQTT_COMP_ID "/rgb", taskLedMqttUpdateRgb);
    mqttmngSubscribe(LED_CFG_MQTT_COMP_ID "/intensity", taskLedMqttUpdateIntensity);
}
//-----------------------------------------------------------------------------
static void taskLedInitializeLed(void){

    ledConfig_t ledConfig;

    ledConfig.lock = 0;
    ledConfig.unlock = 0;
    ledInitialize(&ledConfig);
}
//-----------------------------------------------------------------------------
static void taskLedInitializeHwWs2812(void){

    ledws2812Initialize();

    ledDriver_t driver = {0};

    driver.setColor = ledws2812SetColor;
    driver.setIntensity = ledws2812SetIntensity;

    ledWsIdx = ledRegister(&driver, 1000);
}
//-----------------------------------------------------------------------------
static void taskLedInitializeHwPwm(void){

    ledpwmInitialize();

    ledDriver_t driver = {0};

    driver.setIntensity = ledpwmSetIntensity;

    ledPwmIdx = ledRegister(&driver, 1000);
}
//-----------------------------------------------------------------------------
static void taskLedMqttUpdateState(MQTTContext_t *pContext, MQTTPublishInfo_t *pPublishInfo){

    ( void ) pContext;
    uint8_t state;
    
    state = *( (uint8_t *) pPublishInfo->pPayload );

    LogInfo( ("Invoked led state callback with state %d.", state) );
    
    if( state )
        ledSetIntensity(ledWsIdx, 0, 4, 1000);
    else
        ledSetIntensity(ledWsIdx, 0, 0, 1000);
}
//-----------------------------------------------------------------------------
static void taskLedMqttUpdateRgb(MQTTContext_t *pContext, MQTTPublishInfo_t *pPublishInfo){

    ( void ) pContext;
    uint8_t *p;
    
    p = (uint8_t *) pPublishInfo->pPayload;

    LogInfo( ("Invoked led rgb callback with RGB: %d %d %d.", p[0], p[1], p[2]) );

    ledSetColor(ledWsIdx, 0, p[0], p[1], p[2], 1000);
}
//-----------------------------------------------------------------------------
static void taskLedMqttUpdateIntensity(MQTTContext_t *pContext, MQTTPublishInfo_t *pPublishInfo){

    ( void ) pContext;
    uint8_t intensity;
    
    intensity = *( (uint8_t *) pPublishInfo->pPayload );

    LogInfo( ("Invoked led intensity callback with intensity %d.", intensity) );

    ledSetIntensity(ledPwmIdx, 0, intensity, 1000);
}
//-----------------------------------------------------------------------------
//=============================================================================
