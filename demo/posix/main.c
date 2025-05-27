//=============================================================================
/*-------------------------------- Includes ---------------------------------*/
//=============================================================================
#include <stdio.h>
#include "stdlib.h"
#include <pthread.h>

#include "threads/task_blink.h"
#include "threads/task_mqtt_mng.h"
#include "threads/task_temperature.h"
#include "threads/task_led.h"
//=============================================================================

//=============================================================================
/*----------------------------------- Main ----------------------------------*/
//=============================================================================
//-----------------------------------------------------------------------------
int main(int argc, char** argv){

	pthread_t taskBlinkHandle;
	pthread_t taskMqttmngHandle;
	pthread_t taskTemperatureHandle;
	pthread_t taskLedHandle;	

    pthread_create( &taskBlinkHandle, NULL, taskBlink, NULL );
    pthread_create( &taskMqttmngHandle, NULL, taskMqttmng, NULL );
    pthread_create( &taskTemperatureHandle, NULL, taskTemperature, NULL );
    pthread_create( &taskLedHandle, NULL, taskLed, NULL );

    pthread_join( taskBlinkHandle, NULL );
    pthread_join( taskMqttmngHandle, NULL );
    pthread_join( taskTemperatureHandle, NULL );
    pthread_join( taskLedHandle, NULL );

	exit( 0 );

	return 0;
}
//-----------------------------------------------------------------------------
//=============================================================================

