
#include "clock.h"

#include "esp_timer.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/*-----------------------------------------------------------*/

uint32_t Clock_GetTimeMs( void )
{
    int64_t timems;

    timems = esp_timer_get_time() / 1000;

    return ( uint32_t ) timems );
}

/*-----------------------------------------------------------*/

void Clock_SleepMs( uint32_t sleepTimeMs )
{
    vTaskDelay( sleepTimeMs / portTICK_PERIOD_MS );
    //sleep_ms(sleepTimeMs);
}
