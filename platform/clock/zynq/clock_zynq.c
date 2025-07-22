
/* Kernel */
#include "FreeRTOS.h"
#include "task.h"

/* Device and drivers */
#include "xparameters.h"
#include "xil_types.h"
#include "xiltimer.h"

#define CLOCK_ZYNQ_TIMER_CLOCK_kHz  (XPAR_CPU_CORE_CLOCK_FREQ_HZ/2/1000)

/*-----------------------------------------------------------*/

uint32_t Clock_GetTimeMs( void )
{
    XTime t;
    uint64_t t_ms;

    XTime_GetTime( &t );
    t_ms = (uint64_t)(t) / CLOCK_ZYNQ_TIMER_CLOCK_kHz;

    return ( uint32_t ) t_ms ;
}

/*-----------------------------------------------------------*/

void Clock_SleepMs( uint32_t sleepTimeMs )
{
    vTaskDelay( sleepTimeMs / portTICK_PERIOD_MS );
}
