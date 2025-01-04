
#include <stdio.h>

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

int main(int argc, char** argv)
{

	mqttmngInit();
 	mqttmngAddComponent(MQTT_MNG_COMP_1, (const char*)"temp1", (const char*)"temperature", (const char*)0);
	mqttmngAddComponent(MQTT_MNG_COMP_2, (const char*)"led233", (const char*)"led", (const char*)"ri");

	mqttmngPayload_t payload;
	payload.data = "25";
	payload.size = strlen(payload.data);
	payload.dup = 0;
	payload.qos = 0;
	payload.retain = 0;

	mqttmngPublish(MQTT_MNG_COMP_1, "temperature", &payload);

	mqttmngSubscribe(MQTT_MNG_COMP_2, "intensity", 0, precipitationDataCallback);

	mqttmngRun();

	return 0;
}


