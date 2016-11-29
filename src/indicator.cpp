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
#include "indicator.hpp"
#include "frontPanelConfig.hpp"
static const unsigned int INVALID_COLOR =  0xFFFFFFFF;

static gboolean masterBlinkCallbackFunction(gpointer data)
{
	indicator *ptr = (indicator *)data;
	DEBUG("Enter\n");
	ptr->timerCallback();
	return false;
}

static gboolean masterFlareCallbackFunction(gpointer data)
{
	indicator *ptr = (indicator *)data;
	DEBUG("Enter\n");
	ptr->flareCallback();
	return false;
}

indicator::indicator(const std::string &name)
{
	m_name = name;
	m_source_id = 0;
	m_saved_properties.isValid = false;
	pthread_mutexattr_t mutex_attribute;
	REPORT_IF_UNEQUAL(0, pthread_mutexattr_init(&mutex_attribute));
	REPORT_IF_UNEQUAL(0, pthread_mutexattr_settype(&mutex_attribute, PTHREAD_MUTEX_RECURSIVE));
	REPORT_IF_UNEQUAL(0, pthread_mutex_init(&m_mutex, &mutex_attribute));

	/*Caching a reference to the DS instance of the indicator. This is safe because
	 * we don't expect the indicator instances in DS to change once initialized.
	 * Below line may throw an exception*/
	m_indicator = &(device::FrontPanelConfig::getInstance().getIndicator(m_name));
	m_state = (true == m_indicator->getState() ? STATE_STEADY_ON : STATE_STEADY_OFF);
	INFO("Indicator %s initialized to state 0x%x\n", m_name.c_str(), m_state);
}

indicator::~indicator()
{
	REPORT_IF_UNEQUAL(0, pthread_mutex_lock(&m_mutex));
	if(0 != m_source_id)
	{
		REPORT_IF_UNEQUAL(true, g_source_remove(m_source_id));
		m_source_id = 0;
	}
	REPORT_IF_UNEQUAL(0, pthread_mutex_unlock(&m_mutex));
	pthread_mutex_destroy(&m_mutex);
}

const std::string& indicator::getName() const
{
	return m_name;
}

int indicator::setColor(const unsigned int color)
{
	using namespace device;
	try
	{
		m_indicator->setColor(color, false);
	}
	catch(...)
	{
		ERROR("Error setting color!\n");	
	}
}

int indicator::setBrightness(unsigned int intensity)
{
	using namespace device;
	try
	{
		m_indicator->setBrightness(intensity, false);
	}
	catch(...)
	{
		ERROR("Error setting indicator brightness!\n");
	}
}
int indicator::setBlink(const blinkPattern_t *pattern, int repetitions)
{
	if((0 == repetitions) || (2 > pattern->num_sequences))
	{
		ERROR("Bad inputs!\n");
		return -1;
	}
	INFO("Start\n");
	REPORT_IF_UNEQUAL(0, pthread_mutex_lock(&m_mutex));
	
	
	/*Cancel previous blink pattern if any*/
	if(0 != m_source_id)
	{
		REPORT_IF_UNEQUAL(true, g_source_remove(m_source_id));
		m_source_id = 0;
	}
	/*Check whether the indicator is currently disabled. Enable
	 * it if it is.*/
	else if(STATE_STEADY_OFF == m_state)
	{
		DEBUG("Indicator is currently disabled. Turning it back on.\n");
		enableIndicator(true);
	}

	m_state = STATE_BLINKING;
	m_pattern_ptr = pattern;
	m_pattern_repetitions = repetitions;
	m_sequence_read_offset = 0;
	step();
	REPORT_IF_UNEQUAL(0, pthread_mutex_unlock(&m_mutex));
	INFO("Done\n");
	return 0;
}

int indicator::step()
{
	DEBUG("Start\n");
	unsigned char offset = m_sequence_read_offset;
	enableIndicator(m_pattern_ptr->sequence[offset].isOn);

	/* Advance offset (in other words, the pattern's read-pointer)
	 * for next the next step.*/
	m_sequence_read_offset = (m_sequence_read_offset + 1) % m_pattern_ptr->num_sequences;
	//TODO: Temporary, for developer confidence
	if(m_sequence_read_offset == m_pattern_ptr->num_sequences)
	{
		ERROR("Out of bounds access!\n");
	}

	/* Evaluate whether a callback is necessary:
	 * A callback is necessary when at least one of the below conditions is true:
	 * 1. Pattern is required to iterate indefinitely (m_pattern_repetitions = -1).
	 * 2. There are remaining iterations to be executed.
	 * 3. There are steps remaining to be executed in the present iteration of the
	 *    pattern.
	 * */
	if(-1 ==  m_pattern_repetitions)
	{
		registerCallback(m_pattern_ptr->sequence[offset].length);
	}
	else
	{
		/* Only finite iterations are to be executed. Check whether there are
		 * steps or iterations remaining.*/
		if(0 == m_sequence_read_offset)
		{
			/* We've completed an iteration. Since we're executing limited iterations,
			 * update the counter*/
			m_pattern_repetitions--;
			if(0 < m_pattern_repetitions)
			{
				/* There are iterations pending */
				registerCallback(m_pattern_ptr->sequence[offset].length);
				DEBUG("End iteration\n");
			}
			else
			{
				DEBUG("Final iteration complete\n");
			}
		}
		else
		{
			/* More steps remain to complete this iteration. */
			registerCallback(m_pattern_ptr->sequence[offset].length);
		}
	}
	return 0;
}

