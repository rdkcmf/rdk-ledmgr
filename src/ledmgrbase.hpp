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
#ifndef LEDMGRBASE_H
#define LEDMGRBASE_H
#include <vector>
#include "ledmgr_types.hpp"
#include "indicator.hpp"
#include "pthread.h"
#include "fp_profile.hpp"

#define IARMBUS_OWNER_NAME "ledmgr"
class ledMgrBase
{
	protected:
		bool m_is_powered_on;
		unsigned int m_error_flags;
		pthread_mutex_t m_mutex;
		std::vector <blinkPattern_t> m_patterns;
		std::vector <indicator> m_indicators;
		/* Detect capabilies. Make a list of indicator objects. */
	public:
		ledMgrBase();
		~ledMgrBase();
		virtual int createBlinkPatterns();
		const blinkPattern_t * getPattern(blinkPatternType_t pattern) const;
		void diagnostics();
		indicator& getIndicator(const std::string &name);
		virtual void handleCDLEvents(unsigned int event){}
		virtual void handleModeChange(unsigned int mode){}
		virtual void handleGatewayConnectionEvent(unsigned int state, unsigned int error){}
		virtual void handleDeviceReset(const unsigned int sequence){}
		virtual void handleDeviceResetAbort(){}
		virtual void handleKeyPress(int key_code, int key_type){}
		void setPowerState(bool state);
		bool getPowerState();
		bool setError(unsigned int position, bool value);
};

#endif /*LEDMGRBASE_H*/
