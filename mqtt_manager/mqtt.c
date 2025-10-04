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
#include "mqtt.h"

#include "mqttDefs.h"
#include "mqtt_subscription_manager.h"
#include "mqttLoggingConfig.h"

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
static uint8_t buffer[ MQTT_CONFIG_NETWORK_BUF_SIZE ];

/**
 * @brief Array to track the outgoing publish records for outgoing publishes
 * with QoS > 0.
 *
 * This is passed into #MQTT_InitStatefulQoS to allow for QoS > 0.
 *
 */
static MQTTPubAckInfo_t pOutgoingPublishRecords[ MQTT_CONFIG_OUTGOING_PUBLISH_RECORD_LEN ];

/**
 * @brief Array to track the incoming publish records for incoming publishes
 * with QoS > 0.
 *
 * This is passed into #MQTT_InitStatefulQoS to allow for QoS > 0.
 *
 */
static MQTTPubAckInfo_t pIncomingPublishRecords[ MQTT_CONFIG_INCOMING_PUBLISH_RECORD_LEN ];

/*-----------------------------------------------------------*/

/*-----------------------------------------------------------*/

/* Each compilation unit must define the NetworkContext struct. */
struct NetworkContext
{
    PlaintextParams_t * pParams;
};

typedef struct{

    const char *clientId;

    const char *subsTopics[MQTT_CONFIG_MAX_SUBS];
    uint32_t nSubs;

    MQTTContext_t mqttContext;
    NetworkContext_t networkContext;
    PlaintextParams_t plaintextParams;

    mqttLock_t lock;
    mqttUnlock_t unlock;

    int32_t packetsAwaitingAck[MQTT_CONFIG_OUTGOING_PUBLISH_RECORD_LEN];

    int initDone;

    MQTTPublishInfo_t *lastWillInfo;

}mqtt_t;

//=============================================================================

//=============================================================================
/*-------------------------------- Prototypes -------------------------------*/
//=============================================================================
static int mqttResetSession(void);
static int mqttSocketConnect(void);
static int mqttEstablishMqttSession(void);
static int mqttResubscribe(void);

static void mqttEventCallback(
    MQTTContext_t * pMqttContext,
    MQTTPacketInfo_t * pPacketInfo,
    MQTTDeserializedInfo_t * pDeserializedInfo
);


static int mqttSubscribeToTopic(
    MQTTContext_t * pMqttContext,
    const char * pTopicFilter,
    uint16_t topicFilterLength,
    uint16_t qos
);

static int32_t mqttPublishBare(const char *topic, mqttPayload_t *payload);

static int32_t mqttLock(uint32_t timeout);
static void mqttUnlock(void);

static void mqttAddPacketIdToList(uint16_t id);
static void mqttRemovePacketIdToList(uint16_t id);
//=============================================================================

//=============================================================================
/*--------------------------------- Globals ---------------------------------*/
//=============================================================================
static mqtt_t mqtt = {
    .nSubs = 0,
    .mqttContext = {0}, .networkContext = {0}, .plaintextParams = {0},
    .lock = 0, .unlock = 0,
    .initDone = 0,
    .lastWillInfo = NULL
};
//=============================================================================

