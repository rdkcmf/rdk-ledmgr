/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2016 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

/**
 * @defgroup LED LED Manager
 * - The LED Manager is an application that is responsible for controlling the LEDs on the product. 
 * - Manages the STB front panel color LED to communicate the system status.
 *
 * @defgroup LED_TYPES LED Data types
 * @ingroup  LED
 *
 * @defgroup LED_APIS  LED API
 * @ingroup  LED
 *
 **/
#include <iostream>
#include <stdio.h>
#include <stdint.h>
#include <cstdlib>
#include <string.h>
#include <semaphore.h>
#include <glib.h>
#include <pthread.h>
#include <unistd.h>

#include "libIBus.h"
#include "sysMgr.h"
#include "libIBusDaemon.h"
#include "irMgr.h"
#include "comcastIrKeyCodes.h"
#include "pwrMgr.h"

#include "frontPanelIndicator.hpp"
#include "frontPanelConfig.hpp"

#include "ledmgr_types.hpp"
#include "ledmgr.hpp"
#include "cap.h"

sem_t g_app_done_sem;

/**
 * @addtogroup LED_APIS
 * @{
 */
/** @brief This API is used to trace the debug logs prints.
 *
 *  @param[in] state  system state
 */
void trace_event(int state)
{
#define HANDLE(event) case event:\
	std::cout<<"Detected event "<<#event<<std::endl;\
	break;

	switch(state)
	{
		HANDLE(IARM_BUS_SYSMGR_SYSSTATE_CHANNELMAP);
		HANDLE(IARM_BUS_SYSMGR_SYSSTATE_DISCONNECTMGR);
		HANDLE(IARM_BUS_SYSMGR_SYSSTATE_TUNEREADY);
		HANDLE(IARM_BUS_SYSMGR_SYSSTATE_EXIT_OK);
		HANDLE(IARM_BUS_SYSMGR_SYSSTATE_CMAC);
		HANDLE(IARM_BUS_SYSMGR_SYSSTATE_MOTO_ENTITLEMENT);
		HANDLE(IARM_BUS_SYSMGR_SYSSTATE_MOTO_HRV_RX);
		HANDLE(IARM_BUS_SYSMGR_SYSSTATE_CARD_CISCO_STATUS);
		HANDLE(IARM_BUS_SYSMGR_SYSSTATE_VIDEO_PRESENTING);
		HANDLE(IARM_BUS_SYSMGR_SYSSTATE_HDMI_OUT);
		HANDLE(IARM_BUS_SYSMGR_SYSSTATE_HDCP_ENABLED);
		HANDLE(IARM_BUS_SYSMGR_SYSSTATE_HDMI_EDID_READ);
		HANDLE(IARM_BUS_SYSMGR_SYSSTATE_FIRMWARE_DWNLD);
		HANDLE(IARM_BUS_SYSMGR_SYSSTATE_TIME_SOURCE);
		HANDLE(IARM_BUS_SYSMGR_SYSSTATE_TIME_ZONE);
		HANDLE(IARM_BUS_SYSMGR_SYSSTATE_CA_SYSTEM);
		HANDLE(IARM_BUS_SYSMGR_SYSSTATE_ESTB_IP);
		HANDLE(IARM_BUS_SYSMGR_SYSSTATE_ECM_IP);
		HANDLE(IARM_BUS_SYSMGR_SYSSTATE_LAN_IP);
		HANDLE(IARM_BUS_SYSMGR_SYSSTATE_MOCA);
		HANDLE(IARM_BUS_SYSMGR_SYSSTATE_DOCSIS);
		HANDLE(IARM_BUS_SYSMGR_SYSSTATE_DSG_BROADCAST_CHANNEL);
		HANDLE(IARM_BUS_SYSMGR_SYSSTATE_DSG_CA_TUNNEL);
		HANDLE(IARM_BUS_SYSMGR_SYSSTATE_CABLE_CARD);
		HANDLE(IARM_BUS_SYSMGR_SYSSTATE_CABLE_CARD_DWNLD);
		HANDLE(IARM_BUS_SYSMGR_SYSSTATE_CVR_SUBSYSTEM);
		HANDLE(IARM_BUS_SYSMGR_SYSSTATE_DOWNLOAD);
		HANDLE(IARM_BUS_SYSMGR_SYSSTATE_VOD_AD);
		HANDLE(IARM_BUS_SYSMGR_SYSSTATE_DAC_INIT_TIMESTAMP);
		HANDLE(IARM_BUS_SYSMGR_SYSSTATE_CABLE_CARD_SERIAL_NO);
		HANDLE(IARM_BUS_SYSMGR_SYSSTATE_ECM_MAC);
		HANDLE(IARM_BUS_SYSMGR_SYSSTATE_DAC_ID);
		HANDLE(IARM_BUS_SYSMGR_SYSSTATE_PLANT_ID);
		HANDLE(IARM_BUS_SYSMGR_SYSSTATE_STB_SERIAL_NO);
		HANDLE(IARM_BUS_SYSMGR_SYSSTATE_BOOTUP);
		HANDLE(IARM_BUS_SYSMGR_SYSSTATE_GATEWAY_CONNECTION);
		HANDLE(IARM_BUS_SYSMGR_SYSSTATE_DST_OFFSET);
		HANDLE(IARM_BUS_SYSMGR_SYSSTATE_RF_CONNECTED);
		HANDLE(IARM_BUS_SYSMGR_SYSSTATE_PARTNERID_CHANGE);
		HANDLE(IARM_BUS_SYSMGR_SYSSTATE_IP_MODE);
		HANDLE(IARM_BUS_SYSMGR_SYSSTATE_LP_CONNECTION_RESET);
		HANDLE(IARM_BUS_SYSMGR_SYSSTATE_RWS_CONNECTION_RESET);
		default:
			break;
	}
#undef HANDLE
}

