#include "ledmgr.hpp"
#include "sysMgr.h"
#include "libIBusDaemon.h"
#include "irMgr.h"

typedef enum
{
        GATEWAY_CONNECTION_ERROR = 0,
}errorTypes_t;

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


void ledMgr::handleModeChange(unsigned int mode)
{
        if(false == getPowerState())
        {
                return;
        }

        if(IARM_BUS_SYS_MODE_NORMAL == mode)
        {
                INFO("Detected mode change to NORMAL\n");
                try
                {
                        //TODO (OEM): Call appropriate ledmgr indicator api
                }
                catch(...)
                {
                        ERROR("Error setting steady state!\n");
                }

        }
}


void ledMgr::handleGatewayConnectionEvent(unsigned int state, unsigned int error)
{
        if(0 == state)
        {
                INFO("Detected gateway disconnect.\n");
                if(true == setError(GATEWAY_CONNECTION_ERROR, true))
                {
                        //Transition to error state.
                        //TODO (OEM): Call appropriate ledmgr indicator api
                }
        }
        else if(1 == state)
        {
                INFO("Detected gateway connection.\n");
                /*Clear this error and if no other errors are present, clear
                 * the error blinking pattern */
                if(true == setError(GATEWAY_CONNECTION_ERROR, false))
                {
                        //TODO (OEM): Call appropriate ledmgr indicator api
                }
        }
}

/**
 * @brief Handles key press and give LED indication accordingly.
 * Indicator functions are called with the supported LED pattern or color
 *
 */
void ledMgr::handleKeyPress(int key_code, int key_type)
{
        //TODO (OEM): Call appropriate ledmgr indicator api for keypress
}