//=============================================================================
/*-------------------------------- Functions --------------------------------*/
//=============================================================================
//-----------------------------------------------------------------------------
int32_t mqttInit(
    const char *clientId, MQTTPublishInfo_t *lastWillInfo,
    mqttLock_t lock, mqttUnlock_t unlock
){

    int status; 

    mqtt.clientId = clientId;
    mqtt.lastWillInfo = lastWillInfo;

    mqtt.lock = lock;
    mqtt.unlock = unlock;

    /* Initializes network context and connects to broker (server) */
    status = mqttSocketConnect();
    if( status != 0 ) return status;

    /* Establish MQTT session on top of TCP connection. */
    status = mqttEstablishMqttSession();
    if( status != 0 ) return status;

    mqtt.initDone = 1;

    return 0;
}
//-----------------------------------------------------------------------------
void mqttRun(uint32_t forever){

    int32_t status;
    
    do{
        if( mqttLock(MQTT_CONFIG_LOCK_TIMEOUT_MS) != 0 ) continue;

        status = MQTT_ProcessLoop( &mqtt.mqttContext );

        if( (status != MQTTSuccess) && (status != MQTTNeedMoreBytes) ){
            LogWarn( ("Seems like there was a problem with the connection. Will try to reset it..."));
            mqttResetSession();
        }

        mqttUnlock();

        if( forever ) Clock_SleepMs(MQTT_CONFIG_PROC_INTERVAL_MS);
    }while(forever);
}
//-----------------------------------------------------------------------------
int32_t mqttPublish(const char *topic, mqttPayload_t *payload){

    int status;

    if( mqtt.initDone == 0 ) return -1;

    LogDebug( ("Publishing to %s...", topic) );

    if( mqttLock(MQTT_CONFIG_LOCK_TIMEOUT_MS) != 0 ){
        LogError( ("Failed to obtain lock when trying to publish to %s.", topic) );
        return -1;
    }

    status = mqttPublishBare(topic, payload);

    mqttUnlock();

    LogDebug( ("Publish status: %d.", status) );

    return status;
}
//-----------------------------------------------------------------------------
int32_t mqttSubscribe(const char *topic, mqttSubscrCb_t callback){

    int status;
    uint32_t topiclen;
    if( mqtt.initDone == 0 ) return -1;

    if( mqttLock(MQTT_CONFIG_LOCK_TIMEOUT_MS) != 0 ){
        LogError( ("Failed to obtain lock when trying to subscribe to %s.", topic) );
        return -1;
    }

    if( mqtt.nSubs >= MQTT_CONFIG_MAX_SUBS ){
        mqttUnlock();
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
        mqttUnlock();
        return -1;
    }

    LogDebug(("Subscribing to %s...", topic));
    status = mqttSubscribeToTopic(&mqtt.mqttContext, topic, topiclen, 0);
    LogDebug(("Subscription status: %d.", status));

    if( status < 0 ){
        LogError( ("Failed to subscribe to topic %s.", topic) );
        /* Remove the registered callback for the temperature topic filter as
        * the subscription operation for the topic filter did not succeed. */
        ( void ) SubscriptionManager_RemoveCallback( topic, topiclen );
        mqttUnlock();
        return -1;        
    }

    mqtt.subsTopics[mqtt.nSubs] = topic;
    mqtt.nSubs++;
    
    mqttUnlock();

    return status;
}
//-----------------------------------------------------------------------------
int mqttInitDone(void){

    if( mqtt.initDone == 0 ) return -1;

    return 0;
}
//-----------------------------------------------------------------------------
//=============================================================================


