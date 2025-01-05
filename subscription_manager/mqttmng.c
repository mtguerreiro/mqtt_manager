/*
 * This implementation is based on AWS IOT SDK's MQTT subscription manager
 * example, and coreMQTT (a submodule of svmqtt).
 * 
 * https://github.com/aws/aws-iot-device-sdk-embedded-C/tree/main/demos/mqtt/mqtt_demo_subscription_manager 
 *
 */
//=============================================================================
/*-------------------------------- Includes ---------------------------------*/
//=============================================================================
#include "mqttmng.h"

#include "mqttmngConfig.h"

#include "stdlib.h"
#include "stdio.h"
#include "assert.h"

// #include "signal.h"
// #include "stdio.h"
// #include "string.h"

/* Include Demo Config as the first non-system header. */
//#include "demo_config.h"

/* MQTT API header. */
#include "core_mqtt.h"

/* Plaintext sockets transport implementation. */
#include "plaintext_transport.h"
#include "transport_interface.h"

/* Clock for timer. */
#include "clock.h"

#include "mqtt_subscription_manager.h"
//============================================================================

//=============================================================================
/*------------------------------- Definitions -------------------------------*/
//=============================================================================
/**
 * These configuration settings are required to run the basic TLS demo.
 * Throw compilation error if the below configs are not defined.
 */
#ifndef BROKER_ENDPOINT
    #error "Please define an MQTT broker endpoint, BROKER_ENDPOINT, in demo_config.h."
#endif
#ifndef CLIENT_IDENTIFIER
    #error "Please define a unique CLIENT_IDENTIFIER."
#endif

/**
 * @brief Length of MQTT server host name.
 */
#define BROKER_ENDPOINT_LENGTH      ( ( uint16_t ) ( sizeof( BROKER_ENDPOINT ) - 1 ) )

/**
 * @brief Length of path to server certificate.
 */
#define ROOT_CA_CERT_PATH_LENGTH    ( ( uint16_t ) ( sizeof( ROOT_CA_CERT_PATH ) - 1 ) )

/**
 * Provide default values for undefined configuration settings.
 */
#ifndef BROKER_PORT
    #define BROKER_PORT    ( 8883 )
#endif

#ifndef NETWORK_BUFFER_SIZE
    #define NETWORK_BUFFER_SIZE    ( 1024U )
#endif

/**
 * @brief Length of client identifier.
 */
#define CLIENT_IDENTIFIER_LENGTH                 ( ( uint16_t ) ( sizeof( CLIENT_IDENTIFIER ) - 1 ) )

/**
 * @brief The maximum number of retries for connecting to server.
 */
#define CONNECTION_RETRY_MAX_ATTEMPTS            ( 5U )

/**
 * @brief The maximum back-off delay (in milliseconds) for retrying connection to server.
 */
#define CONNECTION_RETRY_MAX_BACKOFF_DELAY_MS    ( 5000U )

/**
 * @brief The base back-off delay (in milliseconds) to use for connection retry attempts.
 */
#define CONNECTION_RETRY_BACKOFF_BASE_MS         ( 500U )


/**
 * @brief Timeout for receiving CONNACK packet in milli seconds.
 */
#define CONNACK_RECV_TIMEOUT_MS                 ( 1000U )

/**
 * @brief The MQTT topic filter to subscribe to for temperature data in the demo.
 */
#define DEMO_TEMPERATURE_TOPIC_FILTER           CLIENT_IDENTIFIER "/demo/temperature/+"

/**
 * @brief The length of the temperature topic filter.
 */
#define DEMO_TEMPERATURE_TOPIC_FILTER_LENGTH    ( ( uint16_t ) ( sizeof( DEMO_TEMPERATURE_TOPIC_FILTER ) - 1U ) )

/**
 * @brief The MQTT topic for the high temperature data value that the demo will
 * publish to.
 */
#define DEMO_TEMPERATURE_HIGH_TOPIC             CLIENT_IDENTIFIER "/demo/temperature/high"

/**
 * @brief The length of the high temperature topic name.
 */
#define DEMO_TEMPERATURE_HIGH_TOPIC_LENGTH      ( ( uint16_t ) ( sizeof( DEMO_TEMPERATURE_HIGH_TOPIC ) - 1U ) )

/**
 * @brief The MQTT topic for the high temperature data value that the demo will
 * publish to.
 */
