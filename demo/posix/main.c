

//=============================================================================
/*-------------------------------- Includes ---------------------------------*/
//=============================================================================
#include <stdio.h>
#include "stdlib.h"

#include "mqttmng.h"
#include "mqttmngConfig.h"

#include <pthread.h>

#include "threads/task_blink.h"
#include "threads/task_svmqtt.h"
#include "threads/task_temperature.h"
#include "threads/task_led.h"
//=============================================================================

//=============================================================================
/*-------------------------------- Prototypes -------------------------------*/
//=============================================================================

//=============================================================================

//=============================================================================
/*--------------------------------- Globals ---------------------------------*/
//=============================================================================

//=============================================================================

//=============================================================================
/*----------------------------------- Main ----------------------------------*/
//=============================================================================
//-----------------------------------------------------------------------------
int main(int argc, char** argv){

	pthread_t taskBlinkHandle;
	pthread_t taskSvmqttHandle;
	pthread_t taskTemperatureHandle;
	pthread_t taskLedHandle;	

    pthread_create( &taskBlinkHandle, NULL, taskBlink, NULL );
    pthread_create( &taskSvmqttHandle, NULL, taskSvmqtt, NULL );
    pthread_create( &taskTemperatureHandle, NULL, taskTemperature, NULL );
    pthread_create( &taskLedHandle, NULL, taskLed, NULL );

    pthread_join( taskBlinkHandle, NULL );
    pthread_join( taskSvmqttHandle, NULL );
    pthread_join( taskTemperatureHandle, NULL );
    pthread_join( taskLedHandle, NULL );

	exit( 0 );

	// int id;

	// mqttmngInit(0, 0);
 	// mqttmngAddComponent(MQTT_MNG_COMP_1, (const char*)"temp1", (const char*)"temperature", (const char*)0);
	// mqttmngAddComponent(MQTT_MNG_COMP_2, (const char*)"led233", (const char*)"led", (const char*)"ri");

	// mqttmngPayload_t payload;
	// payload.data = "25";
	// payload.size = strlen(payload.data);
	// payload.dup = 0;
	// payload.retain = 0;

	// mqttmngPublish(MQTT_MNG_COMP_1, "temperature", &payload);

	// mqttmngSubscribe(MQTT_MNG_COMP_2, "intensity", precipitationDataCallback);

	// mqttmngRun();

	return 0;
}
//-----------------------------------------------------------------------------
//=============================================================================

//=============================================================================
/*---------------------------- Static functions -----------------------------*/
//=============================================================================
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//=============================================================================