/**
 * @brief This API toggles between two LED modes, such as Dimming the light and Setting full brightness.
 */
void toggleBrightness()
{
	using namespace device;
	static bool isVeryBright = true;
	if(true == isVeryBright)
	{
		FrontPanelConfig::getInstance().getIndicator("Power").setBrightness(0, false);
		isVeryBright = false;
		INFO("Dimming the light\n");
	}
	else
	{
		FrontPanelConfig::getInstance().getIndicator("Power").setBrightness(100, false);
		isVeryBright = true;
		INFO("Setting full brightness\n");
	}
}

/** @brief This API  receives the IR events from IR manager to handle the detected key pressed and give LED indication accordingly using received keycode and type.
 *
 *  @param[in] owner  	owner of the event
 *  @param[in] eventId  event ID
 *  @param[in] data 	event data
 *  @param[in] len  	event size
 */
void keyEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
{
	if( IARM_BUS_PWRMGR_POWERSTATE_STANDBY_DEEP_SLEEP != ledMgr::getInstance().getPowerState() )
	{
		IARM_Bus_IRMgr_EventData_t *irEventData = (IARM_Bus_IRMgr_EventData_t*) data;
		ledMgr::getInstance().handleKeyPress(irEventData->data.irkey.keyCode, irEventData->data.irkey.keyType);	
	}
	else
		INFO("power state is deepsleep, handleKeyPress not invoked");
	return;
}

/** @brief This API handles power mode change events received from power manager.
 *
 *  Power Manager monitors Power IR key events and reacts to power state changes.
 *
 *  @param[in] owner  	owner of the event
 *  @param[in] eventId  power manager event ID
 *  @param[in] data  	event data
 *  @param[in] len 	event size
 */
void powerEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
{
	IARM_Bus_PWRMgr_EventData_t *eventData = (IARM_Bus_PWRMgr_EventData_t *)data;
	switch(eventId)
	{
		case IARM_BUS_PWRMGR_EVENT_MODECHANGED:
			ledMgr::getInstance().setPowerState(eventData->data.state.newState);
			INFO("Detected power status change to 0x%x\n", eventData->data.state.newState);
			break;

		case IARM_BUS_PWRMGR_EVENT_RESET_SEQUENCE:
			if(0 <= eventData->data.reset_sequence_progress)
			{
				INFO("Reset sequence %d.\n", eventData->data.reset_sequence_progress);
				ledMgr::getInstance().handleDeviceReset(eventData->data.reset_sequence_progress);
			}
			else
			{
				INFO("Exit reset sequence.\n");
				ledMgr::getInstance().handleDeviceResetAbort();
			}
			break;
		default:
			break;
	}
}

/** @brief This callback notification received when there is a system mode change to handle from  IARM manager.
 *
 *  @param[in] arg  system mode change param
 *
 *  @return Returns status of the operation.
 */
IARM_Result_t modeChangeHandler(void *arg)
{
	IARM_Bus_CommonAPI_SysModeChange_Param_t *param = (IARM_Bus_CommonAPI_SysModeChange_Param_t *)arg;
	ledMgr::getInstance().handleModeChange((unsigned int) param->newMode);
	return IARM_RESULT_SUCCESS;
}

/** @brief To handle IARM BUS system state event callback.
 *
 *  @param[in] owner  	owner of the event
 *  @param[in] eventId  event ID
 *  @param[in] data  	event data
 *  @param[in] len 	event size
 */
void sysEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
{
	IARM_Bus_SYSMgr_EventData_t *sysEventData = (IARM_Bus_SYSMgr_EventData_t*)data;
	IARM_Bus_SYSMgr_SystemState_t stateId = sysEventData->data.systemStates.stateId;
	trace_event(stateId);

	switch(stateId)
	{
		case IARM_BUS_SYSMGR_SYSSTATE_CHANNELMAP:
			break;
		case IARM_BUS_SYSMGR_SYSSTATE_DISCONNECTMGR:
			break;
		case IARM_BUS_SYSMGR_SYSSTATE_TUNEREADY:
			break;
		case IARM_BUS_SYSMGR_SYSSTATE_EXIT_OK:
			break;
		case IARM_BUS_SYSMGR_SYSSTATE_CMAC:
			break;
		case IARM_BUS_SYSMGR_SYSSTATE_MOTO_ENTITLEMENT:
			break;
		case IARM_BUS_SYSMGR_SYSSTATE_MOTO_HRV_RX:
			break;
		case IARM_BUS_SYSMGR_SYSSTATE_CARD_CISCO_STATUS:
			break;
		case IARM_BUS_SYSMGR_SYSSTATE_VIDEO_PRESENTING:
			break;
		case IARM_BUS_SYSMGR_SYSSTATE_HDMI_OUT:
			break;
		case IARM_BUS_SYSMGR_SYSSTATE_HDCP_ENABLED:
			break;
		case IARM_BUS_SYSMGR_SYSSTATE_HDMI_EDID_READ:
			break;
		case IARM_BUS_SYSMGR_SYSSTATE_FIRMWARE_DWNLD:
			ledMgr::getInstance().handleCDLEvents(sysEventData->data.systemStates.state);
			break;
		case IARM_BUS_SYSMGR_SYSSTATE_TIME_SOURCE:
			break;
		case IARM_BUS_SYSMGR_SYSSTATE_TIME_ZONE:
			break;
		case IARM_BUS_SYSMGR_SYSSTATE_CA_SYSTEM:
			break;
		case IARM_BUS_SYSMGR_SYSSTATE_ESTB_IP:
			break;
		case IARM_BUS_SYSMGR_SYSSTATE_ECM_IP:
			break;
		case IARM_BUS_SYSMGR_SYSSTATE_LAN_IP:
			break;
		case IARM_BUS_SYSMGR_SYSSTATE_MOCA:
			break;
		case IARM_BUS_SYSMGR_SYSSTATE_DOCSIS:
			break;
		case IARM_BUS_SYSMGR_SYSSTATE_DSG_BROADCAST_CHANNEL:
			break;
		case IARM_BUS_SYSMGR_SYSSTATE_DSG_CA_TUNNEL:
			break;
		case IARM_BUS_SYSMGR_SYSSTATE_CABLE_CARD:
			break;
		case IARM_BUS_SYSMGR_SYSSTATE_CABLE_CARD_DWNLD:
			break;
		case IARM_BUS_SYSMGR_SYSSTATE_CVR_SUBSYSTEM:
			break;
		case IARM_BUS_SYSMGR_SYSSTATE_DOWNLOAD:
			break;
		case IARM_BUS_SYSMGR_SYSSTATE_VOD_AD:
			break;
		case IARM_BUS_SYSMGR_SYSSTATE_DAC_INIT_TIMESTAMP:
			break;
		case IARM_BUS_SYSMGR_SYSSTATE_CABLE_CARD_SERIAL_NO:
			break;
		case IARM_BUS_SYSMGR_SYSSTATE_ECM_MAC:
			break;
		case IARM_BUS_SYSMGR_SYSSTATE_DAC_ID:
			break;
		case IARM_BUS_SYSMGR_SYSSTATE_PLANT_ID:
			break;
		case IARM_BUS_SYSMGR_SYSSTATE_STB_SERIAL_NO:
			break;
		case IARM_BUS_SYSMGR_SYSSTATE_BOOTUP:
			break;
		case IARM_BUS_SYSMGR_SYSSTATE_GATEWAY_CONNECTION:
			ledMgr::getInstance().handleGatewayConnectionEvent(sysEventData->data.systemStates.state, sysEventData->data.systemStates.error);
			break;
		case IARM_BUS_SYSMGR_SYSSTATE_DST_OFFSET:
			break;
		case IARM_BUS_SYSMGR_SYSSTATE_RF_CONNECTED:
			break;
		case IARM_BUS_SYSMGR_SYSSTATE_PARTNERID_CHANGE:
			break;
		case IARM_BUS_SYSMGR_SYSSTATE_IP_MODE:
			break;
		case IARM_BUS_SYSMGR_SYSSTATE_LP_CONNECTION_RESET:
			break;
		case IARM_BUS_SYSMGR_SYSSTATE_RWS_CONNECTION_RESET:
			break;
		default:
			break;
	}
}

