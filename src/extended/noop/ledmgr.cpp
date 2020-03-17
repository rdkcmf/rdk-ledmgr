/**
 * @file ledmgr.cpp
 *
 * @brief This file is an template implementation. OEM vendors are to implement the functions required for their platform.
 *
 */

#include "ledmgr.hpp"
#include "sysMgr.h"
#include "libIBusDaemon.h"
#include "irMgr.h"

/**
 * @addtogroup LED_TYPES
 * @{
 */
typedef enum
{
        GATEWAY_CONNECTION_ERROR = 0,
}errorTypes_t;

/* @} */ // End of group LED_TYPES


ledMgr::ledMgr()
{
        //TODO: Explore autodetection
        m_indicators.push_back(indicator("Power"));
}

ledMgr ledMgr::m_singleton;
ledMgr& ledMgr::getInstance()
{
        return m_singleton;
}

/**
 * @addtogroup LED_APIS
 * @{
 */
/**
 * @brief Ths API shows a reference implementation for handles when there is a changes in system mode(IARM bus system modes: NORMAL, EAS, WAREHOUSE).
 *
 * @param[in] mode   IARM bus system mode.
 */
void ledMgr::handleModeChange(unsigned int mode)
{
        if(IARM_BUS_PWRMGR_POWERSTATE_ON != getPowerState())
        {
                return;
        }

        if(IARM_BUS_SYS_MODE_NORMAL == mode)
        {
                INFO("Detected mode change to NORMAL\n");
                try
                {
                        //TODO (OEM): Call appropriate ledmgr indicator API
                }
                catch(...)
                {
                        ERROR("Error setting steady state!\n");
                }

        }
}


/**
 * @brief This API shows a reference implementation of calling appropriate ledmgr indicator API depends of the gateway connection event.
 *
 * @param[in] state   error state.
 * @param[in] error   error
 */
void ledMgr::handleGatewayConnectionEvent(unsigned int state, unsigned int error)
{
        if(0 == state)
        {
                INFO("Detected gateway disconnect.\n");
                if(true == setError(GATEWAY_CONNECTION_ERROR, true))
                {
                        //Transition to error state.
                        //TODO (OEM): Call appropriate ledmgr indicator API
                }
        }
        else if(1 == state)
        {
                INFO("Detected gateway connection.\n");
                /*Clear this error and if no other errors are present, clear
                 * the error blinking pattern */
                if(true == setError(GATEWAY_CONNECTION_ERROR, false))
                {
                        //TODO (OEM): Call appropriate ledmgr indicator API
                }
        }
}

/**
 * @brief This API shows a reference implementation for handling key press and give LED indication accordingly.
 * Indicator functions are called with the supported LED pattern or color
 *
 * @param[in] key_code   key code.
 * @param[in] key_type   key type.
 */
void ledMgr::handleKeyPress(int key_code, int key_type)
{
        //TODO (OEM): Call appropriate ledmgr indicator API for keypress
}

/** @} */  //END OF GROUP LED_APIS
