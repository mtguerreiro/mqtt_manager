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
#include "mqtt_subscription_manager.h"
#include "mqttmngLoggingConfig.h"

#include "stdlib.h"
#include "stdio.h"
#include "assert.h"

/* MQTT API header. */
#include "core_mqtt.h"

/* Plaintext sockets transport implementation. */
#include "plaintext_transport.h"
#include "transport_interface.h"

/* Clock for timer. */
#include "clock.h"
//============================================================================

//=============================================================================
/*------------------------------- Definitions -------------------------------*/
//=============================================================================
#ifndef MQTT_MNG_CONFIG_HOST
    #error "Please define MQTT_MNG_CONFIG_HOST to define the host in your config file."
#endif
#ifndef MQTT_MNG_CONFIG_DEV_ID
    #error "Please define MQTT_MNG_CONFIG_DEV_ID to define a unique for your device."
#endif

#define MQTT_MNG_CONFIG_COMPONENTS_TOPIC    MQTT_MNG_CONFIG_DEV_ID "/components"

/**
 * @brief Length of MQTT server host name.
 */
#define MQTT_MNG_CONFIG_HOST_LEN      ( ( uint16_t ) ( sizeof( MQTT_MNG_CONFIG_HOST ) - 1 ) )

/**
 * Provide default values for undefined configuration settings.
 */
#ifndef MQTT_MNG_CONFIG_PORT
    #define MQTT_MNG_CONFIG_PORT    ( 1883 )
#endif

#ifndef NETWORK_BUFFER_SIZE
    #define NETWORK_BUFFER_SIZE    ( 1024U )
#endif

/**
 * @brief Length of client identifier.
 */
#define MQTT_MNG_CONFIG_DEV_ID_LEN                 ( ( uint16_t ) ( sizeof( MQTT_MNG_CONFIG_DEV_ID ) - 1 ) )

/**
 * @brief Timeout for receiving CONNACK packet in milli seconds.
 */
#define CONNACK_RECV_TIMEOUT_MS                 ( 1000U )

/**
 * @brief The maximum time interval in seconds which is allowed to elapse
 * between two Control Packets.
 */
#ifndef MQTT_MNG_KEEP_ALIVE_INTERVAL_SECONDS
    #define MQTT_MNG_KEEP_ALIVE_INTERVAL_SECONDS        ( 60U )
#endif

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

#define MQTT_MNG_LOCK_TIMEOUT_MS            2000
#define MQTT_MNG_SUBS_COMP_BUF_SIZE         64
#define MQTT_MNG_SUBS_COMP_MAX_TOPIC_SIZE   64

#ifndef MQTT_MNG_PROC_INTERVAL_MS
    #define MQTT_MNG_PROC_INTERVAL_MS              ( 50U )
#endif
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
 * @brief The network buffer must remain valid for the lifetime of the MQTT context.
 */
static uint8_t buffer[ NETWORK_BUFFER_SIZE ];

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

typedef struct{

    const char *subsTopics[MQTT_MNG_CONFIG_MAX_SUBS];
    uint32_t nSubs;

    char components[MQTT_MNG_SUBS_COMP_BUF_SIZE];
    uint32_t componentsLen;

    MQTTContext_t mqttContext;
    NetworkContext_t networkContext;
    PlaintextParams_t plaintextParams;

    mqttmngLock_t lock;
    mqttmngUnlock_t unlock;

    int32_t packetsAwaitingAck[OUTGOING_PUBLISH_RECORD_LEN];

    int initDone;

}mqttmng_t;

//=============================================================================

//=============================================================================
/*-------------------------------- Prototypes -------------------------------*/
//=============================================================================
static int mqttmngResetSession(void);
static int mqttmngSocketConnect(void);
static int mqttmngEstablishMqttSession(void);
static int mqttmngResubscribe(void);

static void mqttmngEventCallback( MQTTContext_t * pMqttContext,
                                  MQTTPacketInfo_t * pPacketInfo,
                                  MQTTDeserializedInfo_t * pDeserializedInfo );


static int mqttmngSubscribeToTopic( MQTTContext_t * pMqttContext, 
                                    const char * pTopicFilter, 
                                    uint16_t topicFilterLength,
                                    uint16_t qos );

static int32_t mqttmngPublishBare(const char *topic, mqttmngPayload_t *payload);

static int32_t mqttmngLock(uint32_t timeout);
static void mqttmngUnlock(void);

