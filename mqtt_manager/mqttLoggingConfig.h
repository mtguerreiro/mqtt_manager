
#ifndef MQTT_LOGGING_CONFIG_H_
#define MQTT_LOGGING_CONFIG_H_

#include "mqttmngConfig.h"

#include "logging_levels.h"

#ifndef MQTT_LOG_NAME
#define MQTT_LOG_NAME           "MQTT"
#endif

#ifndef MQTT_LOG_LEVEL
#define MQTT_LOG_LEVEL          LOG_DEBUG
#endif

#ifndef LIBRARY_LOG_NAME
#define LIBRARY_LOG_NAME        MQTT_LOG_NAME
#endif

#ifndef LIBRARY_LOG_LEVEL
#define LIBRARY_LOG_LEVEL       MQTT_LOG_LEVEL
#endif
#include "logging_stack.h"

#endif
