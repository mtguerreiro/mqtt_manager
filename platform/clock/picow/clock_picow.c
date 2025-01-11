
#include "clock.h"

//#include "time.h"
#include "pico/stdlib.h"

#include "FreeRTOS.h"
#include "task.h"

/*-----------------------------------------------------------*/

uint32_t Clock_GetTimeMs( void )
{
    int64_t timeMs;

    return ( uint32_t ) to_ms_since_boot( get_absolute_time() );
}

/*-----------------------------------------------------------*/

void Clock_SleepMs( uint32_t sleepTimeMs )
{
    vTaskDelay( sleepTimeMs / portTICK_PERIOD_MS );
    //sleep_ms(sleepTimeMs);
}