static void mqttmngAddPacketIdToList(uint16_t id);
static void mqttmngRemovePacketIdToList(uint16_t id);
//=============================================================================

//=============================================================================
/*--------------------------------- Globals ---------------------------------*/
//=============================================================================
static mqttmng_t mqttmng = {
    .components = {0},
    .componentsLen = 0,
    .nSubs = 0,
    .mqttContext = {0}, .networkContext = {0}, .plaintextParams = {0},
    .lock = 0, .unlock = 0,
    .initDone = 0,
    };
//=============================================================================

//=============================================================================
/*-------------------------------- Functions --------------------------------*/
//=============================================================================
//-----------------------------------------------------------------------------
int32_t mqttmngInit(mqttmngLock_t lock, mqttmngUnlock_t unlock){

    int status; 

    mqttmng.lock = lock;
    mqttmng.unlock = unlock;

    /* Initializes network context and connects to broker (server) */
    status = mqttmngSocketConnect();
    if( status != 0 ) return status;

    /* Establish MQTT session on top of TCP connection. */
    status = mqttmngEstablishMqttSession();
    if( status != 0 ) return status;

    mqttmng.initDone = 1;

    return 0;
}
//-----------------------------------------------------------------------------
void mqttmngRun(void){

    int32_t status;
    
    while( 1 ){

        if( mqttmngLock(MQTT_MNG_LOCK_TIMEOUT_MS) != 0 ) continue;
        
        status = MQTT_ProcessLoop( &mqttmng.mqttContext );

        if( (status != MQTTSuccess) && (status != MQTTNeedMoreBytes) ){
            LogWarn( ("Seems like there was a problem with the connection. Will try to reset it..."));
            mqttmngResetSession();
        }

        mqttmngUnlock();

        Clock_SleepMs(MQTT_MNG_PROC_INTERVAL_MS);
    }
}
//-----------------------------------------------------------------------------
int32_t mqttmngPublishComponent(const char *name, const char *type, const char *flags){

    int32_t status;
    mqttmngPayload_t payload;
    int clen;
    int blen;
    
    char buf[MQTT_MNG_SUBS_COMP_BUF_SIZE];
    
    if( mqttmngLock(MQTT_MNG_LOCK_TIMEOUT_MS) != 0 ){
        LogError( ("Failed to obtain lock when publishing component.") );
        return -1;
    }

    if( flags != NULL )
        clen = snprintf(buf, sizeof(buf), "%s:%s-%s;", name, type, flags);
    else
        clen = snprintf(buf, sizeof(buf), "%s:%s;", name, type);

    blen = (int)( sizeof(mqttmng.components) - mqttmng.componentsLen );
    if( clen > blen ){
        LogError( ("Component %s not added because is too large to fit in components buffer.", name) );
        mqttmngUnlock();
        return -1;
    }

    memcpy(&mqttmng.components[mqttmng.componentsLen], buf, clen);
    mqttmng.componentsLen += clen;

    payload.data = mqttmng.components;
    payload.size = mqttmng.componentsLen;
    payload.retain = 1;
    payload.dup = 0;

    LogDebug( ("Publishing components %s...", mqttmng.components) );
    status = mqttmngPublishBare(MQTT_MNG_CONFIG_COMPONENTS_TOPIC, &payload);
    LogDebug( ("Publish status %d ", (int)status) );

    mqttmngUnlock();

    return status;
}
//-----------------------------------------------------------------------------
int32_t mqttmngPublish(const char *topic, mqttmngPayload_t *payload){

    int status;

    if( mqttmng.initDone == 0 ) return -1;

    LogDebug( ("Publishing to %s...", topic) );

    if( mqttmngLock(MQTT_MNG_LOCK_TIMEOUT_MS) != 0 ){
        LogError( ("Failed to obtain lock when trying to publish to %s.", topic) );
        return -1;
    }

    status = mqttmngPublishBare(topic, payload);

    mqttmngUnlock();

    LogDebug( ("Publish status: %d.", status) );

    return status;
}
//-----------------------------------------------------------------------------
int32_t mqttmngSubscribe(const char *topic, mqttmngSubscrCb_t callback){

    int status;
    uint32_t topiclen;
    if( mqttmng.initDone == 0 ) return -1;

    if( mqttmngLock(MQTT_MNG_LOCK_TIMEOUT_MS) != 0 ){
        LogError( ("Failed to obtain lock when trying to subscribe to %s.", topic) );
        return -1;
    }

    if( mqttmng.nSubs >= MQTT_MNG_CONFIG_MAX_SUBS ){
        mqttmngUnlock();
        return -1;
    }

    topiclen = strlen(topic);

    SubscriptionManagerStatus_t managerStatus = ( SubscriptionManagerStatus_t ) 0u;

    /* Register the topic filter and its callback with subscription manager.
    * On an incoming PUBLISH message whose topic name that matches the topic filter
    * being registered, its callback will be invoked. */
    managerStatus = SubscriptionManager_RegisterCallback( topic, topiclen, callback );

    if( managerStatus != SUBSCRIPTION_MANAGER_SUCCESS ){
        LogError( ("Failed to register callback for topic %s.", topic) );
        mqttmngUnlock();
        return -1;
    }

    LogDebug(("Subscribing to %s...", topic));
    status = mqttmngSubscribeToTopic(&mqttmng.mqttContext, topic, topiclen, 0);
    LogDebug(("Subscription status: %d.", status));

    if( status < 0 ){
        LogError( ("Failed to subscribe to topic %s.", topic) );
        /* Remove the registered callback for the temperature topic filter as
        * the subscription operation for the topic filter did not succeed. */
        ( void ) SubscriptionManager_RemoveCallback( topic, topiclen );
        mqttmngUnlock();
        return -1;        
    }

    mqttmng.subsTopics[mqttmng.nSubs] = topic;
    mqttmng.nSubs++;
    
    mqttmngUnlock();

    return status;
}
//-----------------------------------------------------------------------------
int mqttmngInitDone(void){

    if( mqttmng.initDone == 0 ) return -1;

    return 0;
}
//-----------------------------------------------------------------------------
//=============================================================================


