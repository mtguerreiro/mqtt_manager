//=============================================================================
/*-------------------------------- Includes ---------------------------------*/
//=============================================================================
#include "task_wiznet_init.h"

/* Kernel */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* Device and drivers */
#include "stdio.h"
#include "pico/stdlib.h"

/* Wiznet drivers */
#include "mdrivers/wiznet/dhcp.h"
#include "mdrivers/wiznet/socket.h"
#include "mdrivers/wiznet/wizchip_conf.h"

/* Pico initialzation */
#include "mhw/pico/wiznet_init.h"

/* Includes blink task to update blinking rate according to connection status */
#include "task_blink.h"

#include "task_svmqtt.h"
//=============================================================================

//=============================================================================
/*--------------------------------- Defines ---------------------------------*/
//=============================================================================

//=============================================================================

//=============================================================================
/*--------------------------------- Globals ---------------------------------*/
//=============================================================================
static SemaphoreHandle_t lock;
//=============================================================================

//=============================================================================
/*-------------------------------- Prototypes -------------------------------*/
//=============================================================================
static void taskWiznetInitInitialize(void);
static void taskWiznetInitInitializeLock(void);
static void taskWiznetInitLock(void);
static void taskWiznetInitUnlock(void);
//=============================================================================

//=============================================================================
/*---------------------------------- Task -----------------------------------*/
//=============================================================================
//-----------------------------------------------------------------------------
void taskWiznetInit(void *param){

    printf("Trying to init w5500...\n\r");
    taskWiznetInitInitialize();

    xTaskCreate(
        taskSvmqtt,
        "svmqtt",
        TASKS_SVMQTT_CONFIG_TASK_STACK_SIZE,
        NULL,
        TASKS_SVMQTT_CONFIG_TASK_PRIO,
        NULL );
    
    vTaskDelete(NULL);
}
//-----------------------------------------------------------------------------
//=============================================================================

//=============================================================================
/*---------------------------- Static functions -----------------------------*/
//=============================================================================
//-----------------------------------------------------------------------------
static void taskWiznetInitInitialize(void){

    taskBlinkUpdatePeriod(250);

    wiznetInitConfig_t config;

    taskWiznetInitInitializeLock();

    config.lock = taskWiznetInitLock;
    config.unlock = taskWiznetInitUnlock;
    wiznetInit(&config);

    taskBlinkUpdatePeriod(1000);
}
//-----------------------------------------------------------------------------
static void taskWiznetInitInitializeLock(void){

    lock = xSemaphoreCreateMutex();
    xSemaphoreGive( lock );
}
//-----------------------------------------------------------------------------
static void taskWiznetInitLock(void){

    xSemaphoreTake(lock, portMAX_DELAY);
}
//-----------------------------------------------------------------------------
static void taskWiznetInitUnlock(void){

    xSemaphoreGive( lock );
}
//-----------------------------------------------------------------------------
//=============================================================================
