#ifndef MQTT_MNG_H_
#define MQTT_MNG_H_

//=============================================================================
/*-------------------------------- Includes ---------------------------------*/
//=============================================================================
#include "stdint.h"
#include "stddef.h"

// #include "MQTTClient.h"
#include "mqttmngConfig.h"
#include "mqtt_subscription_manager.h"
//============================================================================

//=============================================================================
/*------------------------------- Definitions -------------------------------*/
//=============================================================================
typedef SubscriptionManagerCallback_t mqttmngSubscrCb_t;

typedef struct mqttmngSubscrConfig_t
{
    const char *topic;
    mqttmngSubscrCb_t callback;
}mqttmngSubscrConfig_t;


typedef struct mqttmngConfig_t{
    const char *name;
    const char *type;
    const char *flags;
    mqttmngSubscrConfig_t **subscriptions;
    uint32_t nSubscriptions;
}mqttmngConfig_t;

typedef int32_t (*mqttmngLock_t)(uint32_t timeout);
typedef void (*mqttmngUnlock_t)(void);

#define MQTT_MNG_COMPONENT_STR(comp) 

typedef struct 
{
    uint8_t retain;
    uint8_t dup;
    const void * data;
    uint32_t size;
} mqttmngPayload_t;

//=============================================================================

//=============================================================================
/*-------------------------------- Functions --------------------------------*/
//=============================================================================
//-----------------------------------------------------------------------------
int32_t mqttmngInit(mqttmngLock_t lock, mqttmngUnlock_t unlock);
//-----------------------------------------------------------------------------
void mqttmngRun(void);
//-----------------------------------------------------------------------------
int32_t mqttmngPublishComponent(const char *name, const char *type, const char *flags);
// int32_t mqttmngAddComponent(uint32_t id, mqttmngConfig_t *config);
//-----------------------------------------------------------------------------
int32_t mqttmngPublish(const char *topic, mqttmngPayload_t *payload);
//-----------------------------------------------------------------------------
int32_t mqttmngSubscribe(const char *topic, mqttmngSubscrCb_t callback);
//-----------------------------------------------------------------------------
int mqttmngInitDone(void);
//-----------------------------------------------------------------------------
//=============================================================================

#endif /* MQTT_MNG_H_ */