#define DEMO_TEMPERATURE_LOW_TOPIC              CLIENT_IDENTIFIER "/demo/temperature/low"

/**
 * @brief The length of the high temperature topic name.
 */
#define DEMO_TEMPERATURE_LOW_TOPIC_LENGTH       ( ( uint16_t ) ( sizeof( DEMO_TEMPERATURE_LOW_TOPIC ) - 1U ) )

/**
 * @brief The MQTT topic filter to subscribe and publish to for humidity data in the demo.
 *
 * The topic name starts with the client identifier to ensure that each demo
 * interacts with a unique topic name.
 */
#define DEMO_HUMIDITY_TOPIC                     CLIENT_IDENTIFIER "/demo/humidity"

/**
 * @brief The length of the humidity topic.
 */
#define DEMO_HUMIDITY_TOPIC_LENGTH              ( ( uint16_t ) ( sizeof( DEMO_HUMIDITY_TOPIC ) - 1U ) )

/**
 * @brief The MQTT topic filter to subscribe and publish to for precipitation data in the demo.
 *
 * The topic name starts with the client identifier to ensure that each demo
 * interacts with a unique topic name.
 */
#define DEMO_PRECIPITATION_TOPIC                CLIENT_IDENTIFIER "/demo/precipitation"

/**
 * @brief The length of the precipitation topic.
 */
#define DEMO_PRECIPITATION_TOPIC_LENGTH         ( ( uint16_t ) ( sizeof( DEMO_PRECIPITATION_TOPIC ) - 1U ) )

/**
 * @brief The MQTT message for the #DEMO_TEMPERATURE_HIGH_TOPIC topic published
 * in this example.
 */
#define DEMO_TEMPERATURE_HIGH_MESSAGE           "Today's High is 80 degree F"

/**
 * @brief The MQTT message for the #DEMO_TEMPERATURE_LOW_TOPIC topic published
 * in this example.
 */
#define DEMO_TEMPERATURE_LOW_MESSAGE            "Today's Low 52 degree F"

/**
 * @brief The MQTT message for the #DEMO_HUMIDITY_TOPIC topic published in this example.
 */
#define DEMO_HUMIDITY_MESSAGE                   "Today's humidity at 58%"

/**
 * @brief The MQTT message for the #DEMO_PRECIPITATION_TOPIC_LENGTH topic published in this example.
 */
#define DEMO_PRECIPITATION_MESSAGE              "Today's precipitation at 9%"

/**
 * @brief Timeout for MQTT_ProcessLoop function in milliseconds.
 */
#define MQTT_PROCESS_LOOP_TIMEOUT_MS            ( 500U )

/**
 * @brief The maximum time interval in seconds which is allowed to elapse
 * between two Control Packets.
 *
 * It is the responsibility of the Client to ensure that the interval between
 * Control Packets being sent does not exceed the this Keep Alive value. In the
 * absence of sending any other Control Packets, the Client MUST send a
 * PINGREQ Packet.
 */
#define MQTT_KEEP_ALIVE_INTERVAL_SECONDS        ( 60U )

/**
 * @brief Delay in seconds between two iterations of subscribePublishLoop().
 */
#define MQTT_SUBPUB_LOOP_DELAY_SECONDS          ( 5U )

/**
 * @brief Transport timeout in milliseconds for transport send and receive.
 */
#define TRANSPORT_SEND_RECV_TIMEOUT_MS          ( 1000 )

/**
 * @brief The length of the outgoing publish records array used by the coreMQTT
 * library to track QoS > 0 packet ACKS for outgoing publishes.
 */
#define OUTGOING_PUBLISH_RECORD_LEN             ( 10U )

/**
 * @brief The length of the incoming publish records array used by the coreMQTT
 * library to track QoS > 0 packet ACKS for incoming publishes.
 */
#define INCOMING_PUBLISH_RECORD_LEN             ( 10U )

/*-----------------------------------------------------------*/

/**
 * @brief Structure to keep the MQTT publish packets until an ack is received
 * for QoS1 publishes.
 */
typedef struct PublishPackets
{
    /**
     * @brief Packet identifier of the publish packet.
     */
    uint16_t packetId;

    /**
     * @brief Publish info of the publish packet.
     */
    MQTTPublishInfo_t pubInfo;
} PublishPackets_t;

/*-----------------------------------------------------------*/

/**
 * @brief Packet Identifier updated when an ACK packet is received.
 *
 * It is used to match an expected ACK for a transmitted packet.
 */
