//=============================================================================
/*-------------------------------- Includes ---------------------------------*/
//=============================================================================
#include "task_led.h"

/* Kernel */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

/* Device and drivers */
#include "stdio.h"
#include "led_strip.h"

#include "mqttmng.h"
#include "mqttmngConfig.h"
#include "loggingConfig.h"
//=============================================================================

//=============================================================================
/*--------------------------------- Defines ---------------------------------*/
//=============================================================================
#define LED_CFG_MQTT_COMP_NAME      "led223"
#define LED_CFG_MQTT_COMP_TYPE      "led"
#define LED_CFG_MQTT_COMP_FLAGS     "ri"

#define LED_CFG_MQTT_COMP_ID    MQTT_MNG_CONFIG_DEV_ID "/" LED_CFG_MQTT_COMP_NAME

#define LED_GPIO 23
//=============================================================================

//=============================================================================
/*--------------------------------- Globals ---------------------------------*/
//=============================================================================
led_strip_handle_t led_strip;
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

    /// LED strip common configuration
    led_strip_config_t strip_config = {
        .strip_gpio_num = LED_GPIO,  // The GPIO that connected to the LED strip's data line
        .max_leds = 5,                 // The number of LEDs in the strip,
        .led_model = LED_MODEL_WS2812, // LED strip model, it determines the bit timing
        .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB, // The color component format is G-R-B
        .flags = {
            .invert_out = false, // don't invert the output signal
        }
    };

    /// RMT backend specific configuration
    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,    // different clock source can lead to different power consumption
        .resolution_hz = 10 * 1000 * 1000, // RMT counter clock frequency: 10MHz
        .mem_block_symbols = 64,           // the memory size of each RMT channel, in words (4 bytes)
        .flags = {
            .with_dma = false, // DMA feature is available on chips like ESP32-S3/P4
        }
    };

    /// Create the LED strip object
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));

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

    /* Set the LED pixel using RGB from 0 (0%) to 255 (100%) for each color */
    led_strip_set_pixel(led_strip, 0, p[0], p[1], p[2]);
    led_strip_set_pixel(led_strip, 2, p[0], p[1], p[2]);
    /* Refresh the strip to send data */
    led_strip_refresh(led_strip);

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
