//=============================================================================
/*-------------------------------- Includes ---------------------------------*/
//=============================================================================
#include "task_temperature.h"

/* Kernel */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

/* Device and drivers */
#include "stdio.h"
#include "led_strip.h"

#include "mqttmng.h"
#include "mqttmngConfig.h"
//=============================================================================

//=============================================================================
/*--------------------------------- Defines ---------------------------------*/
//=============================================================================
#define LED_GPIO 23
//=============================================================================

//=============================================================================
/*--------------------------------- Globals ---------------------------------*/
//=============================================================================
mqttmngSubscrConfig_t mqttsubscr[2];
mqttmngSubscrConfig_t *mqttsubscrptr[2];
mqttmngConfig_t mqttconfig;
led_strip_handle_t led_strip;
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

    //if( state )
    //    ledSetIntensity(0, 4, 1000);
    //else
    //    ledSetIntensity(0, 0, 1000);
    
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

    /* Set the LED pixel using RGB from 0 (0%) to 255 (100%) for each color */
    led_strip_set_pixel(led_strip, 0, data[0], data[1], data[2]);
    led_strip_set_pixel(led_strip, 2, data[0], data[1], data[2]);
    /* Refresh the strip to send data */
    led_strip_refresh(led_strip);

    //ledSetColor(0, data[0], data[1], data[2], 1000);
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
