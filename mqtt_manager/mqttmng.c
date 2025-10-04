
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
//=============================================================================

//=============================================================================
/*--------------------------------- Globals ---------------------------------*/
//=============================================================================
static mqttmngmng_t mqttmng = {
    .components = {0},
    .componentsLen = 0
};
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

    status = mqttInit(clientId, 0, 0, 0);

    return status;
}
//-----------------------------------------------------------------------------
void mqttmngRun(void){

    while(1){

        if( mqttmngLock(MQTT_CONFIG_LOCK_TIMEOUT_MS) != 0 ) continue;

        mqttRun(0);

        mqttmngUnlock();

        Clock_SleepMs(MQTT_CONFIG_PROC_INTERVAL_MS);
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

    int32_t status;
    mqttPayload_t payload;
    int clen;
    int blen;

    char topic[MQTT_MNG_CONFIG_PUB_COMP_BUF_SIZE];
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

    payload.data = mqttmng.components;
    payload.size = mqttmng.componentsLen;
    payload.retain = 1;
    payload.dup = 0;

    LogDebug( ("Publishing components %s...", mqttmng.components) );
    snprintf(topic, sizeof(topic), "%s/components", mqttmng.clientId);
    status = mqttPublish(topic, &payload);
    LogDebug( ("Publish status %d ", (int)status) );

    mqttmngUnlock();

    return status;
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
//=============================================================================