static uint16_t globalAckPacketIdentifier = 0U;

/**
 * @brief Packet Identifier generated when Subscribe request was sent to the broker;
 * it is used to match received Subscribe ACK to the transmitted subscribe.
 */
static uint16_t lastSubscribePacketIdentifier = 0U;

/**
 * @brief Packet Identifier generated when Unsubscribe request was sent to the broker;
 * it is used to match received Unsubscribe ACK to the transmitted unsubscribe
 * request.
 */
static uint16_t lastUnsubscribePacketIdentifier = 0U;

/**
 * @brief The network buffer must remain valid for the lifetime of the MQTT context.
 */
static uint8_t buffer[ NETWORK_BUFFER_SIZE ];

/**
 * @brief Flag to represent whether that the temperature callback has been invoked with
 * incoming PUBLISH message for high temperature data.
 */
static bool globalReceivedHighTemperatureData = false;

/**
 * @brief Flag to represent whether that the temperature callback has been invoked with
 * incoming PUBLISH message for low temperature data.
 */
static bool globalReceivedLowTemperatureData = false;

/**
 * @brief Flag to represent whether that the humidity topic callback has been invoked.
 */
static bool globalReceivedHumidityData = false;

/**
 * @brief Flag to represent whether that the precipitation topic callback has been invoked.
 */
static bool globalReceivedPrecipitationData = false;

/**
 * @brief Array to track the outgoing publish records for outgoing publishes
 * with QoS > 0.
 *
 * This is passed into #MQTT_InitStatefulQoS to allow for QoS > 0.
 *
 */
static MQTTPubAckInfo_t pOutgoingPublishRecords[ OUTGOING_PUBLISH_RECORD_LEN ];

/**
 * @brief Array to track the incoming publish records for incoming publishes
 * with QoS > 0.
 *
 * This is passed into #MQTT_InitStatefulQoS to allow for QoS > 0.
 *
 */
static MQTTPubAckInfo_t pIncomingPublishRecords[ INCOMING_PUBLISH_RECORD_LEN ];

/*-----------------------------------------------------------*/

/*-----------------------------------------------------------*/

/* Each compilation unit must define the NetworkContext struct. */
struct NetworkContext
{
    PlaintextParams_t * pParams;
};

#define MQTT_MNG_SUBS_BUFFER_SIZE   128
#define MQTT_MNG_WRITE_BUFFER_SIZE  128
#define MQTT_MNG_READ_BUFFER_SIZE   128
#define MQTT_MNG_DEV_ID_LEN         strlen(MQTT_MNG_CONFIG_DEV_ID)

#define MQTT_MNG_DBF_PREFIX         "mqttmng - "

typedef struct{

    const char *names[MQTT_MNG_COMP_END];
    const char *types[MQTT_MNG_COMP_END];
    const char *flags[MQTT_MNG_COMP_END];

    unsigned char writebuf[MQTT_MNG_WRITE_BUFFER_SIZE];
    unsigned char readbuf[MQTT_MNG_READ_BUFFER_SIZE];

    char subsbuf[MQTT_MNG_SUBS_BUFFER_SIZE];
    uint32_t subsbufSize;

    MQTTContext_t mqttContext;
    NetworkContext_t networkContext;
    PlaintextParams_t plaintextParams;

}mqttmng_t;

//=============================================================================

//=============================================================================
/*-------------------------------- Prototypes -------------------------------*/
//=============================================================================
static void mqttmngEventCallback( MQTTContext_t * pMqttContext,
                                  MQTTPacketInfo_t * pPacketInfo,
                                  MQTTDeserializedInfo_t * pDeserializedInfo );

static int mqttmngEstablishMqttSession( MQTTContext_t * pMqttContext,
                                        NetworkContext_t * pNetworkContext );

static int mqttmngSubscribeToTopic( MQTTContext_t * pMqttContext, 
                                    const char * pTopicFilter, 
                                    uint16_t topicFilterLength,
                                    uint16_t qos );

static int mqttmngWaitForPacketAck( MQTTContext_t * pMqttContext,
                                    uint16_t usPacketIdentifier,
                                    uint32_t ulTimeout );

static int32_t mqttmngPublishListComponents(void);

static int32_t mqttmngPublishBare(const char *topic, mqttmngPayload_t *payload);
//=============================================================================

