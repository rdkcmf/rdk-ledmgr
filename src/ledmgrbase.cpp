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
#include <stdexcept>
#include "ledmgrbase.hpp"
#include "libIBus.h"

static blinkOp_t g_blink_pattern_slow_blink[] = {{500, true}, {1000, false}};
static blinkOp_t g_blink_pattern_double_blink[] = {{200, true}, {100, false}, {200, true}, {1000, false}};
static blinkOp_t g_blink_pattern_fast_blink[] = {{200, true}, {100, false}};

/**
 * @addtogroup LED_APIS
 * @{
 */

/**
 * @brief This API prints pattern details include id, sequence.
 */
void ledMgrBase::diagnostics()
{
	std::cout<<"Size of the pattern list is "<<m_patterns.size()<<"\n";
	for(int i = 0; i < m_patterns.size(); i++)
	{
		DEBUG("%x -- %x -- 0x%x\n", m_patterns[i].id, m_patterns[i].num_sequences, (unsigned int)m_patterns[i].sequence);
	}
}

/**
 * @brief This API search for the matching indicator and return the indicator.
 *
 * @return  Returns matching indicator.
 *
 * Note: throws std::invalid_argument exception
 */
indicator& ledMgrBase::getIndicator(const std::string &name)
{
	std::vector <indicator>::iterator iter;
	for(iter = m_indicators.begin(); iter != m_indicators.end(); iter++)
	{
		if(0 == name.compare(iter->getName()))
		{
			break;
		}
	}

	if(iter == m_indicators.end())
	{
		ERROR("No matching indicator found!\n")
		throw std::invalid_argument("No matching indicator found!");
	}
	return *iter;
}

/**
 * @brief Constructor function performs initialization.
 */
ledMgrBase::ledMgrBase()
{
	m_is_powered_on = false;
	m_error_flags = 0;
	pthread_mutexattr_t mutex_attribute;
	REPORT_IF_UNEQUAL(0, pthread_mutexattr_init(&mutex_attribute));
	REPORT_IF_UNEQUAL(0, pthread_mutexattr_settype(&mutex_attribute, PTHREAD_MUTEX_ERRORCHECK));
	REPORT_IF_UNEQUAL(0, pthread_mutex_init(&m_mutex, &mutex_attribute));

	REPORT_IF_UNEQUAL(0, IARM_Bus_Init(IARMBUS_OWNER_NAME));
	REPORT_IF_UNEQUAL(0, IARM_Bus_Connect());
}

/**
 * @brief Destructor API.
 */
ledMgrBase::~ledMgrBase()
{
	pthread_mutex_destroy(&m_mutex);
	REPORT_IF_UNEQUAL(0, IARM_Bus_Disconnect());
	REPORT_IF_UNEQUAL(0, IARM_Bus_Term());
}

/**
 * @brief This function sets the power state.
 *
 * @param[in] state   power state.
 */
void ledMgrBase::setPowerState(bool state)
{
	REPORT_IF_UNEQUAL(0, pthread_mutex_lock(&m_mutex));
	m_is_powered_on = state;
	REPORT_IF_UNEQUAL(0, pthread_mutex_unlock(&m_mutex));
}

/**
 * @brief This function used to get the power state.
 *
 * @return  Returns power state.
 */
bool ledMgrBase::getPowerState()
{
	bool state;
	REPORT_IF_UNEQUAL(0, pthread_mutex_lock(&m_mutex));
	state = m_is_powered_on;
	REPORT_IF_UNEQUAL(0, pthread_mutex_unlock(&m_mutex));
	return state;
}

/**
 * @brief This API stores the error and returns the transition state in order to call appropriate ledmgr indicator api.
 *
 * @param[in] position   error position which points to the error type.
 * @param[in] value      error state which points to true or false.
 *
 * @return  Returns error state transition.
 */
bool ledMgrBase::setError(unsigned int position, bool value)
{
	if(32 > position)
	{
		bool transition_detected = false;

		REPORT_IF_UNEQUAL(0, pthread_mutex_lock(&m_mutex));
		if(true == value)
		{
			if(0 == m_error_flags)
			{
				/*We're going from no errors to error-state*/ 
				transition_detected = true;
			}
			m_error_flags |= (0x01 << position);
		}
		else
		{	
			unsigned int prev_flags = m_error_flags;
			m_error_flags &= (0xFFFFFFFE << position);
			if((0 != prev_flags) && (0 == m_error_flags))
			{
				/*We're going from error-state to no errors.*/
				transition_detected = true;
			}
		}
		REPORT_IF_UNEQUAL(0, pthread_mutex_unlock(&m_mutex));
		return transition_detected;
	}
	else
	{
		ERROR("Position marker too large!\n");
		return false;
	}
}

/**
 * @brief This API creates blink patterns using the pattern type, duration, sequence … etc. as parameters.
 */
int ledMgrBase::createBlinkPatterns()
{
	m_patterns.resize(NUM_PATTERNS);
	m_patterns[STATE_SLOW_BLINK] = {STATE_SLOW_BLINK, 2, g_blink_pattern_slow_blink};
	m_patterns[STATE_DOUBLE_BLINK] = {STATE_DOUBLE_BLINK, 4, g_blink_pattern_double_blink};
	m_patterns[STATE_FAST_BLINK] = {STATE_FAST_BLINK, 2, g_blink_pattern_fast_blink};
	INFO("Complete\n");
	return 0;
}

/**
 * @brief This API return the desired pattern info with respect to pattern type.
 *
 * @param[in] type  Blink pattern type.
 *
 * @return  Returns corresponding blink pattern info structure.
 */
const blinkPattern_t * ledMgrBase::getPattern(blinkPatternType_t type) const
{
    return &m_patterns[type];
}


/** @} */  //END OF GROUP LED_APIS
