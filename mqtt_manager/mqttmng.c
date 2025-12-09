
//=============================================================================
/*-------------------------------- Includes ---------------------------------*/
//=============================================================================
#include "mqttmng.h"

#include "mqttDefs.h"
#include "mqtt.h"
#include "mqttLoggingConfig.h"

#include "stdlib.h"
#include "stdio.h"
#include "assert.h"

/* Clock for timer. */
#include "clock.h"
//============================================================================

//=============================================================================
/*------------------------------- Definitions -------------------------------*/
//=============================================================================
typedef struct{
    const char *clientId;
    char components[MQTT_MNG_CONFIG_SUBS_COMP_BUF_SIZE];
    uint32_t componentsLen;

    mqttLock_t lock;
    mqttUnlock_t unlock;
}mqttmngmng_t;
//=============================================================================

//=============================================================================
/*-------------------------------- Prototypes -------------------------------*/
//=============================================================================
static int32_t mqttmngLock(uint32_t timeout);
static void mqttmngUnlock(void);
static int32_t mqttmngInitLastWill(void);
static int32_t mqttmngPublishStatus(void);
static void mqttmngPublishComponents(void);
//=============================================================================

//=============================================================================
/*--------------------------------- Globals ---------------------------------*/
//=============================================================================
static mqttmngmng_t mqttmng = {
    .components = {0},
    .componentsLen = 0
};

static uint8_t lastWillPayload = 0;
static char lastWillTopicBuffer[MQTT_CONFIG_TOPIC_WITH_ID_BUF_SIZE];

static MQTTPublishInfo_t lastwill = {0};
//=============================================================================