int indicator::timerCallback()
{
	DEBUG("Enter\n");
	REPORT_IF_UNEQUAL(0, pthread_mutex_lock(&m_mutex));
	m_source_id = 0;
	step();
	REPORT_IF_UNEQUAL(0, pthread_mutex_unlock(&m_mutex));
	return 0;
}

int indicator::registerCallback(unsigned int milliseconds)
{
	if(0 == milliseconds)
	{
		ERROR("Zero-wait timer!\n");
		return -1;
	}
	m_source_id = g_timeout_add(milliseconds, masterBlinkCallbackFunction, (gpointer)this);
	if(0 == m_source_id)
	{
		ERROR("Could not register callback!\n");
	}
	return 0;
}

int indicator::setState(indicatorState_t state)
{
	INFO("state 0x%x\n", state);
	if((STATE_STEADY_ON != state) && (STATE_STEADY_OFF != state))
	{
		ERROR("Unsupported state!\n");
		return -1;
	}
	
	REPORT_IF_UNEQUAL(0, pthread_mutex_lock(&m_mutex));
	/*Cancel any blinking*/
	if(STATE_BLINKING ==  m_state)
	{
		if(0 != m_source_id)
		{
			REPORT_IF_UNEQUAL(true, g_source_remove(m_source_id));
			m_source_id = 0;
			INFO("Cancelled previously started blink operation\n");
		}
	}
	m_state = state;
	REPORT_IF_UNEQUAL(0, pthread_mutex_unlock(&m_mutex));
	
	if(STATE_STEADY_ON == state)
	{
		enableIndicator(true);
	}
	else
	{
		enableIndicator(false);
	}
	return 0;
}

int indicator::enableIndicator(bool enable)
{
	using namespace device;
	try
	{
		m_indicator->setState(enable);
	}
	catch(...)
	{
		ERROR("Could not change indicator state!\n");
	}
	return 0;
}

void indicator::saveState(void)
{
	REPORT_IF_UNEQUAL(0, pthread_mutex_lock(&m_mutex));
	m_saved_properties.state = m_state;
	if(STATE_BLINKING == m_state)
	{
		m_saved_properties.sequence_read_offset = m_sequence_read_offset;
		m_saved_properties.pattern_ptr = m_pattern_ptr;
		m_saved_properties.pattern_repetitions= m_pattern_repetitions;
	}
	try
	{
		m_saved_properties.intensity = m_indicator->getBrightness();
		m_saved_properties.color = m_indicator->getColor();
	}
	catch(...)
	{
		ERROR("Could not read brightness or color values!\n");
		m_saved_properties.intensity = 20; //safe default
		m_saved_properties.intensity= INVALID_COLOR;
	}
	m_saved_properties.isValid = true;
	REPORT_IF_UNEQUAL(0, pthread_mutex_unlock(&m_mutex));
	INFO("Saved state.\n");
}

void indicator::restoreState(void)
{
	REPORT_IF_UNEQUAL(0, pthread_mutex_lock(&m_mutex));
	if(m_saved_properties.isValid)
	{
		/*Stop whatever we're doing right now.*/
		setState(STATE_STEADY_OFF);

		if(INVALID_COLOR != m_saved_properties.color)
		{
			setColor(m_saved_properties.color);
		}

		m_state = m_saved_properties.state;
		if(STATE_STEADY_ON == m_state)
		{
			enableIndicator(true);
			INFO("Successfully restored to STEADY ON state.\n");
		}
		else if(STATE_BLINKING == m_state)
		{
			m_pattern_ptr = m_saved_properties.pattern_ptr;
			m_sequence_read_offset = m_saved_properties.sequence_read_offset;
			m_pattern_repetitions = m_saved_properties.pattern_repetitions;

			/*If the blink pattern is not set to repeat indefinitely and has completed its run,
			 * find out what the last state is supposed to be and set it.*/
			if(0 == m_pattern_repetitions)
			{
				enableIndicator(m_pattern_ptr->sequence[m_pattern_ptr->num_sequences - 1].isOn);
				INFO("Successfully restored to final holding state of blink pattern.\n");
			}
			else
			{
				INFO("Successfully restored blink pattern.\n");
				step();
			}
		}
		m_saved_properties.isValid = false; //This setting has been applied. Mark as stale.
	}
	else
	{
		ERROR("Won't restore stale settings.\n");
	}
	REPORT_IF_UNEQUAL(0, pthread_mutex_unlock(&m_mutex));
}

void indicator::executeFlare(const unsigned int percentage_increase, const unsigned int length_ms)
{
	using namespace device;
	unsigned int preflare_brightness = 20;
	try
	{
		preflare_brightness = m_indicator->getBrightness();
	}
	catch(...)
	{
		ERROR("Could not read brightness!\n");
	}


	if(0 == g_timeout_add(length_ms, masterFlareCallbackFunction, (gpointer)this))
	{
		ERROR("Could not register callback!\n");
	}
	else
	{
		unsigned int flare_level = preflare_brightness * (100  + percentage_increase) / 100;
		if(100 < flare_level)
		{
			flare_level = 100;
		}
		setBrightness(flare_level);
	}
}

void indicator::flareCallback()
{
	using namespace device;
	unsigned int preflare_brightness = 20; //safe default
	try
	{
		preflare_brightness = m_indicator->getBrightness();
	}
	catch(...)
	{
		ERROR("Could not read brightness!\n");
	}
	setBrightness(preflare_brightness);
}
