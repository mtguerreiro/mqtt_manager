
#ifndef MQTT_MNG_LOGGING_CONFIG_H_
#define MQTT_MNG_LOGGING_CONFIG_H_

#include "mqttmngConfig.h"

#include "logging_levels.h"

#ifndef MQTT_MANAGER_LOG_NAME
#define MQTT_MANAGER_LOG_NAME   "MQTT MNG"
#endif

#ifndef MQTT_MANAGER_LOG_LEVEL
#define MQTT_MANAGER_LOG_LEVEL  LOG_WARN
#endif

#ifndef LIBRARY_LOG_NAME
#define LIBRARY_LOG_NAME        MQTT_MANAGER_LOG_NAME
#endif

#ifndef LIBRARY_LOG_LEVEL
#define LIBRARY_LOG_LEVEL       MQTT_MANAGER_LOG_LEVEL
#endif
#include "logging_stack.h"

#endif
