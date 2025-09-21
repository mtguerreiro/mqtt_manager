//=============================================================================
/*-------------------------------- Includes ---------------------------------*/
//=============================================================================
/* Standard */
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

/* Tasks */
#include "tasks/task_blink.h"
#include "tasks/task_temperature.h"
#include "tasks/task_led.h"
#include "tasks/task_wiznet_init.h"

/* FreeRTOS */
#include "FreeRTOS.h"
#include "task.h"

/* Pico includes */
#include "pico/stdlib.h"

#include "mhw/pico/pwm_irq_handler.h"

//=============================================================================

//=============================================================================
/*--------------------------------- Defines ---------------------------------*/
//=============================================================================

//=============================================================================

//=============================================================================
/*-------------------------------- Prototypes -------------------------------*/
//=============================================================================
static int mainSysInit(void);
//=============================================================================

//=============================================================================
/*--------------------------------- Globals ---------------------------------*/
//=============================================================================

//=============================================================================

//=============================================================================
/*---------------------------------- Main -----------------------------------*/
//=============================================================================
//-----------------------------------------------------------------------------
int main(void){

    mainSysInit();

    xTaskCreate(
        taskBlink,                          /* Function that implements the task. */
        "blink",                            /* Text name for the task. */
        TASK_BLINK_CONFIG_TASK_STACK_SIZE,  /* Stack size in words, not bytes. */
        NULL,                               /* Parameter passed into the task. */
        TASK_BLINK_CONFIG_TASK_PRIO,        /* Priority at which the task is created. */
        NULL );                             /* Used to pass out the created task's handle. */

    xTaskCreate(
        taskTemperature,                            /* Function that implements the task. */
        "temperature",                              /* Text name for the task. */
        TASK_TEMPERATURE_CONFIG_TASK_STACK_SIZE,    /* Stack size in words, not bytes. */
        NULL,                                       /* Parameter passed into the task. */
        TASK_TEMPERATURE_CONFIG_TASK_PRIO,          /* Priority at which the task is created. */
        NULL );                                     /* Used to pass out the created task's handle. */

    xTaskCreate(
        taskLed,                                    /* Function that implements the task. */
        "led",                                      /* Text name for the task. */
        TASK_LED_CONFIG_TASK_STACK_SIZE,            /* Stack size in words, not bytes. */
        NULL,                                       /* Parameter passed into the task. */
        TASK_LED_CONFIG_TASK_PRIO,                  /* Priority at which the task is created. */
        NULL );   

    xTaskCreate(
        taskWiznetInit,                             /* Function that implements the task. */
        "wzinit",                                   /* Text name for the task. */
        TASK_WIZNET_INIT_CONFIG_TASK_STACK_SIZE,    /* Stack size in words, not bytes. */
        NULL,                                       /* Parameter passed into the task. */
        TASK_WIZNET_INIT_CONFIG_TASK_PRIO,          /* Priority at which the task is created. */
        NULL );                                     /* Used to pass out the created task's handle. */

    vTaskStartScheduler();
    while(1);
}
//-----------------------------------------------------------------------------
//=============================================================================

//=============================================================================
/*---------------------------- Static functions -----------------------------*/
//=============================================================================
//-----------------------------------------------------------------------------
static int mainSysInit(void){

    stdio_init_all();

    pwmIrqHandlerInitialize(PICO_DEFAULT_IRQ_PRIORITY);

    printf("Pico has initialized\n\r");

    return 0;
}
//-----------------------------------------------------------------------------
//=============================================================================