//=============================================================================
/*---------------------------- Static functions -----------------------------*/
//=============================================================================
//-----------------------------------------------------------------------------
static int mqttSocketConnect(void){

    mqtt.networkContext.pParams = &mqtt.plaintextParams;

    NetworkContext_t * pNetworkContext = &mqtt.networkContext;

    SocketStatus_t socketStatus = SOCKETS_SUCCESS;
    ServerInfo_t serverInfo;

    /* Initialize information to connect to the MQTT broker. */
    serverInfo.pHostName = MQTT_CONFIG_HOST;
    serverInfo.hostNameLength = MQTT_CONFIG_HOST_LEN;
    serverInfo.port = MQTT_CONFIG_PORT;

    LogInfo( ( "Creating a TCP connection to %.*s:%d.",
                MQTT_CONFIG_HOST_LEN,
                MQTT_CONFIG_HOST,
                MQTT_CONFIG_PORT ) );

    socketStatus = Plaintext_Connect( pNetworkContext,
                                        &serverInfo,
                                        MQTT_CONFIG_TRANSPORT_SEND_RECV_TIMEOUT_MS,
                                        MQTT_CONFIG_TRANSPORT_SEND_RECV_TIMEOUT_MS );

    if( socketStatus != SOCKETS_SUCCESS ){
        LogError( ( "Failed to create connection to %.*s:%d. Error code: %d",
                    MQTT_CONFIG_HOST_LEN,
                    MQTT_CONFIG_HOST,
                    MQTT_CONFIG_PORT,
                    socketStatus ) );
        return -1;
    }

    LogInfo( ( "Connection established." ) );

    return 0;
}
//-----------------------------------------------------------------------------
static int mqttEstablishMqttSession(void){

    NetworkContext_t * pNetworkContext = &mqtt.networkContext;
    MQTTContext_t * pMqttContext = &mqtt.mqttContext;

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
    networkBuffer.size = MQTT_CONFIG_NETWORK_BUF_SIZE;

    /* Initialize MQTT library. */
    mqttStatus = MQTT_Init( pMqttContext,
                            &transport,
                            Clock_GetTimeMs,
                            mqttEventCallback,
                            &networkBuffer );

    if( mqttStatus != MQTTSuccess ){
        LogError( ( "MQTT_Init failed: Status = %s.", MQTT_Status_strerror( mqttStatus ) ) );
        return -1;
    }

    mqttStatus = MQTT_InitStatefulQoS( pMqttContext,
                                       pOutgoingPublishRecords,
                                       MQTT_CONFIG_OUTGOING_PUBLISH_RECORD_LEN,
                                       pIncomingPublishRecords,
                                       MQTT_CONFIG_INCOMING_PUBLISH_RECORD_LEN );

    if( mqttStatus != MQTTSuccess ){
        LogError( ( "MQTT_InitStatefulQoS failed: Status = %s.", MQTT_Status_strerror( mqttStatus ) ) );
        return -1;
    }

    connectInfo.cleanSession = true;

    connectInfo.pClientIdentifier = mqtt.clientId;
    connectInfo.clientIdentifierLength = strlen(mqtt.clientId);

    connectInfo.keepAliveSeconds = MQTT_CONFIG_KEEP_ALIVE_INTERVAL_SECONDS;

    connectInfo.pUserName = NULL;
    connectInfo.userNameLength = 0U;
    connectInfo.pPassword = NULL;
    connectInfo.passwordLength = 0U;

    LogInfo( ( "Creating an MQTT connection to %.*s.",
                MQTT_CONFIG_HOST_LEN,
                MQTT_CONFIG_HOST ) );

    /* Establish MQTT session by sending a CONNECT packet. */
    mqttStatus = MQTT_Connect(
        pMqttContext, &connectInfo, mqtt.lastWillInfo,
        MQTT_CONFIG_CONNACK_RECV_TIMEOUT_MS, &sessionPresent
    );

    if( mqttStatus != MQTTSuccess ){
        LogError( ( "Connection with MQTT broker failed with status %s.",
                    MQTT_Status_strerror( mqttStatus ) ) );

        /* Close the TCP connection.  */
        ( void ) Plaintext_Disconnect( &mqtt.networkContext );
        return -1;
    }

    LogInfo( ( "MQTT connection successfully established with broker." ) );

    return 0;
}
//-----------------------------------------------------------------------------
static int mqttResubscribe(void){

    uint32_t k;
    int status;

    for(k = 0; k < mqtt.nSubs; k++){
        LogDebug( ("Resubscribing to %s...", mqtt.subsTopics[k]) );
        status = mqttSubscribeToTopic(
            &mqtt.mqttContext,
            mqtt.subsTopics[k],
            strlen(mqtt.subsTopics[k]),
            0);
        LogDebug(("Subscription status: %d.", status));

        if( status < 0 ) return -1;
    }

    return 0;
}
//-----------------------------------------------------------------------------
static int32_t mqttPublishBare(const char *topic, mqttPayload_t *payload){

    MQTTStatus_t mqttStatus = MQTTSuccess;
    MQTTPublishInfo_t publishInfo;
    uint16_t pubPacketId = MQTT_PACKET_ID_INVALID;
    MQTTContext_t * pMqttContext = &mqtt.mqttContext;

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
static void mqttEventCallback( MQTTContext_t * pMqttContext,
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
            mqttRemovePacketIdToList( pDeserializedInfo->packetIdentifier );
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
static int mqttSubscribeToTopic( MQTTContext_t * pMqttContext,
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
    mqttAddPacketIdToList(id);

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
static int32_t mqttLock(uint32_t timeout){

    if( mqtt.lock == 0 ) return 0;

    if( mqtt.lock(timeout) != 0 ){
        LogError( ("Failed to take lock") );
        return -1;
    }

    return 0;
}
//-----------------------------------------------------------------------------
static void mqttUnlock(void){

    if( mqtt.unlock != 0 ) mqtt.unlock();
}
//-----------------------------------------------------------------------------
static void mqttAddPacketIdToList(uint16_t id){
    
    uint32_t k;

    for(k = 0; k < MQTT_CONFIG_OUTGOING_PUBLISH_RECORD_LEN; k++){
        if( mqtt.packetsAwaitingAck[k] == -1){
            mqtt.packetsAwaitingAck[k] = (int32_t) id;
            break;
        }
    }
}
//-----------------------------------------------------------------------------
static void mqttRemovePacketIdToList(uint16_t id){

    uint32_t k;

    for(k = 0; k < MQTT_CONFIG_OUTGOING_PUBLISH_RECORD_LEN; k++){
        if( mqtt.packetsAwaitingAck[k] == id){
            mqtt.packetsAwaitingAck[k] = -1;
            break;
        }
    }
}
//-----------------------------------------------------------------------------
static int mqttResetSession(void){

    while(1){

        Clock_SleepMs(MQTT_CONFIG_PROC_INTERVAL_MS);

        LogInfo( ("Running an iteration of the reset procedure") );

        MQTT_Disconnect( &mqtt.mqttContext );
        Plaintext_Disconnect( &mqtt.networkContext );

        if( mqttSocketConnect() != 0 ) continue;

        if( mqttEstablishMqttSession() != 0 ) continue;

        if( mqttResubscribe() != 0 ) continue;

        break;
    }

    LogInfo( ("Reset successful") );

    return 0;
}
//-----------------------------------------------------------------------------
//=============================================================================