/**
 * @brief To Register required IARM event handlers with appropriate callback function to handle the event.
 */
int32_t init_event_handlers()
{
	int32_t ret;
	if(0 != IARM_Bus_RegisterEventHandler(IARM_BUS_SYSMGR_NAME, IARM_BUS_SYSMGR_EVENT_SYSTEMSTATE, sysEventHandler))
	{
		goto err_3;
	}
	if(0 != IARM_Bus_RegisterEventHandler(IARM_BUS_PWRMGR_NAME,  IARM_BUS_PWRMGR_EVENT_MODECHANGED, powerEventHandler))
	{
		goto err_4;
	}

	if(0 != IARM_Bus_RegisterEventHandler(IARM_BUS_PWRMGR_NAME,  IARM_BUS_PWRMGR_EVENT_RESET_SEQUENCE, powerEventHandler))
	{
		goto err_5;
	}
	if(0 != IARM_Bus_RegisterEventHandler(IARM_BUS_IRMGR_NAME, IARM_BUS_IRMGR_EVENT_IRKEY, keyEventHandler))
	{
		goto err_6;
	}

	IARM_Bus_PWRMgr_GetPowerState_Param_t power_query_arg;
	REPORT_IF_UNEQUAL(IARM_RESULT_SUCCESS, IARM_Bus_Call(IARM_BUS_PWRMGR_NAME, "GetPowerState", (void *)&power_query_arg, sizeof(power_query_arg)));
	ledMgr::getInstance().setPowerState(power_query_arg.curState);


	if(0 != IARM_Bus_RegisterCall(IARM_BUS_COMMON_API_SysModeChange, modeChangeHandler))
	{
		goto err_7;
	}
	INFO("Successfully initialized event handlers\n");
	return 0;
	
	/*Clean exit if there are errors.*/
err_7:
	IARM_Bus_UnRegisterEventHandler(IARM_BUS_IRMGR_NAME, IARM_BUS_IRMGR_EVENT_IRKEY);
err_6:
	IARM_Bus_UnRegisterEventHandler(IARM_BUS_PWRMGR_NAME,  IARM_BUS_PWRMGR_EVENT_RESET_SEQUENCE);
err_5:
	IARM_Bus_UnRegisterEventHandler(IARM_BUS_PWRMGR_NAME,  IARM_BUS_PWRMGR_EVENT_MODECHANGED);
err_4:
	IARM_Bus_UnRegisterEventHandler(IARM_BUS_SYSMGR_NAME, IARM_BUS_SYSMGR_EVENT_SYSTEMSTATE);
err_3:
	ERROR("Error initializing event handlers\n");
	return -1;
}

/**
 * @brief This API UnRegister IARM event handlers in order to release bus-facing resources.
 */
int32_t term_event_handlers()
{
	IARM_Bus_UnRegisterEventHandler(IARM_BUS_IRMGR_NAME, IARM_BUS_IRMGR_EVENT_IRKEY);
	IARM_Bus_UnRegisterEventHandler(IARM_BUS_PWRMGR_NAME,  IARM_BUS_PWRMGR_EVENT_RESET_SEQUENCE);
	IARM_Bus_UnRegisterEventHandler(IARM_BUS_PWRMGR_NAME,  IARM_BUS_PWRMGR_EVENT_MODECHANGED);
	IARM_Bus_UnRegisterEventHandler(IARM_BUS_SYSMGR_NAME, IARM_BUS_SYSMGR_EVENT_SYSTEMSTATE);
	INFO("Successfully terminated all event handlers\n");
	return 0;
}

/**
 * @brief This API prints the LED use-cases.
 */
void print_menu()
{
	INFO("Menu:\n");
	INFO("1. Enable power\n");
	INFO("2. Disable power\n");
	INFO("3. Cycle power through colours\n");
	INFO("4. 4 slow blinks\n");
	INFO("5. 4 fast blinks\n");
	INFO("6. Simulate reset.\n");
	INFO("7. Simulate reset abort\n");
}

/**
 * @brief This Thread launches command line interface.
 */