//=============================================================================
/*--------------------------------- Globals ---------------------------------*/
//=============================================================================
mqttmng_t mqttmng = {
    .names = {0}, .flags = {0}, .subsbufSize = 0,
    .mqttContext = {0}, .networkContext = {0}, .plaintextParams = {0}
    };

//=============================================================================

//=============================================================================
/*-------------------------------- Functions --------------------------------*/
//=============================================================================
//-----------------------------------------------------------------------------
int32_t mqttmngInit(void){

    mqttmng.networkContext.pParams = &mqttmng.plaintextParams;

    NetworkContext_t * pNetworkContext = &mqttmng.networkContext;
    MQTTContext_t * pMqttContext = &mqttmng.mqttContext;

    int returnStatus = EXIT_FAILURE;
    SocketStatus_t socketStatus = SOCKETS_SUCCESS;
    ServerInfo_t serverInfo;

    /* Initialize information to connect to the MQTT broker. */
    serverInfo.pHostName = BROKER_ENDPOINT;
    serverInfo.hostNameLength = BROKER_ENDPOINT_LENGTH;
    serverInfo.port = BROKER_PORT;

    /* Establish a TCP connection with the MQTT broker. This example connects
     * to the MQTT broker as specified in BROKER_ENDPOINT and BROKER_PORT
     * at the demo config header. */
    LogInfo( ( "Creating a TCP connection to %.*s:%d.",
                BROKER_ENDPOINT_LENGTH,
                BROKER_ENDPOINT,
                BROKER_PORT ) );
    socketStatus = Plaintext_Connect( pNetworkContext,
                                        &serverInfo,
                                        TRANSPORT_SEND_RECV_TIMEOUT_MS,
                                        TRANSPORT_SEND_RECV_TIMEOUT_MS );

    if( socketStatus != SOCKETS_SUCCESS ) return -1;

    /* Establish MQTT session on top of TCP connection. */
    LogInfo( ( "Creating an MQTT connection to %.*s.",
                BROKER_ENDPOINT_LENGTH,
                BROKER_ENDPOINT ) );

    /* Sends an MQTT Connect packet over the already connected TCP socket
        * tcpSocket, and waits for connection acknowledgment (CONNACK) packet. */
    returnStatus = mqttmngEstablishMqttSession( pMqttContext, pNetworkContext );

    if( returnStatus == EXIT_FAILURE )
    {
        /* Close the TCP connection.  */
        ( void ) Plaintext_Disconnect( pNetworkContext );
        return -2;
    }

    return 0;
}
//-----------------------------------------------------------------------------
void mqttmngRun(void){

    mqttmngPublishListComponents();

	while (1)
	{
        MQTT_ProcessLoop( &mqttmng.mqttContext );
        Clock_SleepMs(1000);
	}
}
//-----------------------------------------------------------------------------
int32_t mqttmngAddComponent(uint32_t id, const char *name, const char *type, const char *flags){

    if( id > MQTT_MNG_COMP_END ) return -1;

    mqttmng.names[id] = name;
    mqttmng.types[id] = type;
    mqttmng.flags[id] = flags;

#if( MQTT_MNG_CONFIG_DBG )
	printf("%sAdded component %s with type %s and flags [%s]\n", MQTT_MNG_DBF_PREFIX, name, type, flags);
#endif

    return 0;
}
//-----------------------------------------------------------------------------
int32_t mqttmngPublish(uint32_t id, const char *topic, mqttmngPayload_t *payload){
    
    int returnStatus = EXIT_SUCCESS;

    if( id >= MQTT_MNG_COMP_END ) return -1;

    char buf[MQTT_MNG_WRITE_BUFFER_SIZE] = {0};
    uint32_t len;
    
    len = snprintf(
        buf,
        MQTT_MNG_WRITE_BUFFER_SIZE,
        "%s/%s/%s", 
        MQTT_MNG_CONFIG_DEV_ID, 
        mqttmng.names[id], 
        topic        
    );

   if( (len + 1) >= MQTT_MNG_WRITE_BUFFER_SIZE ) return -2;

#if( MQTT_MNG_CONFIG_DBG )
    printf("%sPublishing to %s\n", MQTT_MNG_DBF_PREFIX, buf);
    fflush( stdout );
#endif

    returnStatus = mqttmngPublishBare(buf, payload);

#if( MQTT_MNG_CONFIG_DBG )
    printf("%sPublish status %d\n", MQTT_MNG_DBF_PREFIX, returnStatus);
    fflush( stdout );
#endif

    return returnStatus;
}
//-----------------------------------------------------------------------------
int32_t mqttmngSubscribe(uint32_t id, const char *topic, uint32_t qos, mqttmngSubscrCb_t callback){


    if( id >= MQTT_MNG_COMP_END ) return -1;

    uint32_t len;

    len = snprintf(&mqttmng.subsbuf[mqttmng.subsbufSize], 
        MQTT_MNG_SUBS_BUFFER_SIZE - mqttmng.subsbufSize, 
        "%s/%s/%s", 
        MQTT_MNG_CONFIG_DEV_ID, 
        mqttmng.names[id], 
        topic);

    if( (len + 1) >= (MQTT_MNG_SUBS_BUFFER_SIZE - mqttmng.subsbufSize) ) return -2;

    mqttmng.subsbufSize += len + 1;

#if( MQTT_MNG_CONFIG_DBG )
    printf("%sSubscribing to %s\n", MQTT_MNG_DBF_PREFIX, &mqttmng.subsbuf[mqttmng.subsbufSize - len - 1]);
    fflush( stdout );
#endif

    int returnStatus = EXIT_SUCCESS;
    SubscriptionManagerStatus_t managerStatus = ( SubscriptionManagerStatus_t ) 0u;
    MQTTContext_t * pContext = &mqttmng.mqttContext;

    /* Register the topic filter and its callback with subscription manager.
     * On an incoming PUBLISH message whose topic name that matches the topic filter
     * being registered, its callback will be invoked. */
    managerStatus = SubscriptionManager_RegisterCallback( &mqttmng.subsbuf[mqttmng.subsbufSize - len - 1],
                                                          len,
                                                          callback );

    if( managerStatus != SUBSCRIPTION_MANAGER_SUCCESS )
    {
        returnStatus = EXIT_FAILURE;
    }
    else
    {
        LogInfo( ( "Subscribing to the MQTT topic %.*s.",
                   len,
                   &mqttmng.subsbuf[mqttmng.subsbufSize - len - 1] ) );

        returnStatus = mqttmngSubscribeToTopic( pContext,
                                         &mqttmng.subsbuf[mqttmng.subsbufSize - len - 1],
                                         len,
                                         qos );
    }

    if( returnStatus != EXIT_SUCCESS )
    {
        /* Remove the registered callback for the temperature topic filter as
        * the subscription operation for the topic filter did not succeed. */
        ( void ) SubscriptionManager_RemoveCallback( &mqttmng.subsbuf[mqttmng.subsbufSize - len - 1],
                                                     len );
    }

#if( MQTT_MNG_CONFIG_DBG )
    printf("%sSubscription status %d\n", MQTT_MNG_DBF_PREFIX, returnStatus);
    fflush( stdout );
#endif

    return returnStatus;
}
//-----------------------------------------------------------------------------
//=============================================================================