//=============================================================================
/*-------------------------------- Functions --------------------------------*/
//=============================================================================
//-----------------------------------------------------------------------------
int32_t mqttmngInit(const char *clientId, mqttLock_t lock, mqttUnlock_t unlock){

    int status; 

    mqttmng.clientId = clientId;

    mqttmng.lock = lock;
    mqttmng.unlock = unlock;

    status = mqttmngInitLastWill();
    if( status != 0 ) return status;

    status = mqttInit(clientId, &lastwill, 0, 0);

    return status;
}
//-----------------------------------------------------------------------------
void mqttmngRun(void){

    while(1){
        if( mqttmngPublishStatus() == 0 ) break;
        LogWarn(( "Failed to publish status... will try again"));
        Clock_SleepMs(MQTT_CONFIG_PROC_INTERVAL_MS);
    }

    while(1){

        Clock_SleepMs(MQTT_CONFIG_PROC_INTERVAL_MS);

        if( mqttmngLock(MQTT_CONFIG_LOCK_TIMEOUT_MS) != 0 ) continue;

        mqttmngPublishComponents();
        mqttRun(0);

        mqttmngUnlock();
    }
}
//-----------------------------------------------------------------------------
int32_t mqttmngPublish(const char *topic, mqttPayload_t *payload){

    int status;

    if( mqttmngInitDone() != 0 ) return -1;

    if( mqttmngLock(MQTT_CONFIG_LOCK_TIMEOUT_MS) != 0 ){
        LogError( ("Failed to obtain lock when trying to publish to %s.", topic) );
        return -1;
    }

    status = mqttPublish(topic, payload);

    mqttmngUnlock();

    return status;
}
//-----------------------------------------------------------------------------
int32_t mqttmngSubscribe(const char *topic, mqttSubscrCb_t callback){

    int status;

    if( mqttmngInitDone() != 0 ) return -1;

    if( mqttmngLock(MQTT_CONFIG_LOCK_TIMEOUT_MS) != 0 ){
        LogError( ("Failed to obtain lock when trying to subscribe to %s.", topic) );
        return -1;
    }

    status = mqttSubscribe(topic, callback);
    
    mqttmngUnlock();

    return status;
}
//-----------------------------------------------------------------------------
int32_t mqttmngAddComponent(const char *name, const char *type, const char *flags){

    int clen;
    int blen;
    char buf[MQTT_MNG_CONFIG_SUBS_COMP_BUF_SIZE];

    if( mqttmngLock(MQTT_CONFIG_LOCK_TIMEOUT_MS) != 0 ){
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

    mqttmngUnlock();

    return 0;
}
//-----------------------------------------------------------------------------
int mqttmngInitDone(void){

    return mqttInitDone();
}
//-----------------------------------------------------------------------------
//=============================================================================

//=============================================================================
/*---------------------------- Static functions -----------------------------*/
//=============================================================================
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
static int32_t mqttmngInitLastWill(void){

    int topiclen;

    topiclen = snprintf(lastWillTopicBuffer, sizeof(lastWillTopicBuffer), "%s/" MQTT_MNG_CONFIG_STATUS_TOPIC, mqttmng.clientId);

    if( topiclen > (int) sizeof(lastWillTopicBuffer) ){
        LogError( ("Insufficient buffer size to format last will topic. Want %d but buffer size is %d.", topiclen, MQTT_MNG_CONFIG_SUBS_COMP_BUF_SIZE) );
        return -1;
    }
    if( topiclen < 0 ){
        LogError( ("Failed to format topic with id with error code %d.", topiclen) );
        return -1;
    }

    lastwill.pTopicName = lastWillTopicBuffer;
    lastwill.topicNameLength = topiclen;

    lastwill.qos = 0;
    lastwill.retain = 1;
    lastwill.dup = 0;

    lastwill.pPayload = (void *)&lastWillPayload;
    lastwill.payloadLength = sizeof(lastWillPayload);

    return 0;
}
//-----------------------------------------------------------------------------
static int32_t mqttmngPublishStatus(void){

    static char topic[MQTT_CONFIG_TOPIC_WITH_ID_BUF_SIZE];
    int topiclen;
    int32_t mqttStatus;
    uint8_t status = 1;
    mqttPayload_t payload = {0};

    topiclen = snprintf(topic, sizeof(topic), "%s/" MQTT_MNG_CONFIG_STATUS_TOPIC, mqttmng.clientId);
    if( topiclen > (int) sizeof(topic) ){
        LogError( ("Insufficient buffer size to format last will topic. Want %d but buffer size is %d.", topiclen, MQTT_CONFIG_TOPIC_WITH_ID_BUF_SIZE) );
        return -1;
    }
    if( topiclen < 0 ){
        LogError( ("Failed to format topic with id with error code %d.", topiclen) );
        return -1;
    }

    payload.data = (void *)&status;
    payload.size = sizeof(status);
    payload.dup = 0;
    payload.retain = 1;

    mqttStatus = mqttmngPublish( topic, &payload );

    return mqttStatus;
}
//-----------------------------------------------------------------------------
static void mqttmngPublishComponents(void){

    static char topic[MQTT_CONFIG_TOPIC_WITH_ID_BUF_SIZE];
    int topiclen;
    int32_t status;
    mqttPayload_t payload;
    static uint32_t prevComponentsLen = 0;

    if( prevComponentsLen == mqttmng.componentsLen ) return;

    topiclen = snprintf(topic, sizeof(topic), "%s/" MQTT_MNG_CONFIG_COMPONENTS_TOPIC, mqttmng.clientId);
    if( topiclen > (int) sizeof(topic) ){
        LogError( ("Insufficient buffer size to format last will topic. Want %d but buffer size is %d.", topiclen, MQTT_CONFIG_TOPIC_WITH_ID_BUF_SIZE) );
        return;
    }
    if( topiclen < 0 ){
        LogError( ("Failed to format topic with id with error code %d.", topiclen) );
        return;
    }

    payload.data = mqttmng.components;
    payload.size = mqttmng.componentsLen;
    payload.retain = 1;
    payload.dup = 0;

    LogDebug( ("Publishing components %s...", mqttmng.components) );
    status = mqttPublish(topic, &payload);
    LogDebug( ("Publish status %d ", (int)status) );

    if( status == 0 ) prevComponentsLen = mqttmng.componentsLen;
}
//-----------------------------------------------------------------------------
//=============================================================================