//=============================================================================
/*---------------------------- Static functions -----------------------------*/
//=============================================================================
//-----------------------------------------------------------------------------
static int mqttmngSocketConnect(void){

    mqttmng.networkContext.pParams = &mqttmng.plaintextParams;

    NetworkContext_t * pNetworkContext = &mqttmng.networkContext;

    SocketStatus_t socketStatus = SOCKETS_SUCCESS;
    ServerInfo_t serverInfo;

    /* Initialize information to connect to the MQTT broker. */
    serverInfo.pHostName = MQTT_MNG_CONFIG_HOST;
    serverInfo.hostNameLength = MQTT_MNG_CONFIG_HOST_LEN;
    serverInfo.port = MQTT_MNG_CONFIG_PORT;

    LogInfo( ( "Creating a TCP connection to %.*s:%d.",
                MQTT_MNG_CONFIG_HOST_LEN,
                MQTT_MNG_CONFIG_HOST,
                MQTT_MNG_CONFIG_PORT ) );

    socketStatus = Plaintext_Connect( pNetworkContext,
                                        &serverInfo,
                                        TRANSPORT_SEND_RECV_TIMEOUT_MS,
                                        TRANSPORT_SEND_RECV_TIMEOUT_MS );

    if( socketStatus != SOCKETS_SUCCESS ){
        LogError( ( "Failed to create connection to %.*s:%d. Error code: %d",
                    MQTT_MNG_CONFIG_HOST_LEN,
                    MQTT_MNG_CONFIG_HOST,
                    MQTT_MNG_CONFIG_PORT,
                    socketStatus ) );
        return -1;
    }

    LogInfo( ( "Connection established." ) );

    return 0;
}
//-----------------------------------------------------------------------------
static int mqttmngEstablishMqttSession(void){

    NetworkContext_t * pNetworkContext = &mqttmng.networkContext;
    MQTTContext_t * pMqttContext = &mqttmng.mqttContext;

    MQTTStatus_t mqttStatus;
    MQTTConnectInfo_t connectInfo;
    bool sessionPresent;
    MQTTFixedBuffer_t networkBuffer;
    TransportInterface_t transport = { NULL };

    transport.pNetworkContext = pNetworkContext;
    transport.send = Plaintext_Send;
    transport.recv = Plaintext_Recv;
    transport.writev = NULL;

    networkBuffer.pBuffer = buffer;
    networkBuffer.size = NETWORK_BUFFER_SIZE;

    /* Initialize MQTT library. */
    mqttStatus = MQTT_Init( pMqttContext,
                            &transport,
                            Clock_GetTimeMs,
                            mqttmngEventCallback,
                            &networkBuffer );

    if( mqttStatus != MQTTSuccess ){
        LogError( ( "MQTT_Init failed: Status = %s.", MQTT_Status_strerror( mqttStatus ) ) );
        return -1;
    }

    mqttStatus = MQTT_InitStatefulQoS( pMqttContext,
                                       pOutgoingPublishRecords,
                                       OUTGOING_PUBLISH_RECORD_LEN,
                                       pIncomingPublishRecords,
                                       INCOMING_PUBLISH_RECORD_LEN );

    if( mqttStatus != MQTTSuccess ){
        LogError( ( "MQTT_InitStatefulQoS failed: Status = %s.", MQTT_Status_strerror( mqttStatus ) ) );
        return -1;
    }

    connectInfo.cleanSession = true;

    connectInfo.pClientIdentifier = MQTT_MNG_CONFIG_DEV_ID;
    connectInfo.clientIdentifierLength = MQTT_MNG_CONFIG_DEV_ID_LEN;

    connectInfo.keepAliveSeconds = MQTT_MNG_KEEP_ALIVE_INTERVAL_SECONDS;

    connectInfo.pUserName = NULL;
    connectInfo.userNameLength = 0U;
    connectInfo.pPassword = NULL;
    connectInfo.passwordLength = 0U;

    LogInfo( ( "Creating an MQTT connection to %.*s.",
                MQTT_MNG_CONFIG_HOST_LEN,
                MQTT_MNG_CONFIG_HOST ) );

    /* Establish MQTT session by sending a CONNECT packet. */
    mqttStatus = MQTT_Connect( pMqttContext, &connectInfo, NULL, CONNACK_RECV_TIMEOUT_MS, &sessionPresent );

    if( mqttStatus != MQTTSuccess ){
        LogError( ( "Connection with MQTT broker failed with status %s.",
                    MQTT_Status_strerror( mqttStatus ) ) );

        /* Close the TCP connection.  */
        ( void ) Plaintext_Disconnect( &mqttmng.networkContext );
        return -1;
    }

    LogInfo( ( "MQTT connection successfully established with broker." ) );

    return 0;
}
//-----------------------------------------------------------------------------
static int mqttmngResubscribe(void){

    uint32_t k;
    int status;

    for(k = 0; k < mqttmng.nSubs; k++){
        LogDebug( ("Resubscribing to %s...", mqttmng.subsTopics[k]) );
        status = mqttmngSubscribeToTopic(
            &mqttmng.mqttContext,
            mqttmng.subsTopics[k],
            strlen(mqttmng.subsTopics[k]),
            0);
        LogDebug(("Subscription status: %d.", status));

        if( status < 0 ) return -1;
    }

    return 0;
}
//-----------------------------------------------------------------------------
static int32_t mqttmngPublishBare(const char *topic, mqttmngPayload_t *payload){

    MQTTStatus_t mqttStatus = MQTTSuccess;
    MQTTPublishInfo_t publishInfo;
    uint16_t pubPacketId = MQTT_PACKET_ID_INVALID;
    MQTTContext_t * pMqttContext = &mqttmng.mqttContext;

    ( void ) memset( &publishInfo, 0x00, sizeof( MQTTPublishInfo_t ) );

    publishInfo.qos = 0;
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

    if( mqttStatus != MQTTSuccess ){
        LogError( ( "Failed to send PUBLISH packet to broker with error = %u.",
                    mqttStatus ) );
        return -1;
    }

    return 0;
}
//-----------------------------------------------------------------------------
static void mqttmngEventCallback( MQTTContext_t * pMqttContext,
                                  MQTTPacketInfo_t * pPacketInfo,
                                  MQTTDeserializedInfo_t * pDeserializedInfo ){

    LogDebug( ("Processing event for packet id: %d", pDeserializedInfo->packetIdentifier) );
    /* Handle incoming publish. The lower 4 bits of the publish packet
     * type is used for the dup, QoS, and retain flags. Hence masking
     * out the lower bits to check if the packet is publish. */
    if( ( pPacketInfo->type & 0xF0U ) == MQTT_PACKET_TYPE_PUBLISH ){
        /* Handle incoming publish. */
        SubscriptionManager_DispatchHandler( pMqttContext, pDeserializedInfo->pPublishInfo );
        return;
    }

    /* Handle other packets. */
    switch( pPacketInfo->type )
    {
        case MQTT_PACKET_TYPE_SUBACK:

            LogDebug( ("Received SUBACK for packet id %d", pDeserializedInfo->packetIdentifier) );
            mqttmngRemovePacketIdToList( pDeserializedInfo->packetIdentifier );
            break;

        case MQTT_PACKET_TYPE_UNSUBACK:
            LogDebug( ( "Received UNSUBACK." ) );
            break;

        case MQTT_PACKET_TYPE_PINGRESP:

            /* Nothing to be done from application as library handles
                * PINGRESP. */
            LogWarn( ( "PINGRESP should not be handled by the application "
                        "callback when using MQTT_ProcessLoop." ) );
            break;

        case MQTT_PACKET_TYPE_PUBACK:
            LogDebug( ( "PUBACK received for packet id %u.",
                        pDeserializedInfo->packetIdentifier ) );
            break;

        /* Any other packet type is invalid. */
        default:
            LogError( ( "Unknown packet type received:(%02x).",
                        pPacketInfo->type ) );
    }
}

