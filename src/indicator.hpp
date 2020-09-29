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
#include <iostream>
#include "ledmgr_types.hpp"
#include "pthread.h"
#include "frontPanelIndicator.hpp"
#include <glib.h>


class indicator
{
	public:
		typedef struct
		{
			bool isValid;
			indicatorState_t state;
			const blinkPattern_t *pattern_ptr;
			int pattern_repetitions;
			unsigned char sequence_read_offset;
			unsigned int intensity;
			unsigned int color;	
		}indicatorProperties_t;

	private:
		std::string m_name;
		pthread_mutex_t m_mutex;
		guint m_source_id;
		device::FrontPanelIndicator *m_indicator;

		indicatorState_t m_state;
		const blinkPattern_t *m_pattern_ptr;
		int m_pattern_repetitions;
		unsigned char m_sequence_read_offset;
		unsigned int m_preflare_brightness;

		indicatorProperties_t m_saved_properties;

	public:
		/* Configure with appropriate identifier.*/
		indicator(const std::string &name);
		~indicator();
		const std::string& getName() const;
		int setState(indicatorState_t state);
		int setBlink(const blinkPattern_t *pattern, int repetitions = -1);
		void setColor(const unsigned int color);
		int timerCallback(void);
		void saveState();
		void restoreState();
		void executeFlare(const unsigned int percentage_increase, const unsigned int length_ms);
		void flareCallback(void);
	private:
		int step();
		int registerCallback(unsigned int milliseconds);
		void setBrightness(unsigned int intensity);
		int enableIndicator(bool enable);

};

