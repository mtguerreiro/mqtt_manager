#ifndef MQTT_DEFS_H_
#define MQTT_DEFS_H_

//=============================================================================
/*-------------------------------- Includes ---------------------------------*/
//=============================================================================
#include "stdint.h"
#include "stddef.h"

#include "mqttConfig.h"
#include "core_mqtt.h"
//============================================================================

//=============================================================================
/*------------------------------- Definitions -------------------------------*/
//=============================================================================
typedef void (* mqttSubscrCb_t )( MQTTContext_t * pContext, MQTTPublishInfo_t * pPublishInfo );

typedef int32_t (*mqttLock_t)(uint32_t timeout);
typedef void (*mqttUnlock_t)(void);

typedef struct
{
    uint8_t retain;
    uint8_t dup;
    const void * data;
    uint32_t size;
}mqttPayload_t;

#ifndef MQTT_CONFIG_HOST
#warning "Setting MQTT_CONFIG_HOST to localhost as not previous definition was found."
#define MQTT_CONFIG_HOST                            "localhost"
#endif

/**
 * @brief Length of MQTT server host name.
 */
#define MQTT_CONFIG_HOST_LEN                        ( ( uint16_t ) ( sizeof( MQTT_CONFIG_HOST ) - 1U ) )

/**
 * Provide default values for undefined configuration settings.
 */
#ifndef MQTT_CONFIG_PORT
#define MQTT_CONFIG_PORT                            1883U
#endif

#ifndef MQTT_CONFIG_NETWORK_BUF_SIZE
#define MQTT_CONFIG_NETWORK_BUF_SIZE                1024U
#endif

/**
 * @brief Timeout for receiving CONNACK packet in milli seconds.
 */
#ifndef MQTT_CONFIG_CONNACK_RECV_TIMEOUT_MS
#define MQTT_CONFIG_CONNACK_RECV_TIMEOUT_MS         1000U
#endif

/**
 * @brief The maximum time interval in seconds which is allowed to elapse
 * between two Control Packets.
 */
#ifndef MQTT_CONFIG_KEEP_ALIVE_INTERVAL_SECONDS
#define MQTT_CONFIG_KEEP_ALIVE_INTERVAL_SECONDS     60U
#endif

/**
 * @brief Transport timeout in milliseconds for transport send and receive.
 */
#ifndef MQTT_CONFIG_TRANSPORT_SEND_RECV_TIMEOUT_MS
#define MQTT_CONFIG_TRANSPORT_SEND_RECV_TIMEOUT_MS  1000U
#endif

/**
 * @brief The length of the outgoing publish records array used by the coreMQTT
 * library to track QoS > 0 packet ACKS for outgoing publishes.
 */
#ifndef MQTT_CONFIG_OUTGOING_PUBLISH_RECORD_LEN
#define MQTT_CONFIG_OUTGOING_PUBLISH_RECORD_LEN     10U
#endif

/**
 * @brief The length of the incoming publish records array used by the coreMQTT
 * library to track QoS > 0 packet ACKS for incoming publishes.
 */
#ifndef MQTT_CONFIG_INCOMING_PUBLISH_RECORD_LEN
#define MQTT_CONFIG_INCOMING_PUBLISH_RECORD_LEN     10U
#endif

#ifndef MQTT_CONFIG_LOCK_TIMEOUT_MS
#define MQTT_CONFIG_LOCK_TIMEOUT_MS                 500U
#endif

#ifndef MQTT_CONFIG_MAX_SUBS
#define MQTT_CONFIG_MAX_SUBS                        10U
#endif

#ifndef MQTT_CONFIG_PROC_INTERVAL_MS
#define MQTT_CONFIG_PROC_INTERVAL_MS                50U
#endif

#ifndef MQTT_MNG_CONFIG_SUBS_COMP_BUF_SIZE
#define MQTT_MNG_CONFIG_SUBS_COMP_BUF_SIZE          64U
#endif

#ifndef MQTT_MNG_CONFIG_PUB_COMP_BUF_SIZE
#define MQTT_MNG_CONFIG_PUB_COMP_BUF_SIZE           64U
#endif
//=============================================================================

#endif /* MQTT_DEFS_H_ */