//-----------------------------------------------------------------------------
static int mqttmngSubscribeToTopic( MQTTContext_t * pMqttContext,
                                    const char * pTopicFilter,
                                    uint16_t topicFilterLength,
                                    uint16_t qos ){
    uint16_t id;
    MQTTStatus_t mqttStatus;
    MQTTSubscribeInfo_t pSubscriptionList[ 1 ];

    /* Start with everything at 0. */
    ( void ) memset( ( void * ) pSubscriptionList, 0x00, sizeof( pSubscriptionList ) );

    pSubscriptionList[ 0 ].qos = qos;
    pSubscriptionList[ 0 ].pTopicFilter = pTopicFilter;
    pSubscriptionList[ 0 ].topicFilterLength = topicFilterLength;

    /* Generate packet identifier for the SUBSCRIBE packet. */
    id = MQTT_GetPacketId( pMqttContext );

    LogDebug( ("Adding packet id %d to list for topic %s.", id, pTopicFilter) );
    mqttmngAddPacketIdToList(id);

    /* Send SUBSCRIBE packet. */
    mqttStatus = MQTT_Subscribe( pMqttContext,
                                 pSubscriptionList,
                                 sizeof( pSubscriptionList ) / sizeof( MQTTSubscribeInfo_t ),
                                 id );

    if( mqttStatus != MQTTSuccess ){
        LogError( ( "Failed to send SUBSCRIBE packet to broker with error = %u.",
                    mqttStatus ) );
        return -1;
    }
 
    return id;
}
//-----------------------------------------------------------------------------
static int32_t mqttmngLock(uint32_t timeout){

    if( mqttmng.lock == 0 ) return 0;

    if( mqttmng.lock(timeout) != 0 ){
        LogError( ("Failed to take lock") );
        return -1;
    }

    return 0;
}
//-----------------------------------------------------------------------------
static void mqttmngUnlock(void){

    if( mqttmng.unlock != 0 ) mqttmng.unlock();
}
//-----------------------------------------------------------------------------
static void mqttmngAddPacketIdToList(uint16_t id){
    
    uint32_t k;

    for(k = 0; k < OUTGOING_PUBLISH_RECORD_LEN; k++){
        if( mqttmng.packetsAwaitingAck[k] == -1){
            mqttmng.packetsAwaitingAck[k] = (int32_t) id;
            break;
        }
    }
}
//-----------------------------------------------------------------------------
static void mqttmngRemovePacketIdToList(uint16_t id){

    uint32_t k;

    for(k = 0; k < OUTGOING_PUBLISH_RECORD_LEN; k++){
        if( mqttmng.packetsAwaitingAck[k] == id){
            mqttmng.packetsAwaitingAck[k] = -1;
            break;
        }
    }
}
//-----------------------------------------------------------------------------
static int mqttmngResetSession(void){

    while(1){

    Clock_SleepMs(MQTT_MNG_PROC_INTERVAL_MS);
    
    LogInfo( ("Running an iteration of the reset procedure") );

    MQTT_Disconnect( &mqttmng.mqttContext );
    Plaintext_Disconnect( &mqttmng.networkContext );
    
    if( mqttmngSocketConnect() != 0 ) continue;

    if( mqttmngEstablishMqttSession() != 0 ) continue;
    
    if( mqttmngResubscribe() != 0 ) continue;

    break;

    }

    LogInfo( ("Reset successful") );

    return 0;
}
//-----------------------------------------------------------------------------
//=============================================================================