void* command_line_prompt(void *ptr)
{
	while(true)
	{
		int singleExecution = 0;
		int choice = 0;

		if(NULL == ptr) // Print menu and read the option for multiple execution in CLI mode
		{
			print_menu();
			INFO("Enter command:\n");
			if(!(std::cin >> choice))
			{
				ERROR("Oops!\n");
				cin.clear();
				cin.ignore(10000, '\n');
				continue;
			}
		}
		else
		{
			choice = *((int *)ptr);
			cout << "Choice is : "<<choice<<endl;
			singleExecution = 1;
		} 
		
		switch(choice)
		{
			case 1:
				ledMgr::getInstance().getIndicator("Power").setState(STATE_STEADY_ON);
				break;
			case 2:
				ledMgr::getInstance().getIndicator("Power").setState(STATE_STEADY_OFF);
				break;
			case 3:
				ERROR("Unimplemented.\n");
				break;
			case 4:
				ledMgr::getInstance().getIndicator("Power").setBlink(ledMgr::getInstance().getPattern(STATE_SLOW_BLINK), 4);
				break;
			case 5:
				ledMgr::getInstance().getIndicator("Power").setBlink(ledMgr::getInstance().getPattern(STATE_FAST_BLINK), 4);
				break;
			case 6:
				{
					IARM_Bus_PWRMgr_EventData_t eventData;
					for(int i = 1; i < 6; i++)
					{
						eventData.data.reset_sequence_progress = i;
						powerEventHandler(NULL, IARM_BUS_PWRMGR_EVENT_RESET_SEQUENCE, &eventData, 0);
						sleep(1);
					}
				}
				break;
			case 7:	
				{
					IARM_Bus_PWRMgr_EventData_t eventData;
					for(int i = 1; i < 3; i++)
					{
						eventData.data.reset_sequence_progress = i;
						powerEventHandler(NULL, IARM_BUS_PWRMGR_EVENT_RESET_SEQUENCE, &eventData, 0);
						sleep(1);
					}
					eventData.data.reset_sequence_progress = -1;
					powerEventHandler(NULL, IARM_BUS_PWRMGR_EVENT_RESET_SEQUENCE, &eventData, 0);
				}
				break;

			default:
				ERROR("Unknown option.\n");
				break;
		}

		if(singleExecution) // To come out of loop in case of single execution in CLI mode
			break;
	}
	return NULL;
}

static bool drop_root()
{
    bool ret = false,retval = false;
    cap_user appcaps = {{0, 0, 0, '\0', 0, 0, 0, '\0'}};
    ret = isBlocklisted();
    if(ret)
    {
        INFO("NonRoot feature is disabled\n");
    }
    else
    {
        INFO("NonRoot feature is enabled\n");
         appcaps.caps = NULL;
         appcaps.user_name = NULL;
         if(init_capability() != NULL) {
            if(drop_root_caps(&appcaps) != -1) {
               if(update_process_caps(&appcaps) != -1) {
                   read_capability(&appcaps);
                   retval = true;
               }
            }
         }
    }
    return retval;
}

int main(int argc, char *argv[])
{
	setlinebuf(stdout); //necessary to make sure the logs get flushed when running as a daemon/service
	INFO("ledmgr is running\n");
	if(!drop_root())
        {
    	   ERROR("drop_root function failed!\n");
        }
	if(0 != sem_init(&g_app_done_sem, 0, 0))
	{
		ERROR("Could not initialize semaphore!\n");
		return -1;
	}
	GMainLoop * main_loop = g_main_loop_new(NULL, false);

	/*Initialize DS-facing resources*/
	ledMgr::getInstance().createBlinkPatterns();
	/*Initialize bus-facing resources*/
	if(0 != init_event_handlers())
	{
		ERROR("Error initializing event handlers!\n");
		return -1;
	}
	INFO("Successfully initialized event handlers\n");
	

	//TODO: Development aid. Remove
	/*Check and enable diagnostic aid*/
	if(2 == argc)
	{
		pthread_t thread;
		if(0 != pthread_create(&thread, NULL, command_line_prompt, NULL))
		{
			ERROR("Could not launch command line interface.\n");
		}
	}
	
	if(3 == argc)
	{
		int stcmp_result = strcmp(argv[1],"--CLI");
		if (0 == stcmp_result)
		{
			print_menu();
			int arg = strtol(argv[2],NULL,10);
			pthread_t thread;
			if(0 != pthread_create(&thread, NULL, command_line_prompt, (void *)&arg))
			{
				ERROR("Could not launch command line interface.\n");
			}
		}	
	}

	/*Enter event loop */
	g_main_loop_run(main_loop);
	g_main_loop_unref(main_loop);
	sem_wait(&g_app_done_sem);

	/*Release bus-facing resources*/
	term_event_handlers();
	/*Release DS-facing resources.*/
	return 0;
}


/** @} */  //END OF GROUP LED_APIS
