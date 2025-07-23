#include <windows.h>
#include <stdint.h>
#include "clock.h"

#define MILLISECONDS_PER_SECOND        (1000L)
#define NANOSECONDS_PER_MILLISECOND    (1000000L)

/*-----------------------------------------------------------*/

uint32_t Clock_GetTimeMs( void )
{
    static LARGE_INTEGER frequency = { 0 };
    LARGE_INTEGER now;
    int64_t timeMs;

    if (frequency.QuadPart == 0)
    {
        QueryPerformanceFrequency(&frequency);
    }

    QueryPerformanceCounter(&now);

    timeMs = (now.QuadPart * MILLISECONDS_PER_SECOND) / frequency.QuadPart;
    return (uint32_t)timeMs;
}

/*-----------------------------------------------------------*/

void Clock_SleepMs( uint32_t sleepTimeMs )
{
    Sleep(sleepTimeMs);
}
