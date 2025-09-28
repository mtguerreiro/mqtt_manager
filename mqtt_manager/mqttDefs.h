#ifndef MQTT_DEFS_H_
#define MQTT_DEFS_H_

//=============================================================================
/*-------------------------------- Includes ---------------------------------*/
//=============================================================================
#include "stdint.h"
#include "stddef.h"

#include "mqttmngConfig.h"
//============================================================================

//=============================================================================
/*------------------------------- Definitions -------------------------------*/
//=============================================================================

#ifndef MQTT_CONFIG_HOST
#warning "Setting MQTT_CONFIG_HOST to localhost as not previous definition was found."
#define MQTT_CONFIG_HOST                            "localhost"
#endif

/**
 * @brief Length of MQTT server host name.
 */
#define MQTT_CONFIG_HOST_LEN                        ( ( uint16_t ) ( sizeof( MQTT_CONFIG_HOST ) - 1 ) )

/**
 * Provide default values for undefined configuration settings.
 */
#ifndef MQTT_CONFIG_PORT
#define MQTT_CONFIG_PORT                            ( 1883 )
#endif

#ifndef MQTT_CONFIG_NETWORK_BUF_SIZE
#define MQTT_CONFIG_NETWORK_BUF_SIZE                ( 1024U )
#endif

/**
 * @brief Timeout for receiving CONNACK packet in milli seconds.
 */
#define MQTT_CONFIG_CONNACK_RECV_TIMEOUT_MS         ( 1000U )

/**
 * @brief The maximum time interval in seconds which is allowed to elapse
 * between two Control Packets.
 */
#ifndef MQTT_KEEP_ALIVE_INTERVAL_SECONDS
#define MQTT_KEEP_ALIVE_INTERVAL_SECONDS            ( 60U )
#endif

/**
 * @brief Transport timeout in milliseconds for transport send and receive.
 */
#define MQTT_CONFIG_TRANSPORT_SEND_RECV_TIMEOUT_MS  ( 1000 )

/**
 * @brief The length of the outgoing publish records array used by the coreMQTT
 * library to track QoS > 0 packet ACKS for outgoing publishes.
 */
#define MQTT_CONFIG_OUTGOING_PUBLISH_RECORD_LEN     ( 10U )

/**
 * @brief The length of the incoming publish records array used by the coreMQTT
 * library to track QoS > 0 packet ACKS for incoming publishes.
 */
#define MQTT_CONFIG_INCOMING_PUBLISH_RECORD_LEN     ( 10U )

#define MQTT_LOCK_TIMEOUT_MS                        500
#define MQTT_SUBS_COMP_BUF_SIZE                     64
#ifndef MQTT_PUB_COMP_BUF_SIZE
#define MQTT_PUB_COMP_BUF_SIZE                      64
#endif

#ifndef MQTT_PROC_INTERVAL_MS
#define MQTT_PROC_INTERVAL_MS                       ( 50U )
#endif

//=============================================================================

#endif /* MQTT_DEFS_H_ */
