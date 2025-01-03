/*******************************************************************************
 * Copyright (c) 2012, 2016 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution. 
 *
 * The Eclipse Public License is available at 
 *   http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at 
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Ian Craggs - initial contribution
 *    Ian Craggs - change delimiter option from char to string
 *    Al Stockdill-Mander - Version using the embedded C client
 *    Ian Craggs - update MQTTClient function names
 *******************************************************************************/

/*
 
 stdout subscriber
 
 compulsory parameters:
 
  topic to subscribe to
 
 defaulted parameters:
 
	--host localhost
	--port 1883
	--qos 2
	--delimiter \n
	--clientid stdout_subscriber
	
	--userid none
	--password none

 for example:

    stdoutsub topic/of/interest --host iot.eclipse.org

*/
#include <stdio.h>
// #include <memory.h>
// #include "MQTTClient.h"

// #include <stdio.h>
// #include <signal.h>

// #include <sys/time.h>

#include "mqttmng.h"
#include "mqttmngConfig.h"

#include "assert.h"

static void precipitationDataCallback( MQTTContext_t * pContext,
                                       MQTTPublishInfo_t * pPublishInfo )
{
    assert( pPublishInfo != NULL );
    assert( pContext != NULL );

    /* Suppress unused parameter warning when asserts are disabled in build. */
    ( void ) pContext;

    LogInfo( ( "Invoked precipitation callback." ) );

    /* Set the global flag to indicate that the humidity data has been received. */
    //globalReceivedPrecipitationData = true;

    //handleIncomingPublish( pPublishInfo );
}

#define STR(x) MQTT_MNG_CONFIG_DEV_ID "/" x

int main(int argc, char** argv)
{

	// printf(STR("temp"));
	// fflush( stdout );

	mqttmngInit();
 	mqttmngAddComponent(MQTT_MNG_COMP_1, (const char*)"temp1", (const char*)"temperature", (const char*)0);
	mqttmngAddComponent(MQTT_MNG_COMP_2, (const char*)"led233", (const char*)"led", (const char*)"ri");

	mqttmngPayload_t payload;
	payload.data = "Teste fjdsif";
	payload.size = strlen(payload.data);
	payload.dup = 0;
	payload.qos = 0;
	payload.retain = 0;

	mqttmngPublish(MQTT_MNG_COMP_1, "temperature", &payload);

	mqttmngSubscribe(MQTT_MNG_COMP_2, "intensity", 0, precipitationDataCallback);
	// mqttmngSubscribe(MQTT_MNG_COMP_2, "rgb", 0, messageArrived);

	mqttmngRun();

	return 0;
}