//=============================================================================
/*---------------------------- Static functions -----------------------------*/
//=============================================================================
//-----------------------------------------------------------------------------
static int32_t mqttmngPublishListComponents(void){

    int returnStatus;
    char buf[MQTT_MNG_WRITE_BUFFER_SIZE] = {0};
    uint32_t k;

    /* TODO: improve forming the component topic's message */
    for(k = 0; k < MQTT_MNG_COMP_END; k++){
        strlcat(buf, mqttmng.names[k], sizeof(buf));
        strlcat(buf, ":", sizeof(buf));
        strlcat(buf, mqttmng.types[k], sizeof(buf));        
        if( mqttmng.flags[k] && strlen(mqttmng.flags[k]) ){
            strlcat(buf, "-", sizeof(buf));
            strlcat(buf, mqttmng.flags[k], sizeof(buf));
        }
        strlcat(buf, ";", sizeof(buf));
    }

	mqttmngPayload_t payload;
	payload.data = buf;
	payload.size = strlen(buf);
	payload.dup = 0;
	payload.qos = 0;
	payload.retain = 1;

#if( MQTT_MNG_CONFIG_DBG )
    printf("%sPublishing components %s to %s\n", MQTT_MNG_DBF_PREFIX, buf, MQTT_MNG_CONFIG_COMPONENTS_TOPIC);
    fflush( stdout );
#endif

    returnStatus = mqttmngPublishBare(MQTT_MNG_CONFIG_COMPONENTS_TOPIC, &payload);

#if( MQTT_MNG_CONFIG_DBG )
    printf("%sPublish status %d\n", MQTT_MNG_DBF_PREFIX, returnStatus);
    fflush( stdout );
#endif

    return returnStatus;
}
//-----------------------------------------------------------------------------
static int32_t mqttmngPublishBare(const char *topic, mqttmngPayload_t *payload){

    int returnStatus = EXIT_SUCCESS;

    MQTTStatus_t mqttStatus = MQTTSuccess;
    MQTTPublishInfo_t publishInfo;
    uint16_t pubPacketId = MQTT_PACKET_ID_INVALID;
    MQTTContext_t * pMqttContext = &mqttmng.mqttContext;

    assert( pMqttContext != NULL );

    ( void ) memset( &publishInfo, 0x00, sizeof( MQTTPublishInfo_t ) );

    if( returnStatus == EXIT_FAILURE )
    {
        LogError( ( "Unable to find a free spot for outgoing PUBLISH message.\n\n" ) );
    }
    else
    {
        publishInfo.qos = payload->qos;
        publishInfo.pTopicName = topic;
        publishInfo.topicNameLength = strlen(topic);
        publishInfo.pPayload = payload->data;
        publishInfo.payloadLength = payload->size;
        publishInfo.retain = payload->retain;

        /* Get a new packet ID for the publish. */
        pubPacketId = MQTT_GetPacketId( pMqttContext );

        /* Send PUBLISH packet. */
        mqttStatus = MQTT_Publish( pMqttContext,
                                   &publishInfo,
                                   pubPacketId );

        if( mqttStatus != MQTTSuccess )
        {
            LogError( ( "Failed to send PUBLISH packet to broker with error = %u.",
                        mqttStatus ) );
            returnStatus = EXIT_FAILURE;
        }

    }

#if( MQTT_MNG_CONFIG_DBG )
    printf("%sPublish status %d\n", MQTT_MNG_DBF_PREFIX, returnStatus);
    fflush( stdout );
#endif

    return returnStatus;
}
//-----------------------------------------------------------------------------
static void mqttmngEventCallback( MQTTContext_t * pMqttContext,
                                  MQTTPacketInfo_t * pPacketInfo,
                                  MQTTDeserializedInfo_t * pDeserializedInfo )
{
    assert( pMqttContext != NULL );
    assert( pPacketInfo != NULL );
    assert( pDeserializedInfo != NULL );
    //assert( pDeserializedInfo->packetIdentifier != MQTT_PACKET_ID_INVALID );

    LogInfo( ("Packet id: %d", pDeserializedInfo->packetIdentifier) );
    /* Handle incoming publish. The lower 4 bits of the publish packet
     * type is used for the dup, QoS, and retain flags. Hence masking
     * out the lower bits to check if the packet is publish. */
    if( ( pPacketInfo->type & 0xF0U ) == MQTT_PACKET_TYPE_PUBLISH )
    {
        assert( pDeserializedInfo->pPublishInfo != NULL );
        /* Handle incoming publish. */
        SubscriptionManager_DispatchHandler( pMqttContext, pDeserializedInfo->pPublishInfo );
    }
    else
    {
        /* Handle other packets. */
        switch( pPacketInfo->type )
        {
            case MQTT_PACKET_TYPE_SUBACK:
                LogInfo( ( "Received SUBACK.\n\n" ) );
                /* Make sure ACK packet identifier matches with Request packet identifier. */
                assert( lastSubscribePacketIdentifier == pDeserializedInfo->packetIdentifier );

                /* Update the global ACK packet identifier. */
                globalAckPacketIdentifier = pDeserializedInfo->packetIdentifier;
                break;

            case MQTT_PACKET_TYPE_UNSUBACK:
                LogInfo( ( "Received UNSUBACK.\n\n" ) );
                /* Make sure ACK packet identifier matches with Request packet identifier. */
                assert( lastUnsubscribePacketIdentifier == pDeserializedInfo->packetIdentifier );

                /* Update the global ACK packet identifier. */
                globalAckPacketIdentifier = pDeserializedInfo->packetIdentifier;
                break;

            case MQTT_PACKET_TYPE_PINGRESP:

                /* Nothing to be done from application as library handles
                 * PINGRESP. */
                LogWarn( ( "PINGRESP should not be handled by the application "
                           "callback when using MQTT_ProcessLoop.\n\n" ) );
                break;

            case MQTT_PACKET_TYPE_PUBACK:
                LogInfo( ( "PUBACK received for packet id %u.\n\n",
                           pDeserializedInfo->packetIdentifier ) );

                /* Update the global ACK packet identifier. */
                globalAckPacketIdentifier = pDeserializedInfo->packetIdentifier;
                break;

            /* Any other packet type is invalid. */
            default:
                LogError( ( "Unknown packet type received:(%02x).\n\n",
                            pPacketInfo->type ) );
        }
    }
}
//-----------------------------------------------------------------------------
static int mqttmngEstablishMqttSession( MQTTContext_t * pMqttContext,
                                        NetworkContext_t * pNetworkContext )
{
    int returnStatus = EXIT_SUCCESS;
    MQTTStatus_t mqttStatus;
    MQTTConnectInfo_t connectInfo;
    bool sessionPresent;
    MQTTFixedBuffer_t networkBuffer;
    TransportInterface_t transport = { NULL };

    assert( pMqttContext != NULL );
    assert( pNetworkContext != NULL );

    /* Fill in TransportInterface send and receive function pointers.
     * For this demo, TCP sockets are used to send and receive data
     * from network. Network context is socket file descriptor.*/
    transport.pNetworkContext = pNetworkContext;
    transport.send = Plaintext_Send;
    transport.recv = Plaintext_Recv;
    transport.writev = NULL;

    /* Fill the values for network buffer. */
    networkBuffer.pBuffer = buffer;
    networkBuffer.size = NETWORK_BUFFER_SIZE;

    /* Initialize MQTT library. */
    mqttStatus = MQTT_Init( pMqttContext,
                            &transport,
                            Clock_GetTimeMs,
                            mqttmngEventCallback,
                            &networkBuffer );

    if( mqttStatus != MQTTSuccess )
    {
        returnStatus = EXIT_FAILURE;
        LogError( ( "MQTT_Init failed: Status = %s.", MQTT_Status_strerror( mqttStatus ) ) );
    }
    else
    {
        mqttStatus = MQTT_InitStatefulQoS( pMqttContext,
                                           pOutgoingPublishRecords,
                                           OUTGOING_PUBLISH_RECORD_LEN,
                                           pIncomingPublishRecords,
                                           INCOMING_PUBLISH_RECORD_LEN );

        if( mqttStatus != MQTTSuccess )
        {
            returnStatus = EXIT_FAILURE;
            LogError( ( "MQTT_InitStatefulQoS failed: Status = %s.", MQTT_Status_strerror( mqttStatus ) ) );
        }
        else
        {
            /* Establish MQTT session by sending a CONNECT packet. */

            /* Start with a clean session i.e. direct the MQTT broker to discard any
             * previous session data. Also, establishing a connection with clean session
             * will ensure that the broker does not store any data when this client
             * gets disconnected. */
            connectInfo.cleanSession = true;

            /* The client identifier is used to uniquely identify this MQTT client to
             * the MQTT broker. In a production device the identifier can be something
             * unique, such as a device serial number. */
            connectInfo.pClientIdentifier = CLIENT_IDENTIFIER;
            connectInfo.clientIdentifierLength = CLIENT_IDENTIFIER_LENGTH;

            /* The maximum time interval in seconds which is allowed to elapse
             * between two Control Packets.
             * It is the responsibility of the Client to ensure that the interval between
             * Control Packets being sent does not exceed the this Keep Alive value. In the
             * absence of sending any other Control Packets, the Client MUST send a
             * PINGREQ Packet. */
            connectInfo.keepAliveSeconds = MQTT_KEEP_ALIVE_INTERVAL_SECONDS;

            /* Username and password for authentication. Not used in this demo. */
            connectInfo.pUserName = NULL;
            connectInfo.userNameLength = 0U;
            connectInfo.pPassword = NULL;
            connectInfo.passwordLength = 0U;

            /* Send MQTT CONNECT packet to broker. */
            mqttStatus = MQTT_Connect( pMqttContext, &connectInfo, NULL, CONNACK_RECV_TIMEOUT_MS, &sessionPresent );

            if( mqttStatus != MQTTSuccess )
            {
                returnStatus = EXIT_FAILURE;
                LogError( ( "Connection with MQTT broker failed with status %s.",
                            MQTT_Status_strerror( mqttStatus ) ) );
            }
            else
            {
                LogInfo( ( "MQTT connection successfully established with broker.\n\n" ) );
            }
        }
    }

    return returnStatus;
}
//-----------------------------------------------------------------------------
static int mqttmngSubscribeToTopic( MQTTContext_t * pMqttContext,
                                    const char * pTopicFilter,
                                    uint16_t topicFilterLength,
                                    uint16_t qos )
{
    int returnStatus = EXIT_SUCCESS;
    MQTTStatus_t mqttStatus;
    MQTTSubscribeInfo_t pSubscriptionList[ 1 ];

    assert( pMqttContext != NULL );

    /* Start with everything at 0. */
    ( void ) memset( ( void * ) pSubscriptionList, 0x00, sizeof( pSubscriptionList ) );

    pSubscriptionList[ 0 ].qos = qos;
    pSubscriptionList[ 0 ].pTopicFilter = pTopicFilter;
    pSubscriptionList[ 0 ].topicFilterLength = topicFilterLength;

    /* Generate packet identifier for the SUBSCRIBE packet. */
    lastSubscribePacketIdentifier = MQTT_GetPacketId( pMqttContext );

    /* Send SUBSCRIBE packet. */
    mqttStatus = MQTT_Subscribe( pMqttContext,
                                 pSubscriptionList,
                                 sizeof( pSubscriptionList ) / sizeof( MQTTSubscribeInfo_t ),
                                 lastSubscribePacketIdentifier );

    if( mqttStatus != MQTTSuccess )
    {
        LogError( ( "Failed to send SUBSCRIBE packet to broker with error = %u.",
                    mqttStatus ) );
        returnStatus = EXIT_FAILURE;
    }
    else
    {
        LogInfo( ( "SUBSCRIBE sent for topic %.*s to broker.\n\n",
                   topicFilterLength,
                   pTopicFilter ) );

        /* Wait for acknowledgement packet (SUBACK) from the broker in response to the
         * subscribe request. */
        returnStatus = mqttmngWaitForPacketAck( pMqttContext,
                                                lastSubscribePacketIdentifier,
                                                MQTT_PROCESS_LOOP_TIMEOUT_MS );
    }

    return returnStatus;
}
//-----------------------------------------------------------------------------
static int mqttmngWaitForPacketAck( MQTTContext_t * pMqttContext,
                                    uint16_t usPacketIdentifier,
                                    uint32_t ulTimeout )
{
    uint32_t ulMqttProcessLoopEntryTime;
    uint32_t ulMqttProcessLoopTimeoutTime;
    uint32_t ulCurrentTime;

    MQTTStatus_t eMqttStatus = MQTTSuccess;
    int returnStatus = EXIT_FAILURE;

    /* Reset the ACK packet identifier being received. */
    globalAckPacketIdentifier = 0U;

    ulCurrentTime = pMqttContext->getTime();
    ulMqttProcessLoopEntryTime = ulCurrentTime;
    ulMqttProcessLoopTimeoutTime = ulCurrentTime + ulTimeout;

    /* Call MQTT_ProcessLoop multiple times until the expected packet ACK
     * is received, a timeout happens, or MQTT_ProcessLoop fails. */
    while( ( globalAckPacketIdentifier != usPacketIdentifier ) &&
           ( ulCurrentTime < ulMqttProcessLoopTimeoutTime ) &&
           ( eMqttStatus == MQTTSuccess || eMqttStatus == MQTTNeedMoreBytes ) )
    {
        /* Event callback will set #globalAckPacketIdentifier when receiving
         * appropriate packet. */
        eMqttStatus = MQTT_ProcessLoop( pMqttContext );
        ulCurrentTime = pMqttContext->getTime();
    }

    if( ( ( eMqttStatus != MQTTSuccess ) && ( eMqttStatus != MQTTNeedMoreBytes ) ) ||
        ( globalAckPacketIdentifier != usPacketIdentifier ) )
    {
        LogError( ( "MQTT_ProcessLoop failed to receive ACK packet: Expected ACK Packet ID=%02X, LoopDuration=%u, Status=%s",
                    usPacketIdentifier,
                    ( ulCurrentTime - ulMqttProcessLoopEntryTime ),
                    MQTT_Status_strerror( eMqttStatus ) ) );
    }
    else
    {
        returnStatus = EXIT_SUCCESS;
    }

    return returnStatus;
}
//-----------------------------------------------------------------------------
//=============================================================================
