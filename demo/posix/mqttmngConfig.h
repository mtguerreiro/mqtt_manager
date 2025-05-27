#ifndef MQTT_MNG_CONFIG_H_
#define MQTT_MNG_CONFIG_H_

//=============================================================================
/*-------------------------------- Includes ---------------------------------*/
//=============================================================================
#include "stdint.h"

/**************************************************/
/******* DO NOT CHANGE the following order ********/
/**************************************************/

/* Include logging header files and define logging macros in the following order:
 * 1. Include the header file "logging_levels.h".
 * 2. Define the LIBRARY_LOG_NAME and LIBRARY_LOG_LEVEL macros depending on
 * the logging configuration for DEMO.
 * 3. Include the header file "logging_stack.h", if logging is enabled for DEMO.
 */

#include "logging_levels.h"

/* Logging configuration for the Demo. */
#ifndef LIBRARY_LOG_NAME
    #define LIBRARY_LOG_NAME    "DEMO"
#endif

#ifndef LIBRARY_LOG_LEVEL
    #define LIBRARY_LOG_LEVEL    LOG_INFO
#endif
#include "logging_stack.h"

/************ End of logging configuration ****************/

//============================================================================

//=============================================================================
/*------------------------------- Definitions -------------------------------*/
//=============================================================================
#define MQTT_MNG_CONFIG_DEV_ID      "dev-0e3ax1"
#define MQTT_MNG_CONFIG_MAX_SUBS    10

#define MQTT_MNG_CONFIG_HOST    "localhost"
#define MQTT_MNG_CONFIG_PORT    1883

#define MQTT_MNG_CONFIG_DBG     1
//=============================================================================

#endif /* MQTT_MNG_CONFIG_H_ */
